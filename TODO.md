# TODO — ESP32 Smart Home v1.3 — FOLLOW v1.2 SPEC (READ-ONLY) + OTA

> **How to use:** One phase `in_progress` at a time. Move tasks only after physical/serial verification (§4 checklist). Current phase is the phase marked `in_progress`.

## Phases Overview

| Phase | Name | Status | Depends On |
|-------|------|--------|------------|
| 0 | Docs & Design (spec, wiring, BOM, overview) | **completed** | — |
| 1 | Firmware & Dashboard | **completed** | Phase 0 |
| 1.5 | OTA Update Capability (ElegantOTA + version) | **in_progress** | Phase 1 |
| 2 | Procurement | **pending** | Phase 1.5 (use `docs/BOM.csv`) |
| 3 | Breadboard Power & ESP32 Baseline | **pending** | Phase 2 |
| 4 | Sensors Incremental | **pending** | Phase 3 |
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

## Phase 1.5 — OTA Update Capability (in_progress) — **Do before Phase 2 flash**

> ElegantOTA `GET /update` (browser, phone) — aligns with WiFiManager portal + dashboard story. Test on breadboard Phase 3 before sealing model.

- [x] **1.5.1** `platformio.ini` add `ayushsharma82/ElegantOTA@^3.1.6` — done
- [x] **1.5.2** `firmware/Smart_Home_ESP32_v1.3.ino` (from v1.2) — `#define FW_VERSION "v1.3-ota"`, `#include <ElegantOTA.h>`, `ElegantOTA.begin(&server)` + `ElegantOTA.onStart` safety + `ElegantOTA.loop()`, `fwVersion` in `GET /api/status` + Serial + dashboard header + `/update` link + `mockState` sync
- [ ] **1.5.3** Flash via USB once with OTA, then test OTA upload: `http://<ESP32_IP>/update` — confirm no relay chatter on OTA reboot (§4.1) — **do on breadboard Phase 3**
- [x] **1.5.4** Document in `README.md` + `firmware/README.md` — OTA steps + rollback via USB if WiFi bricks — done
- [ ] **1.5.5** Mark Phase 1.5 `completed` → proceed to Phase 2 procurement

**Note:** `Smart_Home_Client_Design_ESP32_v1_2.docx` stays READ-ONLY (spec law); only firmware version bumps to v1.3.

## Phase 2 — Procurement (pending) — **Next: start here**

> Reference `docs/BOM.csv` + `docs/BOM.md` for verified BD prices (Aug 2026). Do NOT substitute pins/wiring.

- [ ] **2.1** Buy controller + sensors — ESP32 (485), DHT11 (130), PIR×2 (190), LDR×2 (24), MQ-2 (250) + resistors (9) — verify 3.3V logic on PIR before leaving shop
- [ ] **2.2** Buy relay & loads — 4-CH relay (240, 3.3V-compatible), fans×2 (280), LEDs×3 (75), buzzer (35)
- [ ] **2.3** Buy prototyping — breadboard 830 (140), jumpers (110), 5V 2A supply (220), USB cable (80), 1000µF cap (15)
- [ ] **2.4** Buy model — foam/cardboard (600) + glue/paint (350) — cardboard preferred to stay <5k
- [ ] **2.5** Unbox & label every module: check printed `VCC/GND/OUT/AO/IN1-4` labels match `docs/wiring_diagram.md §2-3`
- [ ] **2.6** Record relay board model photo — note if active-LOW or active-HIGH (§8 polarity test)

**Verification:** All parts on table, checklist ticked, no missing `10k/20k` for MQ-2 divider.

## Phase 3 — Breadboard Power & ESP32 Baseline (pending)

- [ ] **3.1** Build power rails only — `5V` rail / `GND` rail / separate `3.3V` section (§10) — measure with multimeter: `5V ±0.2V`, `3.3V ±0.1V`
- [ ] **3.2** ESP32 straddles center gap — flash `firmware/Smart_Home_ESP32_v1.3.ino` (`v1.2` archived) — Serial `115200` must show `v1.3-ota` + `Relay pins initialized HIGH (OFF)` and no relay click; OTA @ `http://<IP>/update`
- [ ] **3.3** If relay clicks on boot → stop, flip `RELAY_ON/OFF` defines and re-flash (§8)

## Phase 4 — Sensors Incremental (pending) — One sensor at a time, verify via `GET /api/status`

- [ ] **4.1** DHT11 on `GPIO4` — Serial/`/api/status` shows `temperature`/`humidity` (away from fan/MQ-2 heat)
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
| - | None yet — waiting for Phase 2 procurement | — | — |

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

## Verification Checklist (for agent self-report)

Before marking any Phase 4–6 task completed, state:
- Which §6 pin(s) were touched: ___
- Divider/power check done (3.3V LDR, 10k/20k MQ-2): yes/no ___
- Relay ACTIVE-LOW confirmed: yes/no ___
- `/api/status` or Serial 115200 evidence: ___
