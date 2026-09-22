# Wiring & Breadboard Diagram — ESP32 Smart Home v1.1

> Reference: `Smart_Home_Client_Design_ESP32_v1.1.docx` §6–§8, §10, §15  
> SVG: `docs/wiring_diagram.svg` (open in browser for printable diagram)

## 1. Master Pin Map (v1.1 — GPIO13 fix)

| ESP32 Pin | Component | Type | Notes |
|-----------|-----------|------|-------|
| GPIO 4 | DHT11 DATA | Digital I/O | 3.3V power, 4.7k pull-up if bare sensor |
| GPIO 27 | PIR #1 OUT | Digital In | Living room, **verify 3.3V OUT** |
| GPIO 26 | PIR #2 OUT | Digital In | Bedroom, ESP32 not 5V-tolerant |
| GPIO 25 | Reed Switch | Digital In | `INPUT_PULLUP`, switch to GND |
| GPIO 34 | LDR #1 divider | ADC1 In | Input-only, Wi-Fi safe |
| GPIO 35 | LDR #2 divider | ADC1 In | Input-only |
| GPIO 32 | MQ-2 AO via divider | ADC1 In | 10k/20k scales 5V→3.33V |
| GPIO 13 | Relay CH1 IN1 | Digital Out | **Living Light — moved from GPIO5 (strapping pin)** |
| GPIO 18 | Relay CH2 IN2 | Digital Out | Bedroom Light |
| GPIO 19 | Relay CH3 IN3 | Digital Out | Room Fan |
| GPIO 21 | Relay CH4 IN4 | Digital Out | Kitchen Exhaust |
| GPIO 23 | Buzzer | Digital Out | Active buzzer; transistor for 5V high-current |
| 3.3V | DHT11 + LDR dividers | Power | Never 5V for dividers |
| 5V / VIN | PIR VCC, MQ-2 VCC, Relay VCC, Loads | Power | Regulated 5V 2A |
| GND | All modules | Power | **Common ground** |

**Why ADC1?** GPIO 32/34/35 are ADC1 — usable while Wi-Fi active (ADC2 conflicts).  
**Why GPIO13?** GPIO 0/2/5/12/15 are strapping pins sampled at boot — relay chatter on GPIO5.

---

## 2. Sensor Wiring Details

### 2.1 DHT11 (§7.1)
```
DHT11 VCC → 3.3V
DHT11 DATA → GPIO4
DHT11 GND → GND
```
Place central, away from fan / MQ-2 heater.

### 2.2 PIR ×2 (§7.2/7.3) — **Pre-check required (v1.1)**
```
PIR VCC → 5V
PIR OUT → GPIO27 (PIR#1) / GPIO26 (PIR#2)
PIR GND → GND
```
> **Multimeter step:** Power PIR alone, measure OUT (motion vs. no motion). Must be ≈3.3V, not 5V. Some clones output 5V — needs level shifter.

### 2.3 LDR ×2 (§7.4/7.5)
```
      3.3V
        |
       LDR
        |
        +----------→ GPIO34 (LDR1) / GPIO35 (LDR2)
        |
       10kΩ
        |
       GND
```
Bright → high ADC (2500–3500), Dark → low (200–800). Threshold `LDR_DARK_THRESHOLD` default 1500 — tune per room.

### 2.4 MQ-2 (§7.6) — **Divider mandatory**
```
MQ-2 VCC → 5V
MQ-2 GND → GND
MQ-2 AO --[10kΩ]--+------→ GPIO32
                 |
               [20kΩ]
                 |
                GND
```
Scales 5V→3.33V. Warm-up 2–3 min. Set threshold experimentally; never claim ppm.

### 2.5 Reed Switch (§7.7)
```
GPIO25 ── [REED] ── GND
pinMode(25, INPUT_PULLUP);
```
Magnet on door, reed on frame. `LOW=closed`, `HIGH=open`.

### 2.6 Buzzer (§9)
```
GPIO23 → Buzzer IN
GND    → Buzzer GND
```
For 5V high-current buzzer, use NPN transistor (2N2222) + base resistor.

---

## 3. Relay & Loads (§8)

```
ESP32              4-CH RELAY
GPIO13 ─────────→ IN1 ─┐
GPIO18 ─────────→ IN2 ─┤
GPIO19 ─────────→ IN3 ─┤
GPIO21 ─────────→ IN4 ─┘
GND    ─────────→ GND
5V     ─────────→ VCC

RELAY CONTACTS (NO = Normally Open, off when idle)
5V ── CH1 COM   CH1 NO ── Living Light + ── GND
5V ── CH2 COM   CH2 NO ── Bedroom Light + ── GND
5V ── CH3 COM   CH3 NO ── Room Fan + ── GND
5V ── CH4 COM   CH4 NO ── Kitchen Exhaust + ── GND
```

**Polarity test (v1.1 §8, §16.8):**
1. Power relay board alone.
2. Drive IN pin with ESP32 HIGH then LOW — observe which level clicks.
3. If active-LOW (most boards): `HIGH=off`, `LOW=on` — set all pins `HIGH` first in `setup()`.

**3.3V logic check:** Some relay boards need 5V trigger — ESP32 3.3V won't click reliably. Use 3.3V-compatible module or transistor driver.

---

## 4. Power Rails

| Rail | Components |
|------|------------|
| **3.3V** | DHT11, LDR dividers |
| **5V** | PIR VCC, MQ-2 VCC, Relay VCC, 5V loads (lights/fans) |
| **GND** | Common for ESP32 + all modules |

Regulated **5V 2A** DC supply recommended + 1000µF electrolytic bulk capacitor near ESP32 VIN.

---

## 5. Breadboard Layout (§10)

```
[ESP32 straddles center gap]
+-----------------------------+
| 5V rail (red) ──────────── |
| GND rail (blue/common) ─── |
| 3.3V section (separate) ── |
| LDR 10kΩ near GPIO34/35 ── |
| MQ-2 10k/20k near GPIO32 ─ |
| Relay/load wires separated  |
+-----------------------------+
Label every wire both ends!
House electronics: under/behind model, sensors via hidden holes.
```

---

## 6. Room Mapping (§5, §11)

- **Living Room:** PIR#1 (27) + LDR1 (34) + DHT11 (4) + Light (13) + Fan (19)
- **Bedroom:** PIR#2 (26) + LDR2 (35) + Light (18)
- **Kitchen:** MQ-2 (32) + Exhaust (21) + Buzzer (23)
- **Main Door:** Reed (25)

---

## 7. Fritzing-Style ASCII

```
         ┌──────────────┐
  DHT11 ─┤GPIO4    GPIO13├─ Relay CH1 Living Light
  PIR1  ─┤GPIO27   GPIO18├─ Relay CH2 Bedroom Light
  PIR2  ─┤GPIO26   GPIO19├─ Relay CH3 Room Fan
  Reed  ─┤GPIO25   GPIO21├─ Relay CH4 Exhaust
  LDR1  ─┤GPIO34   GPIO23├─ Buzzer
  LDR2  ─┤GPIO35     3.3V├─ DHT11, LDR dividers
  MQ-2  ─┤GPIO32       5V├─ PIR, MQ-2, Relay, Loads
         └──────┬───────┘
                GND (common)
```

Open `wiring_diagram.svg` in any browser for a printable color diagram to include in report/viva.
