# Progress Log — ESP32 Smart Home v1.3

> One line per verified step. Details in `TODO.md` phases.

## 2026-09-22 — Breadboard baseline + WiFi (Phase 3.2 partial)
- [x] Procurement unboxed, modules labeled (relay confirmed active-LOW SRD-05VDC-SL-C)
- [x] PIR HW-416A identified: 3-pin power VCC-OUT-GND + config jumper side, left alone. OUT must measure ~3.3V before GPIO27/26 (deferred)
- [x] LDR GL5528 identified: no polarity, pair with 10k for GPIO34/35 divider (wiring deferred)
- [x] Breadboard 830 explained: a-e / f-j rows, center gap for ESP32, +/- rails for power
- [x] CH340C driver installed, COM5 confirmed (COM1/3/4 ignored)
- [x] Arduino IDE libs installed: DHT, Unified Sensor, ArduinoJson v6 (not v7), WiFiManager 2.0.17, ElegantOTA 3.1.6, esp32 board
- [x] Fix compile: `handleStatus()` nullptr ternary -> if/else (v1.2 + v1.3). ArduinoJson v6 `DynamicJsonDocument` kept
- [x] Flash via USB COM5 @921600 (BOOT hold): Wrote 1065264 bytes, hash verified. Chip ESP32-D0WD-V3 MAC 38:3e:51:59:02:08
- [x] Serial 115200 boot OK: `v1.3-ota` + `Relay HIGH (OFF)` + `[MQ2] warmup` + portal `SmartHome-ESP32 @192.168.4.1`
- [x] WiFiManager portal: joined Tertho, IP 192.168.1.106, NVS loaded, `Server @ http://192.168.1.106/` + OTA `@ /update`
- [x] Dashboard `/` + OTA `/update` both live (same LAN)
- [x] Reboot test (no EN press): auto-joins Tertho, same IP, relays OFF, NVS OK — client power-cycle behavior confirmed
- [ ] Next: power via 5V 2A (not USB) OR DHT11 GPIO4 single-sensor test. Security (auth) + mDNS `smarthome.local` deferred per user.

## Notes / Decisions
- First flash must be USB; OTA after that.
- Client needs no laptop/Serial: portal shows IP, Fing/router as fallback. Fixed name via mDNS planned v1.4.
- DHCP: reserve 192.168.1.106 for MAC above to keep IP stable.
