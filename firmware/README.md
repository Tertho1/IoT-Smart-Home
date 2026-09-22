# Smart Home ESP32 v1.3 — Firmware (OTA)

## Quick Start

### Arduino IDE
1. Install ESP32 board package (Boards Manager → `esp32` by Espressif)
2. Install libraries: `DHT sensor library`, `Adafruit Unified Sensor`, `ArduinoJson` (v6), `WiFiManager` by tzapu, `ElegantOTA` by ayushsharma82
3. Open `Smart_Home_ESP32_v1.3.ino` (v1.2 kept as archive)
4. Board: `ESP32 Dev Module`, Upload Speed: `921600`, Flash: `4MB`
5. Upload via USB (first flash), open Serial Monitor @ 115200
6. On first boot, connect phone to `SmartHome-ESP32` AP → select your Wi-Fi → done (no re-flash)
7. OTA updates after first flash: open `http://<ESP32_IP>/update` on phone/browser → upload new `.bin`

### PlatformIO
```bash
cd firmware
pio run --target upload
pio device monitor
# OTA after first flash: build then upload via http://<IP>/update (ElegantOTA)
```

## First Boot Checklist (§16)
1. Verify 5V and 3.3V rails with multimeter
2. Check Serial: `Smart Home ESP32 v1.3-ota` + `Relay pins initialized HIGH (OFF)` — relays should NOT click
3. If relays click on boot → your board is active-HIGH: change `RELAY_ON/OFF` defines
4. Test each relay via dashboard or `POST /api/control?living=toggle`
5. PIR test: measure OUT with multimeter first (should be ~3.3V on motion) before connecting to GPIO27/26
6. LDR test: cover LDR → ADC should drop (<800), shine light → rises (>2500)
7. MQ-2: wait 2-3 min warm-up, note clean-air ADC (e.g., 400-900), set threshold ~300 above that
8. Config persistence: thresholds + mode + security saved to NVS — survives reboot
9. OTA: open `http://<ESP32_IP>/update` → upload `.bin` → check Serial `[OTA] Start/End` and `fwVersion` in `GET /api/status` increments

## Endpoints
- `GET /` → Dashboard HTML (shows `fwVersion` + `⚡ OTA Update` link to `/update`)
- `GET /update` → ElegantOTA upload portal (browser/phone, no auth on LAN — same as control endpoints)
- `GET /api/status` → JSON status (includes `fwVersion: "v1.3-ota"`)
- `POST /api/control?living=toggle&bedroom=toggle&fan=toggle&exhaust=toggle&mode=toggle&security=toggle`
- `POST /api/config?ldrTh=1500&mqTh=1800&tHigh=30&tLow=28&ltOut=30000` (clamped: ldr/mq 0-4095, temp 0-50 with 1°C hysteresis, tout 5s-5min)

## Tuning
- `LDR_DARK_THRESHOLD` 1500: lower if lights trigger in bright room
- `MQ2_GAS_THRESHOLD` 1800: calibrate in clean air
- `TEMP_HIGH/LOW` 30/28 hysteresis prevents fan chattering

## Safety Notes (§19)
- Low-voltage 5V DC only — no 220V AC in prototype
- MQ-2 becomes hot — don't mount touching foam
- Disconnect power before rewiring
- OTA `onStart` turns all relays `HIGH (OFF)` + buzzer LOW for safe flash/reboot
