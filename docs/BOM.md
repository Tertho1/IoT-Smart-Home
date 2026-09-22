# Bill of Materials — ESP32 Smart Home v1.2

> **Budget target:** BDT 4,000–5,000 (§17) — prices verified Aug 2026 from TechShopBD / BDTronics / RoboticsBD / Electronics.com.bd / Dream RC  
> **Full CSV:** `docs/BOM.csv` (import to Excel/Sheets)  
> **SVG Wiring:** `docs/wiring_diagram.svg`

## Summary

| Group | Target (Doc §17) | This BOM | Status |
|-------|------------------|----------|--------|
| Controller + sensors | BDT 1,300–1,700 | **BDT 1,080** | ✅ Under |
| Relay + loads | BDT 700–1,000 | **BDT 630** | ✅ Under |
| Breadboard + wiring | BDT 400–600 | **BDT 270** | ✅ Under |
| Power | BDT 200–300 | **BDT 315** | Slight over — 2A quality supply |
| Physical model | BDT 900–1,300 | **BDT 950** | ✅ Mid |
| Contingency | BDT 200–400 | **BDT 300** | — |
| **TOTAL** | **BDT 3,700–5,300** | **BDT ~3,809 (grand ~4,109 with contingency)** | **✅ Within 4k–5k** |

> To stay **strictly < BDT 5,000**: use cardboard/foam board (not acrylic), 2 LEDs not 4, and buy standard modules (no Type-C premium ESP32).

---

## Detailed BOM

| # | Component | Spec | Qty | Unit BDT | Line BDT | Pin | Source (Aug 2026) |
|---|-----------|------|-----|----------|----------|-----|-------------------|
| 1 | **ESP32 DevKit V1** (30-pin, CH340) | Wi-Fi + BT, 240MHz dual-core | 1 | 485 | **485** | VIN/5V, 3.3V, GND | TechShopBD 485, IoTNest 430, Robotics 525 — **use TechShopBD price** |
| 2 | DHT11 Module | With onboard pull-up | 1 | 130 | **130** | GPIO4 | BDTronics 179, Dream RC 119 → avg ~130 |
| 3 | HC-SR501 PIR | 4.5–20V, 3.3V TTL | 2 | 95 | **190** | GPIO27, GPIO26 | RoboticsBD 92, BDTronics 95, Electronics 95 |
| 4 | LDR GL5528 | Photoresistor | 2 | 12 | **24** | GPIO34/35 via divider | Local 10–15 |
| 5 | Resistor 10kΩ | LDR divider | 2 | 3 | **6** | — | — |
| 6 | MQ-2 Module | AO + DO | 1 | 250 | **250** | GPIO32 via 10k/20k | Market 220–300 |
| 7 | Resistor 10kΩ | MQ-2 upper | 1 | 3 | **3** | AO→GPIO32 | — |
| 8 | Resistor 20kΩ | MQ-2 lower | 1 | 3 | **3** | GPIO32→GND | — |
| 9 | Reed Switch | PS-3150 door | 1 | 45 | **45** | GPIO25 | Local 35–60 |
| 10 | 4-CH 5V Relay | Active-LOW, 3.3V logic | 1 | 240 | **240** | GPIO13*,18,19,21 | Market 220–280 — *v1.1 GPIO13 fix |
| 11 | 5V Mini Fan 40mm | DC fan | 2 | 140 | **280** | Relay CH3/CH4 | Local 120–160 |
| 12 | 5V LED Module | Light (use 2–4) | 3 | 25 | **75** | Relay CH1/CH2 | Local 15–30 |
| 13 | Active Buzzer 5V | Low-current | 1 | 35 | **35** | GPIO23 | Local 25–40 |
| 14 | Breadboard 830pt | — | 1 | 140 | **140** | — | Local 120–150 |
| 15 | Jumper Wires | M-M + M-F 65pcs | 1 | 110 | **110** | — | Local 80–120 |
| 16 | Resistor spares | 4.7k + extras | 1 | 20 | **20** | — | — |
| 17 | 5V 2A Supply | Regulated DC | 1 | 220 | **220** | VIN | Local 180–250 |
| 18 | Micro USB Cable | — | 1 | 80 | **80** | — | Local 60–100 |
| 19 | Cap 1000µF 16V | Bulk cap | 1 | 15 | **15** | VIN–GND | Recommended for fan inrush |
| 20 | Foam/Cardboard + Base | House 4 rooms | 1 | 600 | **600** | — | §17 |
| 21 | Glue/Tape/Paint | Finishing | 1 | 350 | **350** | — | — |

**Hardware subtotal (no model): BDT 2,859**  
**With model: BDT 3,809**  
**With 10% contingency: BDT ~4,109**

---

## Price Verification Notes (Aug 2026)

- **ESP32:** TechShopBD lists 485.04 (regular) and 678 for Type-C; IoTNest lists 430 (sale from 480). **Budget at 485** — Type-C variant costs more but not required.
- **PIR HC-SR501:** Consistently 92–95 across RoboticsBD/BDTronics/Electronics — **use 95**.
- **DHT11:** Wide range 119 (Dream RC) to 179 (BDTronics) — **budget 130–150**, buy module with pull-up.
- **MQ-2 / Relay / Fans:** No single canonical price — market depends on import. Relay 4-CH ~240, MQ-2 ~250, fans ~140 each are realistic Dhaka (Patriot, TechShop) averages.
- **General inflation:** Component prices shift with import USD rate — re-check before purchase; contingency BDT 300 covers a failed sensor or extra wire set.

---

## Buying Tips to Stay < BDT 5,000

1. **Cardboard not acrylic** — saves BDT 400–600.
2. **2 LEDs not 4** — saves BDT 25–50, still demonstrates.
3. **Standard ESP32 (Micro-USB CH340) not Type-C** — saves BDT ~190.
4. **Buy PIR + DHT11 as bundle** — sellers often discount.
5. **Skip optional features** (§18): ESP32-CAM, OLED, RFID — each adds 300–800.
6. **One supplier, one delivery** — save BDT 69–129 delivery per order (inside/outside Dhaka).

---

## Where to Buy (Dhaka)

- **TechShopBD** (techshopbd.com) — ESP32, kits
- **BDTronics** (bdtronics.com) — DHT11 179, PIR 95
- **RoboticsBD** (store.roboticsbd.com) — PIR 92
- **Electronics.com.bd** — PIR 95
- **Dream RC** (dream-rc.com) — DHT11 119 (cheapest)
- **New Elephant Road / Patuatuli** — best for foam, fans, LEDs, breadboard (bargain)

---

## Alternatives (if budget increases later)

- DHT22 / SHT30 instead of DHT11 (+BDT 250) — far more accurate
- 5V buck converter module for clean 5V from 12V adapter
- Logic level shifter if PIR outputs 5V (instead of replacing PIR)
