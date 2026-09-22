# TODO — ESP32 Smart Home v1.4-mdns — FOLLOW v1.2 SPEC (READ-ONLY) + OTA

> **How to use:** One phase `in_progress` at a time. Move tasks only after physical/serial verification (§4 checklist). Current phase is the phase marked `in_progress`.

## Phases Overview

| Phase | Name | Status | Depends On |
|-------|------|--------|------------|
| 0 | Docs & Design (spec, wiring, BOM, overview) | **completed** | — |
| 1 | Firmware & Dashboard | **completed** | Phase 0 |
| 1.5 | OTA Update Capability (ElegantOTA + version + mDNS) | **completed** | Phase 1 |
| 2 | Procurement (electronics in hand; model materials → Phase 7) | **completed** | Phase 1.5 (use `docs/BOM.csv`) |
| 3 | Breadboard Power & ESP32 Baseline (USB baseline accepted; 5V rail check → Phase 5) | **completed** | Phase 2 |
| 4 | Sensors Incremental | **in_progress** | Phase 3 |
| 5 | Relay & Loads | **pending** | Phase 4 |
| 6 | Automation & Wi-Fi | **pending** | Phase 5 |
| 7 | House Mount & Final Handover | **pending** | Phase 6 |

---

## Phase 0 — Docs & Design (completed) ✅ — NOW v1.2 LAW

> **READ-ONLY:** `Smart_Home_Client_Design_ESP32_v1_2.docx` (v1.2) is frozen — do NOT edit. All work follows its §5 layout + §6 pin map.

- [x] Analyze `archive/Smart_Home_Client_Design_ESP32_v1.1.docx` (23 §, v1.1 GPIO fix) — archived
- [x] Analyze `Smart_Home_Client_Design_ESP32_v1_2.docx` (v1.2, §5 layout fix: door→living, exhaust→back) — **current LAW, 0 GPIO changes**
- [x] Generate `firmware/Smart_Home_ESP32_v1.2.ino` (all §6 pins, hysteresis, WiFiManager portal)
- [x] Generate `dashboard/index.html` (mock + live, embedded in firmware)
- [x] Generate `docs/wiring_diagram.svg` (solid wires 3.2px, top-layer, gap 70/50) + `docs/wiring_diagram.md` + `docs/circuit_diagram.svg/.md` — matches v1.2 §6
- [x] Generate `docs/BOM.csv` + `docs/BOM.md` (BDT ~4,109 with contingency, no budget in Overview)
- [x] Generate `Smart_Home_Client_Overview_ESP32.docx` (clean 9-section client brief + 4a real-use layout, no budget) — updated for v1.2 circulation
- [x] Create `AGENTS.md` (pin law + double-check + v1.2 read-only) + this `TODO.md`
- [x] Verify `README.md` + `firmware/README.md` quick start — now references v1.2
- [x] Remove `C:\Users\User\AppData\Local\Temp\opencode\diff.txt` — no longer needed

## Phase 1 — Firmware & Dashboard (completed) ✅

- [x] Pin defines `4,27,26,25,34,35,32,13,18,19,21,23` — v1.1 GPIO13 not GPIO5
- [x] Relay active-LOW `HIGH=OFF` as first line in `setup()` — no boot chatter
- [x] LDR `10k` + MQ-2 `10k/20k` dividers reflected in code (ADC 11dB)
- [x] Endpoints `GET /`, `GET /api/status`, `POST /api/control`, `/api/config`
- [x] Dashboard mock + thresholds editable
- [x] `platformio.ini` + lib deps `DHT + ArduinoJson + WiFiManager`
- [x] NVS persistence — `Preferences` library saves thresholds + mode + security across reboots
- [x] Buzzer unified `wantBuzzer = gasWarning || (securityArmed && doorOpen)` + `/api/config` bounds (ldr/mq 0-4095, temp hysteresis, tout 5s-5min)

## Phase 1.5 — OTA Update Capability (completed) ✅ — now v1.4-mdns

> ElegantOTA `GET /update` (browser, phone) — aligns with WiFiManager portal + dashboard story. Verified OTA on breadboard 2026-09-22.

- [x] **1.5.1** `platformio.ini` add `ayushsharma82/ElegantOTA@^3.1.6` — done
- [x] **1.5.2** `firmware/Smart_Home_ESP32_v1.3/Smart_Home_ESP32_v1.3.ino` (Arduino folder) — `#define FW_VERSION "v1.4-mdns"`, `#include <ElegantOTA.h>` + `<ESPmDNS.h>`, `ElegantOTA.begin(&server)` + `ElegantOTA.onStart` safety + `ElegantOTA.loop()`, `fwVersion` in `GET /api/status` + Serial + dashboard header + `/update` link
- [x] **1.5.3** USB flash once (COM5 @921600, BOOT hold, 1065264 bytes verified), then OTA upload `Smart_Home_ESP32_v1.3.ino.bin` (~1MB app only) via `http://192.168.1.106/update` — `[OTA] End success`, reboot `v1.4-mdns`, relays OFF, no chatter (§4.1) — **verified 2026-09-22**
- [x] **1.5.4** Document in `README.md` + `firmware/README.md` — OTA steps + rollback via USB if WiFi bricks — done
- [x] **1.5.5** Phase 1.5 `completed`
- [x] **1.5.6** mDNS `smarthome` — Serial always prints `http://smarthome.local/` + `/update` AND `http://<IP>/` + `/update`; survives DHCP changes — **verified after OTA reboot**

**Note:** `Smart_Home_Client_Design_ESP32_v1_2.docx` stays READ-ONLY (spec law); only firmware version bumped to v1.4-mdns.

## Phase 2 — Procurement (completed) ✅ — electronics in hand 2026-09-22

> Reference `docs/BOM.csv` + `docs/BOM.md` for verified BD prices (Aug 2026). Do NOT substitute pins/wiring.

- [x] **2.1** Controller + sensors — ESP32 DevKit V1 CH340C (COM5), DHT11 module, PIR HW-416A×2, LDR GL5528×2, MQ-2 + 10k/20k — in hand
- [x] **2.2** Relay & loads — 4-CH relay SRD-05VDC-SL-C active-LOW confirmed, fans/LEDs/buzzer — in hand
- [x] **2.3** Prototyping — breadboard 830 (wide-ESP32 noted: legs span ~9 cols, sensor work at ends / ESP32 off-board via F-F), jumpers incl. F-F, USB cable — in hand; 5V 2A supply + 1000µF cap → Phase 5
- [ ] **2.4** Model — foam/cardboard (600) + glue/paint (350) — deferred to Phase 7 (cardboard preferred to stay <5k)
- [x] **2.5** Unboxed & labeled: `VCC/GND/OUT/AO/IN1-4` checked vs `docs/wiring_diagram.md §2-3`; PIR unlabeled → identified VCC-OUT-GND + config jumper; LDR no polarity; ESP32 D-names mapped (D4=GPIO4 etc.)
- [x] **2.6** Relay board active-LOW verified (EL817 optocoupler, LOW=ON) — matches `RELAY_ON=LOW` (§8)

**Verification:** Electronics on table, relay polarity confirmed, no missing `10k/20k` for MQ-2 divider.

## Phase 3 — Breadboard Power & ESP32 Baseline (completed) ✅ — 2026-09-22

- [ ] **3.1** Formal power-rail measurement — `5V ±0.2V`, `3.3V ±0.1V` with 5V 2A supply + 1000µF — **deferred to Phase 5** (USB baseline accepted for sensors; no loads yet)
- [x] **3.2** ESP32 off-board via F-F (wide board spans 830) — USB flash → WiFiManager Tertho → IP 192.168.1.106 (MAC 38:3e:51:59:02:08) — Serial `115200` shows `v1.4-mdns` + `Relay pins initialized HIGH (OFF)`; dashboard `/` + OTA `/update` + `http://smarthome.local` all live
- [x] **3.3** No relay click on boot/OTA reboot (no relay wired yet; pins HIGH confirmed in Serial + `[OTA] Start OFF`)

## Phase 4 — Sensors Incremental (in_progress) — One sensor at a time, verify via `GET /api/status`

- [x] **4.1** DHT11 on `GPIO4` (board D4, 3.3V, F-F direct, ESP32 off-board) — `/api/status` + dashboard show `temperature 29.8°C` / `humidity 77%`, `fwVersion v1.4-mdns` — **verified 2026-09-22**. Unwired pins float (expected ghosts): PIR MOTION, LDR 1564/4095, door OPEN, MQ2 4095→WARNING+buzzer+exhaust in AUTO; set MANUAL for now.
- [ ] **4.2** PIRs — **Measure OUT first (±3.3V on motion)** then connect `GPIO27/26` — Serial shows `pir1`/`pir2` toggling
- [ ] **4.3** LDR dividers — `3.3V→LDR→GPIO34/35→10k→GND` — cover LDR → read drops, shine → rises; tune `LDR_DARK_THRESHOLD` (default 1500)
- [ ] **4.4** MQ-2 — `AO→10k→GPIO32→20k→GND` **never direct** — wait 2-3 min warm-up, note clean-air `mq2` raw (e.g., 400-900), set `MQ2_GAS_THRESHOLD` = clean + ~300
- [ ] **4.5** Reed switch — `GPIO25 → REED → GND` (`INPUT_PULLUP`) — door closed `LOW`, open `HIGH` / `doorOpen` toggles

## Phase 5 — Relay & Loads (pending) — **High-risk: test without loads first**

- [ ] **5.1** Relay polarity test — power relay alone, drive `IN1` from ESP32 `HIGH` then `LOW` — note which energizes (expected `LOW=ON`); document in `TODO` Blockers
- [ ] **5.2** Connect relay inputs `13,18,19,21` + `GND/5V` — control via `POST /api/control?living=toggle` — confirm `GET /api/status` `livingLight` etc. toggle with no click on boot
- [ ] **5.3** Connect loads via `COM→5V, NO→Load+→GND` — test each with LED first, then fans — confirm `CH1 Living, CH2 Bedroom, CH3 Fan, CH4 Exhaust`
- [ ] **5.4** Buzzer on `GPIO23` — test via `gasWarning` threshold; use transistor if 5V high-current buzzer

## Phase 6 — Automation & Wi-Fi (pending) — One feature at a time

- [ ] **6.1** Living light — `pir1 + ldr1 < threshold → CH1 ON`, 30s no-motion → OFF — test dark vs bright room
- [ ] **6.2** Bedroom light — same `pir2 + ldr2`
- [ ] **6.3** Room fan — hysteresis `TEMP_HIGH 30°C ON / TEMP_LOW 28°C OFF` — verify no chatter near threshold
- [ ] **6.4** Door security — `ARM` → open door → buzzer + `doorOpen` alert; `DISARM` → no buzzer
- [ ] **6.5** Kitchen — `mq2 >= threshold → CH4 + buzzer` — safe demo only (no flame/gas leak)
- [ ] **6.6** Dashboard — phone on same Wi-Fi opens `http://<ESP32_IP>/` or AP `SmartHome-ESP32` — all controls + `AUTO/MANUAL` + `ARM/DISARM` + thresholds editable

## Phase 7 — House Mount & Final Handover (pending)

- [ ] **7.1** Install sensors in model — hidden wall holes, label `PIR/LDR/DHT/MQ-2/Reed` both ends of every wire
- [ ] **7.2** Keep relay/load wires separated from sensor wires (§10) — MQ-2 off foam (hot)
- [ ] **7.3** Secure & label all wires — electronics under/behind model (§10)
- [ ] **7.4** End-to-end demo — §22 checklist: Wi-Fi, dashboard, temp, PIR×2, manual override, auto light/dark, fan, door, gas/exhaust, boot-OFF, labelled wiring
- [ ] **7.5** Deliver `Smart_Home_Client_Overview_ESP32.docx` to client (no budget included)

---

## Current Blockers

| # | Blocker | Phase | Action |
|---|---------|-------|--------|
| - | Floating ghosts on unwired pins (PIR HIGH, ADC 4095, reed OPEN) — expected, not a fault | 4 | MANUAL mode until each sensor wired; no fix needed |
| - | 5V 2A rail measurement + 1000µF cap | 5 | Do before relay loads (USB enough for sensors) |

_Add here: pin conflict, relay not 3.3V-compatible, PIR 5V out, MQ-2 AO >3.3V, etc. Do NOT silently work around._

---

## Change Log

| Date | Phase | Change | By |
|------|-------|--------|----|
| 2026-08-28 | 0→1 | Initial generation: firmware, dashboard, wiring (fixed solid wires), BOM, Overview | AGENTS.md v1.1 |
| 2026-08-28 | v1.2 | Adopt `Smart_Home_Client_Design_ESP32_v1_2.docx` as LAW (v1.2 §5 layout: door→living, kitchen rear-left, exhaust→back; 0 GPIO changes) — updated AGENTS.md/TODO.md, marked v1_2 READ-ONLY, removed `diff.txt`, aligned Overview §4a + circuit docs | AGENTS.md v1.2 |
| 2026-08-28 | 1 | Add WiFiManager (captive portal) + NVS persistence (Preferences library) — thresholds/mode/security survive reboot | user request |
| 2026-08-29 | 1 | Fix buzzer unified + /api/config bounds/hysteresis | user request |
| 2026-08-29 | 1.5 | Add OTA Phase 1.5 — ElegantOTA + FW_VERSION v1.3-ota (in_progress, before first flash) | user request |
| 2026-09-22 | 1.5→4 | USB flash COM5 + WiFi Tertho .106 + OTA to v1.4-mdns (mDNS smarthome.local, Serial IP+.local links) verified; relay OFF, NVS OK; ArduinoJson nullptr fix; procurement electronics done; baseline done; DHT11 D4 29.8°C/77% verified, MANUAL for floats | build session |

## Verification Checklist (for agent self-report)

Before marking any Phase 4–6 task completed, state:
- Which §6 pin(s) were touched: _4.1: GPIO4 (D4) only — DHT11 DATA, 3.3V power_
- Divider/power check done (3.3V LDR, 10k/20k MQ-2): _yes — DHT on 3.3V, VIN unused; LDR/MQ-2 dividers not yet wired (pending 4.3/4.4)_
- Relay ACTIVE-LOW confirmed: _yes — SRD-05VDC-SL-C EL817, LOW=ON; pins HIGH (OFF) on boot + OTA reboot_
- `/api/status` or Serial 115200 evidence: _29.8°C/77%, fwVersion v1.4-mdns, IP .106 + smarthome.local links_
