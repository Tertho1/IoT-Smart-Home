/*
 * IoT-Based Smart Home Monitoring & Automation System
 * Controller: ESP32 DevKit V1 — v1.3-ota (GPIO13 fix + WiFiManager + ElegantOTA)
 * Spec: D:\Projects\IoT Smart Home\Smart_Home_Client_Design_ESP32_v1_2.docx — READ-ONLY
 *
 * Wiring §6-8 (v1.1 GPIO fix + v1.2 layout):
 *   GPIO4  -> DHT11 DATA
 *   GPIO27 -> PIR#1 OUT (Living)  | CHECK 3.3V logic before wiring!
 *   GPIO26 -> PIR#2 OUT (Bedroom) | ESP32 not 5V-tolerant
 *   GPIO25 -> Reed Switch -> GND (INPUT_PULLUP)
 *   GPIO34 -> LDR#1 divider midpoint (ADC1, input-only)
 *   GPIO35 -> LDR#2 divider midpoint (ADC1, input-only)
 *   GPIO32 -> MQ-2 AO via 10k/20k divider (ADC1) ~5V->3.33V
 *   GPIO13 -> Relay CH1 IN1 Living Light (moved from GPIO5 strapping pin)
 *   GPIO18 -> Relay CH2 IN2 Bedroom Light
 *   GPIO19 -> Relay CH3 IN3 Room Fan
 *   GPIO21 -> Relay CH4 IN4 Kitchen Exhaust
 *   GPIO23 -> Buzzer (+ GND) — use transistor for 5V high-current buzzer
 *
 * Relay Note: Most budget 4CH modules are ACTIVE-LOW.
 *   RELAY_ON = LOW, RELAY_OFF = HIGH. Confirm with bare-board test!
 *   All relay pins set HIGH (OFF) as FIRST line in setup().
 */

#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <WiFiManager.h> // v1.2 — captive portal for client Wi-Fi (no re-flash)
#include <Preferences.h>
#include <ElegantOTA.h>   // v1.3 — OTA via http://<IP>/update (browser, phone)

// ---------- CONFIGURATION ----------
#define FW_VERSION           "v1.3-ota"   // shown in Serial + GET /api/status, bump on each OTA
#define WIFI_AP_SSID    "SmartHome-ESP32"
#define WIFI_AP_PASS    "12345678"
#define WIFI_PORTAL_TIMEOUT 180  // seconds portal stays open

// Pin definitions — §6 v1.1
#define PIN_DHT         4
#define PIN_PIR1        27
#define PIN_PIR2        26
#define PIN_REED        25
#define PIN_LDR1        34
#define PIN_LDR2        35
#define PIN_MQ2         32
#define PIN_RELAY_CH1   13  // Living Light — v1.1 moved from GPIO5
#define PIN_RELAY_CH2   18  // Bedroom Light
#define PIN_RELAY_CH3   19  // Room Fan
#define PIN_RELAY_CH4   21  // Kitchen Exhaust
#define PIN_BUZZER      23

#define DHT_TYPE        DHT11

// Active-LOW relay module (change to HIGH/LOW if your board is active-HIGH)
#define RELAY_ON        LOW
#define RELAY_OFF       HIGH

// Thresholds — configurable via dashboard (§12)
int LDR_DARK_THRESHOLD   = 1500;  // ADC 0-4095; lower = darker (tune in your room)
int MQ2_GAS_THRESHOLD    = 1800;  // ADC 0-4095; calibrate in clean air + warm-up 2-3 min
float TEMP_HIGH          = 30.0;  // °C hysteresis (§12.2)
float TEMP_LOW           = 28.0;
unsigned long LIGHT_TIMEOUT_MS = 30000; // 30s no-motion -> OFF (configurable)

// ---------- GLOBAL STATE ----------
DHT dht(PIN_DHT, DHT_TYPE);
WebServer server(80);
Preferences prefs;

bool modeAuto = true;          // AUTO vs MANUAL
bool securityArmed = false;

float temperature = NAN;
float humidity = NAN;
int ldr1Raw = 0, ldr2Raw = 0;
int mq2Raw = 0;
bool pir1Motion = false, pir2Motion = false;
bool doorOpen = false;         // true = OPEN (reed open = HIGH with pullup)
bool gasWarning = false;

// Manual overrides (when MANUAL or via dashboard)
bool livingLightState = false;
bool bedroomLightState = false;
bool roomFanState = false;
bool exhaustState = false;
bool buzzerState = false;

unsigned long lastDhtRead = 0;
unsigned long lastSensorRead = 0;
unsigned long livingLightTimer = 0;
unsigned long bedroomLightTimer = 0;

// ---------- RELAY HELPERS ----------
void setRelay(uint8_t pin, bool on) {
  digitalWrite(pin, on ? RELAY_ON : RELAY_OFF);
}
bool getRelay(uint8_t pin) {
  // returns true if load is ON (considering active-LOW inversion)
  return digitalRead(pin) == RELAY_ON;
}

// ---------- NVS PERSISTENCE ----------
void loadConfig() {
  prefs.begin("smartHome", true); // read-only
  LDR_DARK_THRESHOLD = prefs.getInt("ldrTh", 1500);
  MQ2_GAS_THRESHOLD  = prefs.getInt("mqTh", 1800);
  TEMP_HIGH          = prefs.getFloat("tHigh", 30.0);
  TEMP_LOW           = prefs.getFloat("tLow", 28.0);
  LIGHT_TIMEOUT_MS   = prefs.getULong("ltOut", 30000);
  modeAuto           = prefs.getBool("auto", true);
  securityArmed      = prefs.getBool("secArmed", false);
  prefs.end();
  Serial.printf("[NVS] Loaded: LDR=%d MQ2=%d T_H=%.1f T_L=%.1f tout=%lu auto=%d sec=%d\n",
    LDR_DARK_THRESHOLD, MQ2_GAS_THRESHOLD, TEMP_HIGH, TEMP_LOW, LIGHT_TIMEOUT_MS, modeAuto, securityArmed);
}

void saveConfig() {
  prefs.begin("smartHome", false); // read-write
  prefs.putInt("ldrTh", LDR_DARK_THRESHOLD);
  prefs.putInt("mqTh", MQ2_GAS_THRESHOLD);
  prefs.putFloat("tHigh", TEMP_HIGH);
  prefs.putFloat("tLow", TEMP_LOW);
  prefs.putULong("ltOut", LIGHT_TIMEOUT_MS);
  prefs.putBool("auto", modeAuto);
  prefs.putBool("secArmed", securityArmed);
  prefs.end();
}

// ---------- SENSOR READING ----------
void readSensors() {
  ldr1Raw = analogRead(PIN_LDR1);
  ldr2Raw = analogRead(PIN_LDR2);
  mq2Raw  = analogRead(PIN_MQ2);
  pir1Motion = digitalRead(PIN_PIR1) == HIGH;
  pir2Motion = digitalRead(PIN_PIR2) == HIGH;
  doorOpen   = digitalRead(PIN_REED) == HIGH; // INPUT_PULLUP: closed=GND=LOW, open=HIGH

  // DHT11 every 2s (sensor slow)
  if (millis() - lastDhtRead > 2000) {
    lastDhtRead = millis();
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (!isnan(t) && !isnan(h)) {
      temperature = t;
      humidity = h;
    }
  }
}

// ---------- AUTOMATION LOGIC §12 ----------
void applyAutomation() {
  unsigned long now = millis();

  // Gas/Smoke — highest priority (§12.4)
  if (mq2Raw >= MQ2_GAS_THRESHOLD) {
    if (!gasWarning) Serial.println("[ALERT] Gas threshold exceeded!");
    gasWarning = true;
    setRelay(PIN_RELAY_CH4, true); exhaustState = true;
  } else {
    gasWarning = false;
    // In AUTO, exhaust follows gasWarning (off when gas clears); in MANUAL keep manual state
    if (modeAuto) {
      setRelay(PIN_RELAY_CH4, false); exhaustState = false;
    }
  }

  // Buzzer: ON if gas OR (armed && door open) — single source of truth
  {
    bool wantBuzzer = gasWarning || (securityArmed && doorOpen);
    digitalWrite(PIN_BUZZER, wantBuzzer ? HIGH : LOW);
    buzzerState = wantBuzzer;
  }

  if (!modeAuto) return; // Manual mode skips light/fan automation

  // Living Room Light: PIR1 + LDR1 (§12.1)
  if (pir1Motion && ldr1Raw < LDR_DARK_THRESHOLD) {
    setRelay(PIN_RELAY_CH1, true); livingLightState = true;
    livingLightTimer = now;
  } else if (now - livingLightTimer > LIGHT_TIMEOUT_MS) {
    // Only turn off if it was auto-turned on (check timer was set)
    if (livingLightTimer != 0) {
      setRelay(PIN_RELAY_CH1, false); livingLightState = false;
    }
  }
  // If PIR but room bright -> do not turn on, but also don't reset timer

  // Bedroom Light: PIR2 + LDR2
  if (pir2Motion && ldr2Raw < LDR_DARK_THRESHOLD) {
    setRelay(PIN_RELAY_CH2, true); bedroomLightState = true;
    bedroomLightTimer = now;
  } else if (now - bedroomLightTimer > LIGHT_TIMEOUT_MS) {
    if (bedroomLightTimer != 0) {
      setRelay(PIN_RELAY_CH2, false); bedroomLightState = false;
    }
  }

  // Room Fan — hysteresis (§12.2)
  if (!isnan(temperature)) {
    if (temperature >= TEMP_HIGH) {
      setRelay(PIN_RELAY_CH3, true); roomFanState = true;
    } else if (temperature <= TEMP_LOW) {
      setRelay(PIN_RELAY_CH3, false); roomFanState = false;
    }
  }
}

// ---------- WEB DASHBOARD ----------
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Smart Home — ESP32</title>
<style>
:root{--bg:#0f172a;--card:#1e293b;--accent:#38bdf8;--ok:#22c55e;--warn:#f59e0b;--danger:#ef4444;--text:#e2e8f0}
*{box-sizing:border-box;font-family:system-ui,Segoe UI,Roboto,Arial}
body{margin:0;background:var(--bg);color:var(--text);padding:16px}
h1{text-align:center;color:var(--accent);margin:8px 0 4px}
.sub{text-align:center;opacity:.7;margin-bottom:16px;font-size:.9em}
.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(280px,1fr));gap:16px;max-width:1000px;margin:auto}
.card{background:var(--card);border-radius:16px;padding:16px;box-shadow:0 4px 12px rgba(0,0,0,.3)}
.card h2{margin:0 0 12px;font-size:1.05em;color:var(--accent);border-bottom:1px solid #334155;padding-bottom:8px}
.row{display:flex;justify-content:space-between;padding:6px 0;border-bottom:1px solid #1f2a3a}
.row:last-child{border:none}
.badge{padding:2px 10px;border-radius:999px;font-size:.85em;font-weight:600}
.ok{background:#14532d;color:#86efac}
.warn{background:#78350f;color:#fde68a}
.danger{background:#7f1d1d;color:#fecaca}
.controls{display:grid;grid-template-columns:1fr 1fr;gap:10px}
.btn{padding:10px;border:none;border-radius:10px;font-weight:700;cursor:pointer}
.on{background:var(--ok);color:#052e16}
.off{background:#334155;color:var(--text)}
.btn:active{transform:scale(.97)}
.mode{width:100%;margin-top:10px}
select,input{width:100%;padding:8px;border-radius:8px;border:1px solid #334155;background:#0f172a;color:var(--text)}
label{font-size:.85em;opacity:.8}
small{opacity:.6}
</style>
</head>
<body>
<h1>🏠 SMART HOME</h1>
<div class="sub">ESP32 DevKit V1 • <span id="ip"></span> • <span id="fw"></span> • <span id="modeLabel">AUTO</span> • Security: <span id="secLabel">DISARMED</span> • <a href="/update" target="_blank" style="color:var(--accent);text-decoration:none">⚡ OTA Update</a></div>
<div class="grid">
  <div class="card">
    <h2>📊 Sensors</h2>
    <div class="row"><span>Temperature</span><b id="temp">-- °C</b></div>
    <div class="row"><span>Humidity</span><b id="hum">-- %</b></div>
    <div class="row"><span>Living Motion (PIR1)</span><span id="pir1" class="badge">--</span></div>
    <div class="row"><span>Bedroom Motion (PIR2)</span><span id="pir2" class="badge">--</span></div>
    <div class="row"><span>Living Light (LDR1)</span><b id="ldr1">--</b></div>
    <div class="row"><span>Bedroom Light (LDR2)</span><b id="ldr2">--</b></div>
    <div class="row"><span>Door</span><span id="door" class="badge">--</span></div>
    <div class="row"><span>Gas (MQ-2)</span><span id="gas" class="badge">--</span></div>
    <div class="row"><span>MQ-2 Raw</span><b id="mqraw">--</b></div>
  </div>
  <div class="card">
    <h2>💡 Controls</h2>
    <div class="controls">
      <button class="btn" id="bLiving" onclick="toggle('living')">Living Light</button>
      <button class="btn" id="bBed" onclick="toggle('bedroom')">Bedroom Light</button>
      <button class="btn" id="bFan" onclick="toggle('fan')">Room Fan</button>
      <button class="btn" id="bExh" onclick="toggle('exhaust')">Exhaust</button>
    </div>
    <button class="btn mode" id="bMode" onclick="toggle('mode')">Switch to MANUAL</button>
    <button class="btn mode" id="bSec" onclick="toggle('security')">Security: DISARMED</button>
    <div class="row" style="margin-top:12px"><span>Buzzer</span><span id="buz" class="badge">OFF</span></div>
  </div>
  <div class="card">
    <h2>⚙️ Thresholds</h2>
    <label>LDR Dark Threshold (0-4095)</label><input id="ldrTh" type="number" min="0" max="4095">
    <label>MQ-2 Gas Threshold (0-4095)</label><input id="mqTh" type="number" min="0" max="4095">
    <label>Temp HIGH (°C)</label><input id="tHigh" type="number" step="0.5">
    <label>Temp LOW (°C)</label><input id="tLow" type="number" step="0.5">
    <label>Light Timeout (ms)</label><input id="ltOut" type="number" step="1000">
    <button class="btn mode on" onclick="saveThresholds()">Save Thresholds</button>
    <small>Tip: LDR bright ~2500-3500, dark ~200-800. Calibrate in your room.</small>
  </div>
</div>
<script>
let state={};
async function fetchStatus(){
  let r=await fetch('/api/status'); state=await r.json();
  document.getElementById('ip').textContent=state.ip;
  document.getElementById('fw').textContent=state.fwVersion || '--';
  document.getElementById('temp').textContent=state.temperature!=null? state.temperature.toFixed(1)+' °C' : '--';
  document.getElementById('hum').textContent=state.humidity!=null? state.humidity.toFixed(0)+' %' : '--';
  setBadge('pir1', state.pir1? 'MOTION':'None', state.pir1?'warn':'ok');
  setBadge('pir2', state.pir2? 'MOTION':'None', state.pir2?'warn':'ok');
  document.getElementById('ldr1').textContent=state.ldr1 + (state.ldr1 < state.ldrThreshold ? ' (DARK)' : ' (BRIGHT)');
  document.getElementById('ldr2').textContent=state.ldr2 + (state.ldr2 < state.ldrThreshold ? ' (DARK)' : ' (BRIGHT)');
  document.getElementById('mqraw').textContent=state.mq2;
  setBadge('door', state.doorOpen? 'OPEN':'CLOSED', state.doorOpen? 'danger':'ok');
  setBadge('gas', state.gasWarning? 'WARNING':'NORMAL', state.gasWarning? 'danger':'ok');
  setBadge('buz', state.buzzer? 'ON':'OFF', state.buzzer? 'danger':'ok');
  document.getElementById('modeLabel').textContent=state.modeAuto? 'AUTO':'MANUAL';
  document.getElementById('secLabel').textContent=state.securityArmed? 'ARMED':'DISARMED';
  updBtn('bLiving', state.livingLight);
  updBtn('bBed', state.bedroomLight);
  updBtn('bFan', state.roomFan);
  updBtn('bExh', state.exhaust);
  document.getElementById('bMode').textContent=state.modeAuto? 'Switch to MANUAL':'Switch to AUTO';
  document.getElementById('bMode').className='btn mode '+(state.modeAuto?'off':'on');
  document.getElementById('bSec').textContent='Security: '+(state.securityArmed? 'ARMED':'DISARMED');
  document.getElementById('bSec').className='btn mode '+(state.securityArmed? 'danger':'off');
  document.getElementById('ldrTh').value=state.ldrThreshold;
  document.getElementById('mqTh').value=state.mq2Threshold;
  document.getElementById('tHigh').value=state.tempHigh;
  document.getElementById('tLow').value=state.tempLow;
  document.getElementById('ltOut').value=state.lightTimeout;
}
function setBadge(id, text, cls){let e=document.getElementById(id); e.textContent=text; e.className='badge '+cls;}
function updBtn(id, on){let e=document.getElementById(id); e.textContent=e.textContent.split(' ')[0]+' '+(on?'[ON]':'[OFF]'); e.className='btn '+(on?'on':'off');}
async function toggle(what){
  await fetch('/api/control?'+what+'=toggle', {method:'POST'});
  fetchStatus();
}
async function saveThresholds(){
  let p=new URLSearchParams({
    ldrTh: document.getElementById('ldrTh').value,
    mqTh: document.getElementById('mqTh').value,
    tHigh: document.getElementById('tHigh').value,
    tLow: document.getElementById('tLow').value,
    ltOut: document.getElementById('ltOut').value
  });
  await fetch('/api/config?'+p.toString(), {method:'POST'});
  fetchStatus();
}
setInterval(fetchStatus, 1500); fetchStatus();
</script>
</body>
</html>
)rawliteral";

void handleRoot() { server.send_P(200, "text/html", INDEX_HTML); }

void handleStatus() {
  DynamicJsonDocument doc(1024);
  doc["ip"] = WiFi.localIP().toString();
  if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) {
    if (WiFi.localIP().toString() == "0.0.0.0") doc["ip"] = WiFi.softAPIP().toString();
  }
  if (isnan(temperature)) doc["temperature"] = nullptr; else doc["temperature"] = temperature;
  if (isnan(humidity)) doc["humidity"] = nullptr; else doc["humidity"] = humidity;
  doc["ldr1"] = ldr1Raw; doc["ldr2"] = ldr2Raw; doc["mq2"] = mq2Raw;
  doc["pir1"] = pir1Motion; doc["pir2"] = pir2Motion;
  doc["doorOpen"] = doorOpen; doc["gasWarning"] = gasWarning;
  doc["livingLight"] = getRelay(PIN_RELAY_CH1);
  doc["bedroomLight"] = getRelay(PIN_RELAY_CH2);
  doc["roomFan"] = getRelay(PIN_RELAY_CH3);
  doc["exhaust"] = getRelay(PIN_RELAY_CH4);
  doc["buzzer"] = buzzerState;
  doc["modeAuto"] = modeAuto;
  doc["securityArmed"] = securityArmed;
  doc["ldrThreshold"] = LDR_DARK_THRESHOLD;
  doc["mq2Threshold"] = MQ2_GAS_THRESHOLD;
  doc["tempHigh"] = TEMP_HIGH;
  doc["tempLow"] = TEMP_LOW;
  doc["lightTimeout"] = LIGHT_TIMEOUT_MS;
  doc["fwVersion"] = FW_VERSION;
  String out; serializeJson(doc, out);
  server.send(200, "application/json", out);
}

void handleControl() {
  // Toggle endpoints: /api/control?living=toggle etc.
  if (server.hasArg("living")) {
    bool cur = getRelay(PIN_RELAY_CH1); setRelay(PIN_RELAY_CH1, !cur);
  }
  if (server.hasArg("bedroom")) {
    bool cur = getRelay(PIN_RELAY_CH2); setRelay(PIN_RELAY_CH2, !cur);
  }
  if (server.hasArg("fan")) {
    bool cur = getRelay(PIN_RELAY_CH3); setRelay(PIN_RELAY_CH3, !cur);
  }
  if (server.hasArg("exhaust")) {
    bool cur = getRelay(PIN_RELAY_CH4); setRelay(PIN_RELAY_CH4, !cur);
  }
  if (server.hasArg("mode")) {
    modeAuto = !modeAuto;
    Serial.printf("[MODE] %s\n", modeAuto ? "AUTO" : "MANUAL");
    saveConfig();
  }
  if (server.hasArg("security")) {
    securityArmed = !securityArmed;
    Serial.printf("[SECURITY] %s\n", securityArmed ? "ARMED" : "DISARMED");
    saveConfig();
  }
  // Also support explicit ?living=1/0
  // (toggle is default for buttons)
  server.send(200, "application/json", "{\"ok\":true}");
}

void handleConfig() {
  if (server.hasArg("ldrTh")) {
    int v = server.arg("ldrTh").toInt();
    if (v < 0) v = 0; else if (v > 4095) v = 4095;
    LDR_DARK_THRESHOLD = v;
  }
  if (server.hasArg("mqTh")) {
    int v = server.arg("mqTh").toInt();
    if (v < 0) v = 0; else if (v > 4095) v = 4095;
    MQ2_GAS_THRESHOLD = v;
  }
  if (server.hasArg("tHigh")) {
    float v = server.arg("tHigh").toFloat();
    if (v < 0) v = 0; else if (v > 50) v = 50;
    TEMP_HIGH = v;
  }
  if (server.hasArg("tLow")) {
    float v = server.arg("tLow").toFloat();
    if (v < 0) v = 0; else if (v > 50) v = 50;
    TEMP_LOW = v;
  }
  // Enforce hysteresis: HIGH must be > LOW (+1°C min)
  if (TEMP_HIGH <= TEMP_LOW) {
    TEMP_HIGH = TEMP_LOW + 1.0;
    if (TEMP_HIGH > 50) { TEMP_HIGH = 50; TEMP_LOW = 49; }
    Serial.printf("[CONFIG] Hysteresis corrected -> T_H=%.1f T_L=%.1f\n", TEMP_HIGH, TEMP_LOW);
  }
  if (server.hasArg("ltOut")) {
    long v = server.arg("ltOut").toInt();
    if (v < 5000) v = 5000; else if (v > 300000) v = 300000;
    LIGHT_TIMEOUT_MS = (unsigned long)v;
  }
  Serial.printf("[CONFIG] LDR=%d MQ2=%d T_H=%.1f T_L=%.1f tout=%lu\n",
    LDR_DARK_THRESHOLD, MQ2_GAS_THRESHOLD, TEMP_HIGH, TEMP_LOW, LIGHT_TIMEOUT_MS);
  saveConfig();
  server.send(200, "application/json", "{\"ok\":true}");
}

void setupWiFi() {
  WiFiManager wm;
  wm.setConfigPortalTimeout(WIFI_PORTAL_TIMEOUT);
  wm.setWiFiAutoReconnect(true);
  // wm.resetSettings(); // uncomment to force portal for testing
  Serial.println("[WiFi] AutoConnect — if no saved Wi-Fi, portal SmartHome-ESP32 will open");
  bool res = wm.autoConnect(WIFI_AP_SSID, WIFI_AP_PASS);
  if (!res) {
    Serial.println("[WiFi] Portal timed out — restarting");
    delay(2000);
    ESP.restart();
  } else {
    Serial.printf("[WiFi] Connected! IP: %s\n", WiFi.localIP().toString().c_str());
    if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) {
      // Connected via portal, still show AP IP for debug
      Serial.printf("[WiFi] Portal AP was %s\n", WIFI_AP_SSID);
    }
  }
}

// ---------- SETUP / LOOP ----------
void setup() {
  // CRITICAL: relays OFF first — before anything else (active-LOW boards)
  pinMode(PIN_RELAY_CH1, OUTPUT); digitalWrite(PIN_RELAY_CH1, RELAY_OFF);
  pinMode(PIN_RELAY_CH2, OUTPUT); digitalWrite(PIN_RELAY_CH2, RELAY_OFF);
  pinMode(PIN_RELAY_CH3, OUTPUT); digitalWrite(PIN_RELAY_CH3, RELAY_OFF);
  pinMode(PIN_RELAY_CH4, OUTPUT); digitalWrite(PIN_RELAY_CH4, RELAY_OFF);

  Serial.begin(115200);
  delay(500);
  Serial.printf("\n=== Smart Home ESP32 %s — WiFiManager + OTA ===\n", FW_VERSION);
  Serial.println("Relay pins initialized HIGH (OFF for active-LOW)");

  pinMode(PIN_PIR1, INPUT);
  pinMode(PIN_PIR2, INPUT);
  pinMode(PIN_REED, INPUT_PULLUP);
  pinMode(PIN_BUZZER, OUTPUT); digitalWrite(PIN_BUZZER, LOW);
  // ADC pins no pinMode needed, but set attenuation for full 0-3.3V
  analogSetPinAttenuation(PIN_LDR1, ADC_11db);
  analogSetPinAttenuation(PIN_LDR2, ADC_11db);
  analogSetPinAttenuation(PIN_MQ2,  ADC_11db);

  dht.begin();
  // MQ-2 warm-up note: readings unstable for ~2-3 min after power-on
  Serial.println("[MQ2] Warming up — wait 2-3 min before calibrating threshold");

  setupWiFi();
  loadConfig(); // restore saved thresholds + mode from NVS

  server.on("/", handleRoot);
  server.on("/api/status", handleStatus);
  server.on("/api/control", HTTP_POST, handleControl);
  server.on("/api/control", HTTP_GET, handleControl);
  server.on("/api/config", HTTP_POST, handleConfig);
  server.on("/api/config", HTTP_GET, handleConfig);
  ElegantOTA.begin(&server); // http://<IP>/update — OTA from phone/browser (no USB)
  ElegantOTA.onStart([]() {
    Serial.printf("[OTA] Start — turning relays OFF for safety\n");
    // Safety: de-energize all loads before flash/reboot
    digitalWrite(PIN_RELAY_CH1, RELAY_OFF);
    digitalWrite(PIN_RELAY_CH2, RELAY_OFF);
    digitalWrite(PIN_RELAY_CH3, RELAY_OFF);
    digitalWrite(PIN_RELAY_CH4, RELAY_OFF);
    digitalWrite(PIN_BUZZER, LOW);
  });
  ElegantOTA.onEnd([](bool success) {
    Serial.printf("[OTA] End %s — rebooting\n", success ? "success" : "failed");
  });
  server.begin();
  Serial.printf("[HTTP] Server started @ http://%s/  OTA @ http://%s/update\n", WiFi.localIP().toString().c_str(), WiFi.localIP().toString().c_str());
  if (WiFi.getMode() == WIFI_AP) Serial.printf("[HTTP] AP @ http://%s/  OTA @ http://%s/update\n", WiFi.softAPIP().toString().c_str(), WiFi.softAPIP().toString().c_str());
}

void loop() {
  server.handleClient();
  ElegantOTA.loop();
  if (millis() - lastSensorRead > 200) {
    lastSensorRead = millis();
    readSensors();
    applyAutomation();
  }
}
