# Regen — Comportamento Pedale nelle varie condizioni

> Configurazione: `DEADZONE=25%`, `REGEN_THRESHOLD=25%`, `HYST=±3%`, `MODE=CONSERVATIVE (4.9 Nm max)`, `SPEED_MIN=3000 RPM`, `SPEED_CRITICAL=2000 RPM`

## Pedale fisico → `final_percent`

| Pedale fisico | `final_percent` |
|:---:|:---:|
| 0-25% | 0% (deadzone) |
| 25-100% | 0-100% (lineare) |

---

## Comportamento per condizione di velocità

### 🔴 Velocità < 2000 RPM — **Solo trazione**
Regen **impossibile** (latch forzatamente OFF).

| `final_pct` | Coppia |
|:---:|:---|
| 0% | 0 Nm (coasting) |
| 50% | +10.3 Nm |
| 100% | +20.6 Nm |

---

### 🟡 Velocità 2000-3000 RPM — **Trazione + regen solo se già attiva**
Non puoi **entrare** in regen, ma se eri già in regen (latch ON da prima) ci resti con fade-out.

| `final_pct` | Se latch OFF (trazione) | Se latch ON (regen) |
|:---:|:---|:---|
| 0% | 0 Nm (coasting) | -4.9 × k_vel Nm (fade-out) |
| 22% | +4.6 Nm | -0.6 × k_vel Nm |
| 28% | +5.8 Nm | latch OFF → +5.8 Nm |
| 100% | +20.6 Nm | — |

*k_vel = speed / 3000 (es. a 2500 RPM → k_vel = 0.83)*

---

### 🟢 Velocità > 3000 RPM — **Trazione + regen completa**
Puoi **entrare** in regen. Coppia regen al 100%.

| `final_pct` | Modo | Coppia |
|:---:|:---:|:---|
| 0% | **REGEN** | **-4.9 Nm** (100% regen conservative) |
| 12.5% | REGEN | -2.45 Nm |
| 22% | LATCH border | -0.6 Nm (regen) o +4.6 Nm (trazione) |
| 22-28% | LATCH band | Mantiene modo precedente |
| 28% | **TRACTION** | +5.8 Nm |
| 50% | TRACTION | +10.3 Nm |
| 100% | TRACTION | **+20.6 Nm** |

---

### Protezione aggiuntiva
| Condizione | Coppia |
|:---|:---:|
| speed < 10 RPM | **0 Nm** |
| speed > 16000 RPM | **0 Nm** |
| DC bus > 380V | Regen derata |
| DC bus > 395V | Regen = 0 |

---

## Schema riassuntivo completo

```
                    < 2000 RPM        2000-3000 RPM       > 3000 RPM
                 ┌──────────────┬──────────────────┬──────────────────┐
  Pedale 0%      │  0 Nm coast  │ -4.9×k Nm (se    │  -4.9 Nm         │
  (rilasciato)   │              │  già in regen)    │  REGEN PIENA     │
                 │              │ 0 Nm (se no)      │                  │
  ───────────────┼──────────────┼──────────────────┼──────────────────┤
  Pedale 22%     │  +4.6 Nm     │ dipende dal latch │ dipende dal latch│
  (latch enter)  │  trazione    │                  │                  │
  ───────────────┼──────────────┼──────────────────┼──────────────────┤
  Pedale 28%     │  +5.8 Nm     │  +5.8 Nm         │  +5.8 Nm         │
  (latch exit)   │  trazione    │  TRAZIONE         │  TRAZIONE        │
  ───────────────┼──────────────┼──────────────────┼──────────────────┤
  Pedale 100%    │  +20.6 Nm    │  +20.6 Nm        │  +20.6 Nm        │
  (pieno)        │  trazione    │  TRAZIONE         │  TRAZIONE        │
                 └──────────────┴──────────────────┴──────────────────┘
```
