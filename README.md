# IoT Smart Home — ESP32 v1.3-ota

> **Spec LAW:** `Smart_Home_Client_Design_ESP32_v1_2.docx` (23 sections, v1.2 layout fix — door→living, exhaust→back, 0 GPIO changes, inherits v1.1 GPIO fix) — **READ-ONLY, do not edit**  
> Archive: `Smart_Home_Client_Design_ESP32_v1.1.docx` (v1.1 GPIO fix)  
> Firmware bumped to **v1.3-ota** (ElegantOTA + `fwVersion` in status) — spec stays v1.2 READ-ONLY, no GPIO/wiring change.

## 📦 Project Structure

```
IoT Smart Home/
├── Smart_Home_Client_Design_ESP32_v1_2.docx  # ★ LAW v1.2 (READ-ONLY) — door→living, exhaust→back
├── archive/Smart_Home_Client_Design_ESP32_v1.1.docx  # Archive v1.1 (GPIO fix reference)
├── Smart_Home_Client_Overview_ESP32.docx     # Client brief (no budget, 4a real-use layout)
├── AGENTS.md                                 # Must-read rules (v1.2 law, pin double-check)
├── TODO.md                                   # Phases 0-7, v1.2 aligned
├── README.md                                 # This file
├── firmware/
│   ├── Smart_Home_ESP32_v1.3.ino             # Arduino firmware — all §6 pins, WiFiManager + ElegantOTA (v1.2 kept as archive)
│   ├── platformio.ini                        # PlatformIO config (esp32dev + libs + ElegantOTA)
│   └── README.md                             # Flash guide & tuning (OTA)
├── dashboard/
│   └── index.html                            # Standalone dashboard (mock + live ESP32)
├── docs/
│   ├── wiring_diagram.svg                    # Color wiring (matches v1.2 §6)
│   ├── wiring_diagram.md                     # Text wiring + breadboard
│   ├── circuit_diagram.svg/.md               # Full pin-level circuit
│   ├── BOM.csv                               # Excel-ready BOM
│   └── BOM.md                                # Verified BD pricing (Aug 2026)
```

---

## ✅ Deliverable 1 — Firmware

**File:** `firmware/Smart_Home_ESP32_v1.3.ino` (v1.2 archived, ~501 lines)

**Implements all spec (§6–§13) + OTA:**
- **Pins v1.1:** DHT11 GPIO4, PIR GPIO27/26, Reed GPIO25 (INPUT_PULLUP), LDR GPIO34/35 (ADC1), MQ-2 GPIO32 via 10k/20k, Relays GPIO13/18/19/21, Buzzer GPIO23
- **Safety:** Relays `HIGH` (OFF for active-LOW) as *first* line in `setup()` — prevents boot flicker; GPIO13 not GPIO5; OTA `onStart` forces all relays `OFF` + buzzer `LOW`
- **Automation:** PIR+LDR lights (30s timeout), hysteresis fan (30°C ON / 28°C OFF), door ARM/DISARM + buzzer (`wantBuzzer = gas || (armed&&open)`), MQ-2 → exhaust+buzzer; `/api/config` bounds + hysteresis guard
- **Wi-Fi:** STA with fallback AP `SmartHome-ESP32` (192.168.4.1) if home Wi-Fi fails — essential for demo reliability
- **Dashboard + OTA:** `GET /` HTML (shows `fwVersion` + `⚡ OTA Update` link), `GET /update` ElegantOTA portal, `GET /api/status` JSON (`fwVersion`), `POST /api/control` + `/api/config`

**Libs:** `DHT sensor library`, `Adafruit Unified Sensor`, `ArduinoJson@6.21.5`, `WiFiManager@2.0.17`, `ElegantOTA@3.1.6`  
**Flash:** Arduino IDE (ESP32 Dev Module, 115200) or `pio run --target upload` — first flash via USB, later updates via `http://<IP>/update`

See `firmware/README.md` for first-boot checklist and endpoint docs.

---

## ✅ Deliverable 2 — Dashboard

**Files:** 
- **Embedded:** `INDEX_HTML` PROGMEM inside firmware (served at `http://<ESP32_IP>/`)
- **Standalone:** `dashboard/index.html` — open directly in browser, supports Mock mode + remote IP

**Features (§13–§14):**
- Live sensors: temp/humidity, PIR motion badges, LDR dark/bright, door, gas raw + warning, buzzer
- Controls: Living/Bedroom Light, Room Fan, Exhaust, AUTO/MANUAL toggle, Security ARM/DISARM
- Thresholds: LDR/MQ-2 (0–4095), Temp HIGH/LOW, Light timeout — POST to `/api/config`
- Responsive (phone/laptop), polls `/api/status` every 1.5s, dark theme

**Use without hardware:** Open `dashboard/index.html` → `Mock: ON` for demo/presentation.  
**Use with hardware:** Enter ESP32 IP (or leave empty if same host) → Connect.

---

## ✅ Deliverable 3 — Wiring / Breadboard Diagram

**Files:** `docs/wiring_diagram.svg` + `docs/wiring_diagram.md`

- **SVG** (10752 bytes): Color diagram — ESP32 center, left sensors, right relays/loads, power rails, breadboard tips, room mapping. Open in Chrome/Edge → Print (Ctrl+P) for report.
- **MD** (5616 bytes): Full pin table, divider schematics, relay NO wiring, polarity test steps, power rail table, ASCII Fritzing block. Copy-paste into report.

**Key v1.1 wiring calls out:** MQ-2 divider (10k/20k), LDR dividers (3.3V), PIR 3.3V pre-check.

---

## ✅ Deliverable 4 — BOM with BD Pricing

**Files:** `docs/BOM.csv` + `docs/BOM.md`

- **Hardware subtotal:** BDT 2,859 (no model) — **with model: BDT 3,809** — with contingency: **BDT ~4,109**
- **Status:** ✅ Within BDT 4,000–5,000 budget (§17) — uses TechShopBD 485, BDTronics/Electronics 95 PIR, Dream RC 119 DHT11 pricing (Aug 2026)
- **Verification:** ESP32 430–525 range, PIR 92–95 consistent, DHT11 119–179, Relay/Fans market averages noted.
- **Tips:** Cardboard not acrylic saves ~500, standard Micro-USB ESP32 saves ~190, 2 LEDs not 4.

Import `BOM.csv` into Sheets → filter by Category / calculate alternative scenarios.

---

## 🚀 Quick Start

1. **Wire** per `docs/wiring_diagram.svg` (§7) — **test PIR 3.3V + relay polarity before final wiring**
2. **Flash** `firmware/Smart_Home_ESP32_v1.3.ino` via USB — on first boot, connect phone to `SmartHome-ESP32` AP to configure Wi-Fi (`v1.3-ota` shows in Serial + `/api/status`)
3. **Open** `http://<ESP32_IP>/` on phone (same Wi-Fi) or AP `SmartHome-ESP32` — header shows `fwVersion` + `⚡ OTA Update` → `/update`
4. **Tune** thresholds via dashboard (cover LDR, warm-up MQ-2 2–3 min, note clean-air value) — OTA for later tweaks: `http://<IP>/update`
5. **Demo** checklist §22 — 14 items, all wiring labelled; demo OTA: bump `FW_VERSION` then re-upload via `/update` and show version change live

---

## ⚠️ Safety (§19)

- Low-voltage 5V DC only — never 220V AC in student prototype
- MQ-2 hot — keep off foam; no flame/gas leak test
- Common GND everywhere; protect ADC from >3.3V
- Disconnect power before rewiring

---

*Generated from v1.2 spec (inherits v1.1 GPIO13 fix + v1.2 layout: door→living, kitchen rear-left, exhaust→back) — v1_2.docx is READ-ONLY. Firmware v1.3-ota adds ElegantOTA + unified buzzer + config bounds, no GPIO change.*
