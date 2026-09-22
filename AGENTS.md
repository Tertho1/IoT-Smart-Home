# AGENTS.md — Mandatory Operating Rules

> **This file is the first thing any agent (human, AI, or assistant) must read before touching code, wiring, or docs. Non-compliance risks hardware damage (e.g., 5V on ESP32 GPIO).**

## 1. Prime Directive — Docs First, Then TODO

```
ALWAYS in this exact order:
1) Read this AGENTS.md (you are here)
2) Read TODO.md for current phase & next tasks
3) Read SPEC docs in priority order (§6 pin map is law)
4) Then act — with double-checks for every pin/config
```

**Never** start implementation from memory, ChatGPT defaults, or generic ESP32 examples. The project’s pin map is custom and v1.2-fixed (inherits v1.1 GPIO fix + v1.2 layout fix).

## 2. Canonical Source of Truth — Priority Order

| Priority | Document | What it defines | When to read |
|----------|----------|-----------------|--------------|
| **P0 — LAW** | `Smart_Home_Client_Design_ESP32_v1_2.docx` — §5, §6, §7, §8, §15 | **Final GPIO pin map, wiring dividers, relay wiring, architecture + v1.2 layout (door→living, exhaust→back)** | Before any firmware or wiring change |
| **P0 — LAW** | `archive/Smart_Home_Client_Design_ESP32_v1.1.docx` — §6 (archive) | Previous GPIO fix reference (superseded by v1.2) | Only for history |
| **P0 — LAW** | `docs/wiring_diagram.svg` + `docs/wiring_diagram.md` + `docs/circuit_diagram.svg` | Visual + text wiring; must match v1.2 §6 exactly | Before any breadboard work |
| **P1** | `Smart_Home_Client_Overview_ESP32.docx` | Client-facing summary (no budget, no wiring detail) | When answering “what will it do” for clients |
| **P1** | `TODO.md` | What is done / what is next / current blockers | After AGENTS.md, before any task |
| **P2** | `firmware/Smart_Home_ESP32_v1.2.ino` (top defines) | Implemented pin constants, thresholds, Wi-Fi | When editing firmware |
| **P2** | `docs/BOM.md` + `docs/BOM.csv` | Parts list, verified BD prices | When buying / substituting parts |
| **P3** | `README.md` + `firmware/README.md` + `docs/README` | Quick start, build sequence | For onboarding |

**Conflict rule:** If any doc disagrees on pins/wiring, `Smart_Home_Client_Design_ESP32_v1_2.docx §6` wins (v1.2 inherits v1.1 GPIO fix). Flag the conflict in `TODO.md` — do not silently pick a pin.

**Immutability rule — READ-ONLY:** `Smart_Home_Client_Design_ESP32_v1_2.docx` is the frozen client spec. **No agent or human may write, edit, overwrite, or re-save this file.** Treat it as read-only. Edits to design go to `docs/`, `firmware/`, or `Smart_Home_Client_Overview_ESP32.docx` only.

## 3. Pin Map — The Law (v1.2 Fixed — layout, v1.1 GPIO) — §6

**A single wrong GPIO can destroy a sensor or cause relay chatter on boot. Copy-paste this table — do NOT re-type from memory.**

```
SENSORS (inputs):
  GPIO  4  -> DHT11 DATA       (3.3V power)
  GPIO 27  -> PIR #1 OUT       Living — VERIFY 3.3V logic (§7.2) | ESP32 NOT 5V-tolerant
  GPIO 26  -> PIR #2 OUT       Bedroom — same check
  GPIO 25  -> Reed Switch      -> GND, pinMode(25, INPUT_PULLUP)  (OPEN=HIGH, CLOSED=LOW)
  GPIO 34  -> LDR #1 divider   ADC1, input-only, Wi-Fi safe
  GPIO 35  -> LDR #2 divider   ADC1, input-only
  GPIO 32  -> MQ-2 AO via 10k/20k divider  ADC1, scales 5V->3.33V — NEVER connect AO directly

ACTUATORS (outputs):
  GPIO 13  -> Relay CH1 IN1    Living Light   [MOVED from GPIO5 in v1.1 — GPIO5 is strapping pin → chatter]
  GPIO 18  -> Relay CH2 IN2    Bedroom Light
  GPIO 19  -> Relay CH3 IN3    Room Fan
  GPIO 21  -> Relay CH4 IN4    Kitchen Exhaust
  GPIO 23  -> Buzzer           Active buzzer; transistor if 5V high-current

POWER:
  3.3V -> DHT11 + LDR dividers   (DO NOT use 5V for dividers)
  5V/VIN -> PIR VCC, MQ-2 VCC, Relay VCC, 5V loads
  GND  -> COMMON for ESP32 + all modules
```

**Dividers (§7.4-7.6):**
```
LDR:   3.3V -> LDR -> ● GPIO34/35 -> 10k -> GND   Bright ~2500-3500, Dark ~200-800
MQ-2:  AO -> 10k -> ● GPIO32 -> 20k -> GND       Do NOT skip divider — ADC max is 3.3V
```

## 4. Double-Check Protocol — Before Every Change

**Every agent must run this checklist and state results before committing.**

### 4.1 Before editing `firmware/*.ino` or `*.h`
- [ ] Did I re-read §6 pin defines at top of `.ino` and confirm against `AGENTS.md §3`?
- [ ] Did I confirm relay polarity: `RELAY_ON = LOW` / `RELAY_OFF = HIGH` for active-LOW boards, and `digitalWrite(HIGH)` is first in `setup()`?
- [ ] Did I confirm I did NOT use strapping pins `0, 2, 5, 12, 15` for relay outputs?
- [ ] Did I confirm ADC pins `34,35,32` are ADC1 (Wi-Fi safe) and `34/35` are input-only (no output)?
- [ ] Did I keep `INPUT_PULLUP` for GPIO25 reed switch?

### 4.2 Before touching wiring / `docs/wiring_diagram.*`
- [ ] Did I measure PIR OUT with multimeter (≈3.3V on motion) before connecting to GPIO27/26?
- [ ] Did I verify MQ-2 divider (10k/20k) is in place before connecting AO to GPIO32?
- [ ] Did I test relay board alone to confirm active-LOW vs active-HIGH before coding?
- [ ] Did I keep relay/load wires separated from sensor signal wires (§10)?

### 4.3 Before any `git commit` or doc update
- [ ] Did I cross-check changed pins against `TODO.md` current phase?
- [ ] Did I update `TODO.md` status (`pending`→`in_progress`→`completed`) in same commit?

## 5. Forbidden Assumptions

*   **NEVER** assume a relay is active-HIGH — test it (§8).
*   **NEVER** assume PIR outputs 3.3V — some clones output 5V and will kill the ESP32.
*   **NEVER** power LDR divider from 5V — 3.3V only.
*   **NEVER** connect MQ-2 AO directly to ESP32 — always via divider.
*   **NEVER** drive a fan/light directly from a GPIO — always via relay NO contact.
*   **NEVER** invent a threshold — `LDR_DARK_THRESHOLD` (default 1500) and `MQ2_GAS_THRESHOLD` (default 1800) are tuned per room after warm-up.
*   **NEVER** use `GPIO5` for relay — use `GPIO13` (v1.1 fix).

## 6. Workflow — How to Work

1.  **Read** `AGENTS.md` → `TODO.md` → relevant SPEC §.
2.  **Plan** — if task has 3+ steps, create/update `TODO.md` and use `TodoWrite` (or checklist) — one `in_progress` at a time.
3.  **Implement** — small, verifiable increments (wire one sensor, flash, test via Serial/`/api/status`).
4.  **Verify** — run checklist §4, test via `Serial Monitor 115200` or `GET /api/status`, update `TODO.md`.
5.  **Document** — update `docs/` or `firmware/README.md` if behavior changes.

## 7. Build Sequence — Follow This Order (§16)

`Power rails → ESP32 flash → DHT11 → PIR×2 → LDR×2 → MQ-2 divider → Reed → Relay polarity test → Relay loads → Buzzer → Automation one feature at a time → Wi-Fi dashboard → Mount in house → Label wires → End-to-end demo (§22)`

Skipping order hides bugs (e.g., testing lights before LDR divider masks dark-threshold errors).

## 8. Safety — Read Before Powering

*   Only **5V DC** loads in prototype — **NEVER 220V AC**.
*   MQ-2 hot — mount off foam, no flame/gas leak demo.
*   Disconnect power before rewiring.
*   Common GND required; protect all ADC from >3.3V.

## 9. If Unsure — Stop and Ask

If a pin, divider value, or relay polarity is unclear after reading §6-§8, **do not guess**. Open an issue in `TODO.md` under `## Blockers` or ask the user for physical measurement / module photo.

---
*Violating this file’s pin map is considered a breaking change — treat it like a failed test.*
