# Full Circuit Diagram — Pin-Level (v1.1)

> **Visual:** `docs/circuit_diagram.svg` (open in browser, 1600×1100, printable)  
> **Law:** `Smart_Home_Client_Design_ESP32_v1.1.docx §6-§8` + `AGENTS.md §3` — this file expands that table to every module pin.

## 1. Master Pin Map — One Line Per Wire

| From (Module Pin) | → To (ESP32 / Rail) | Wire Color (suggested) | Notes |
|-------------------|---------------------|------------------------|-------|
| **DHT11 VCC** | `ESP32 3.3V` | Red (3.3V) | 3.3V only |
| **DHT11 DATA** | `GPIO 4` | Blue | Add 4.7k pull-up if bare sensor |
| **DHT11 GND** | `GND` (common) | Black |  |
| **PIR #1 VCC** | `5V` rail | Red (5V) | 5V |
| **PIR #1 OUT** | `GPIO 27` | Purple | **Measure ≈3.3V on motion before wiring** |
| **PIR #1 GND** | `GND` | Black |  |
| **PIR #2 VCC** | `5V` | Red (5V) |  |
| **PIR #2 OUT** | `GPIO 26` | Purple | Same 3.3V check |
| **PIR #2 GND** | `GND` | Black |  |
| **LDR #1 — top leg** | `3.3V` | Red (3.3V) |  |
| **LDR #1 — bottom leg + node** | `GPIO 34` + `10k → GND` | Green | Divider `3.3V→LDR→●34→10k→GND` |
| **LDR #2 — top leg** | `3.3V` | Red (3.3V) |  |
| **LDR #2 — bottom leg + node** | `GPIO 35` + `10k → GND` | Green | Same divider |
| **MQ-2 VCC** | `5V` | Red (5V) | Hot — keep off foam |
| **MQ-2 GND** | `GND` | Black |  |
| **MQ-2 AO** | `GPIO 32` via `10k →●→20k→GND` | Orange | **Never directly — scales 5V→3.3V** |
| **MQ-2 DO** | *Not connected* | — | AO only used |
| **Reed Switch Pin 1** | `GPIO 25` | Yellow | `pinMode(25, INPUT_PULLUP)` |
| **Reed Switch Pin 2** | `GND` | Black | OPEN=HIGH, CLOSED=LOW |
| **Relay VCC** | `5V` | Red (5V) |  |
| **Relay GND** | `GND` | Black |  |
| **Relay IN1** | `GPIO 13 *` | Orange | * v1.1 moved from GPIO5 (strapping) |
| **Relay IN2** | `GPIO 18` | Orange |  |
| **Relay IN3** | `GPIO 19` | Orange |  |
| **Relay IN4** | `GPIO 21` | Orange |  |
| **Relay CH1 COM** | `5V` | Red (5V) |  |
| **Relay CH1 NO** | `Living Light + → GND` | White | NO = off when idle |
| **Relay CH2 COM** | `5V` | Red |  |
| **Relay CH2 NO** | `Bedroom Light + → GND` | White |  |
| **Relay CH3 COM** | `5V` | Red |  |
| **Relay CH3 NO** | `Room Fan + → GND` | White |  |
| **Relay CH4 COM** | `5V` | Red |  |
| **Relay CH4 NO** | `Kitchen Exhaust + → GND` | White |  |
| **Buzzer + / IN** | `GPIO 23` | Yellow | Active buzzer; transistor if 5V high-current |
| **Buzzer - / GND** | `GND` | Black |  |
| **ESP32 VIN** | `5V` (regulated 2A) | Red (5V) | + 1000µF cap near VIN |
| **ESP32 3V3** | `LDR dividers + DHT11` | Red (3.3V) |  |
| **ESP32 GND** | `GND` common | Black | All modules share this |

## 2. Dividers — Exact Wiring (§7.4-7.6)

```
LDR #1:  3.3V ── LDR ──┬──► GPIO34 (ADC1, input-only)
                      └── 10k ──► GND
           Bright → ADC 2500-3500, Dark → 200-800

LDR #2:  same → GPIO35

MQ-2:    AO ──[10k]──┬──► GPIO32 (ADC1)
                    └──[20k]──► GND
           5V AO → 3.33V max at GPIO32
```

## 3. Relay — Polarity & Power

```
ESP32 GPIO ──► Relay IN1-4
Relay VCC ──► 5V,  Relay GND ──► GND
Most budget boards are ACTIVE-LOW:  LOW = ON (relay clicks), HIGH = OFF
Firmware: digitalWrite(pin, HIGH) as FIRST line in setup() — prevents all loads ON at boot
Test: power relay alone, drive IN with ESP32 HIGH/LOW and listen for click before wiring loads
Use NO (Normally Open): 5V → COM,  NO → Load+ → GND  (load OFF when relay idle)
```

## 4. Power Rails

*   **3.3V rail** → DHT11 + LDR dividers only (never 5V)
*   **5V rail** → PIR VCC, MQ-2 VCC, Relay VCC, 5V loads, ESP32 VIN
*   **GND** → common for ESP32 + all modules + loads
*   Regulated 5V 2A supply + 1000µF electrolytic near VIN (handles fan inrush)

## 5. Breadboard Tips (§10)

*   ESP32 straddles center gap, 5V on one rail, GND on the other, separate 3.3V section
*   Place `10k` LDR resistors close to GPIO34/35 nodes, `10k/20k` MQ-2 divider close to GPIO32
*   Keep relay/load (5V) wires away from sensor signal wires
*   Label every wire both ends, run sensor wires through hidden holes in house model

## 6. How to Use This Diagram

1.  Print `circuit_diagram.svg` (Ctrl+P in browser, landscape, 100%)
2.  Wire **one module at a time** in the order `TODO.md §16`: power → DHT11 → PIRs (measure 3.3V first) → LDR dividers → MQ-2 divider → Reed → Relay polarity test → loads → buzzer
3.  After each module, check `Serial 115200` or `GET /api/status` — don’t wire next until current reads correctly

---
*Full pin-level — if a wire isn’t in the table above, don’t connect it.*
