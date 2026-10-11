# EV Powertrain Control - Peacock Elettrica

Vehicle Control Unit (VCU) firmware, vehicle dynamics controls (MATLAB/Simulink) and AMK inverter configuration for **Peacock Elettrica**, the Formula SAE electric car of the **Polimarche Racing Team** (UNIVPM).

> **Working repository.** This is the development repository (`Zinale/ev-powertrain-control`).
> A full-history mirror lives in the team organization. The content will later be split into separate repositories (VCU firmware, vehicle controls, inverter configuration); until then, this repository is the reference.

---

## Contents

| Folder | Status | What it contains |
|---|---|---|
| `mcu-g474re-stm32/` | **Active** | VCU firmware for STM32G474RE (STM32CubeIDE, FreeRTOS, FDCAN) with the Simulink-generated `driveController` |
| `Controls/` | **Active** | Simulink vehicle model, `driveController` model, parameter scripts, tracks and plot tools |
| `AIPEX/` | Active | AMK inverter configurations and AIPEX PRO scope templates |
| `Documents/` | Reference | Datasheets, AMK manuals, FS Rules 2026, sensor and board documentation |
| `Tests/` | Reference | Bench and test projects (data collectors, Nucleo F756ZG test unit, ESP32 link, backups) |
| `mcu-g474re-stm32-old/` | Legacy | First G474RE project, superseded by `mcu-g474re-stm32/` |
| `mcu-stm32/` | Legacy | Previous MCU on Nucleo-F756ZG (CAN at 1 Mbit/s, `Config.h` build modes). Do not use for new work |
| `Regen_Pedal_Behavior.md` | Reference | Regen pedal behaviour tables (deadzone, latch, speed thresholds) |

---

## 1. VCU firmware (`mcu-g474re-stm32/`)

### Hardware

| Item | Value |
|---|---|
| MCU | STM32G474RET6 (Nucleo-G474RE), 170 MHz |
| Toolchain | STM32CubeIDE / STM32CubeMX (`mcu-g474re-stm32.ioc`) |
| RTOS | FreeRTOS (CMSIS-RTOS v2) |
| FDCAN1 "MOTOR" | PA11 RX / PA12 TX, classic CAN 500 kbit/s, AMK inverters |
| FDCAN2 "MAIN" | PB5 RX / PB6 TX, classic CAN 500 kbit/s, vehicle bus (BMS, Dash, PDM, SDC, IMU, LEM CAB500) |
| ADC1 | APPS1 (PB11), APPS2 (PB12), SAS steering (PB14), brake sensor (PB1) |
| TIM2 | Wheel speed sensors FL (PB3, CH2) and FR (PB10, CH3) |
| LPUART1 | PA2 / PA3, 115200 baud, debug serial |
| IWDG | Prescaler 16, reload 2500 (about 1.25 s), refreshed by DataManager |

### Source layout

```
mcu-g474re-stm32/Core/
├── Inc/ , Src/
│   ├── main.c, app_freertos.c        CubeMX init, task and mutex creation
│   ├── shared_data.*                 VehicleData_t shared structure + vehicle FSM states
│   ├── mutexes.*                     mutex_VehicleData, LPUART1, FDCAN1_MOTOR, FDCAN2_MAIN
│   ├── runtime_stats.*               CPU load per task (DWT based)
│   ├── Connectivity/  Can.*, Serial.*            CAN IDs, RX/TX, IMU config, serial
│   ├── Control/       inverter.*, motor_control.*, error_recovery.*,
│   │                  plausibility.*, drive_profiles.*, control_config.h
│   │   └── Simulink/  driveController.* (Embedded Coder output, do not edit by hand)
│   ├── Sensors/       adc_manager.*, apps.*, sas.*, bs.*, wss.*, filters.*
│   └── Tasks/         data_manager.c, drive_logic.c, data_logger.c
└── Startup/startup_stm32g474retx.s
```

### FreeRTOS tasks

| Task | Rate | Priority | Role |
|---|---|---|---|
| `DataManager` | 200 Hz (5 ms) | High | Reads ADC (APPS, SAS, brake), wheel speeds, CAN RX from inverters and vehicle bus; refreshes the watchdog |
| `DriveLogic` | 100 Hz (10 ms) | High | Vehicle FSM, R2D, plausibility checks, drive profiles, one `driveController_step()` per cycle, torque setpoints to the inverters |
| `DataLogger` | 10 Hz (100 ms) | Low | Telemetry and diagnostic frames on FDCAN2, runtime statistics every about 5 s |

### Vehicle state machine

`INIT` → `READY` → `RUNNING`, with `IMPLAUSIBILITY`, `CAR_ERROR` (CAN timeout of Inverter / BMS / Dash / PDM / IMU) and `ERROR`.

### Control modes (`Control/control_config.h`)

| Define | Meaning |
|---|---|
| `CONTROL_MODE_PASS_THROUGH` | Same torque left and right, no torque vectoring |
| `CONTROL_MODE_SIMPLE_TVC` | Simple torque vectoring proportional to the steering angle |
| `CONTROL_MODE_SIMULINK` | Full Simulink controller (yaw / TVC, slip control, grip estimator, regen) **(current setting)** |

Other switches in the same file: `INVERTER_LEFT_PRESENT`, `INVERTER_RIGHT_PRESENT`, `BENCH_TEST_MODE` (bypasses R2D arm conditions and TS shutdown).

> ⚠️ `BENCH_TEST_MODE` is currently set to `1U`. Set it to `0U` before running on the car.

### Plausibility checks (`Control/plausibility.h`)

11 debounced checks, reported as a bitmask on CAN `0x007`:
APPS/brake (EV.4.7), APPS disagreement, sensor range, over-power, over-current vs BMS limit, torque greater than pedal request, regen while accelerating, drive while braking, motor overspeed, DC bus voltage window, emulated pack fuse I²t (pack current from LEM CAB500, `0x3C2`).

### Drive profiles (`Control/drive_profiles.*`)

The steering wheel sends the engine map (ECO / NORMAL / GAS) and the parameter byte on `0x106`. `DriveProfiles_Update()` maps them to torque peak, regen level and the gains of regen, slip control, TVC and understeer gradient that are pushed into the Simulink model, together with the BMS current limits and the DC bus voltage.

### CAN summary

**FDCAN1 (inverters)**: AMK node 1 = right, node 2 = left. Setpoints `0x183 + node`, actual values from `0x282 + node` onwards (see `Can.h`).

**FDCAN2 RX**: `0x001` BMS error, `0x004`/`0x005` PDM errors, `0x100-0x102` IMU, `0x103` BMS, `0x106` Dash R2D / map / parameters, `0x107` SDC status, `0x3C2` LEM CAB500 current.

**FDCAN2 TX**:

| ID | Content | To |
|---|---|---|
| `0x002` / `0x003` | Inverter right / left error code and info | Dash, telemetry |
| `0x006` | MCU errors and status byte | Dash |
| `0x007` | Plausibility bitmask (U16 BE) | Telemetry |
| `0x104` | Wheel speeds, car speed, FSM state, R2D armed / active | Dash, PDM |
| `0x105` | Front brake pressure, brake light state | Dash, PDM |
| `0x108` | APPS final, APPS1, APPS2, steering angle | Telemetry |
| `0x109` / `0x110` / `0x111` | Control debug (torque, yaw, lambda, slip, grip estimator) | Telemetry |
| `0x250` / `0x251` | Inverter right / left real-time data | Telemetry |

Endianness: `0x104`, `0x105`, `0x007`, `0x109-0x111` are big-endian (as in the DBC); the other MCU TX frames are little-endian.

### Build and flash

1. Open STM32CubeIDE and import `mcu-g474re-stm32/` as an existing project.
2. If you change pins or peripherals, edit `mcu-g474re-stm32.ioc` and regenerate. Keep your code inside the `USER CODE` blocks.
3. Check `control_config.h` (control mode, inverters present, `BENCH_TEST_MODE`).
4. Build the `Debug` configuration and flash through the on-board ST-LINK.
5. `variables-for-debug.PNG` shows the useful Live Expressions for debugging.

---

## 2. Vehicle controls (`Controls/`)

MATLAB / Simulink environment for vehicle simulation and controller design.

| Item | Description |
|---|---|
| `peacockElettricaSimulink.slx` | Full vehicle model: 3DOF dual-track body, tyres, AMK motors, Pure Pursuit driver |
| `driveController.slx` | Controller model used for code generation (TVC, slip control, grip estimator, torque allocation, regen) |
| `driveController_ert_rtw/` | Embedded Coder output (copied into `mcu-g474re-stm32/Core/.../Control/Simulink/`) |
| `AMK_Derating.slx`, `remake.slx` | Supporting models |
| `setupParams.m`, `setupParamsSingle.m` | Vehicle and controller parameters (`Simulink.Parameter`, ExportedGlobal) |
| `tracks/` | `acceleration.m`, `skidpad.m`, `track1-3.m`, cone meshes |
| `plot_*.m`, `list_logsout_signals.m` | Plot tools for simulation results |
| `tireFit.m`, `tireSetup.m`, `FSAE_Tire_Combined*.tir`, `tire_data/` | Tyre model fitting |
| `PedalMap/`, `maps/`, `regen_fcn.m`, `pure_pursuit.m` | Pedal maps, regen function, path follower |
| `docs/`, `images/` | MathWorks guides, design PDFs (acceleration, skidpad, free lap), screenshots |
| `old/`, `esempi/`, `bases/` | Old versions and examples |

### Typical workflow

1. Run `setupParams.m`.
2. Choose a track script in `tracks/` and run `peacockElettricaSimulink.slx`.
3. Analyse results with the `plot_*.m` scripts; results go in `lap_data_simulations/`.
4. When the controller changes, generate code from `driveController.slx` with Embedded Coder and copy the generated files into the firmware `Control/Simulink/` folder. Keep parameter names aligned with `drive_profiles.c`.

---

## 3. AMK inverter configuration (`AIPEX/`)

| Folder | Content |
|---|---|
| `Invs_Configs/` | Inverter parameter configurations |
| `FIXED_speed/`, `FREE_torque/` | Bench test setups (fixed speed, free torque) |
| `Errors/` | Inverter error records |
| `Scope Templates/` | AIPEX PRO oscilloscope templates |

Tool: AMK AIPEX PRO. Licence and access data are **not** stored in this repository; ask the Software Division lead.

---

## 4. Documents and tests

- `Documents/`: AMK manuals (control loop, diagnostics, CAN messages, KW-R25 parameters, field weakening), motor datasheets and STEP files, APPS, brake sensor, IMU Aviorace, CAN transceiver, Nucleo-G474RE, FS Rules 2026.
- `Tests/`: `DataCollectorSoftware`, `FeatherDataCollectorBancoMotori`, `FixedCan - Banco Loccioni`, `ESP32-WIFI-BT-communication`, `ControlUnit_Nucleo_F756ZG`, `Backups`, sample logs and pinout images.

---

## Related

- Regen pedal behaviour: [`Regen_Pedal_Behavior.md`](Regen_Pedal_Behavior.md)
- Component specifications (CCU, PDM, Dashboard, ...) are kept in the team Teams / Trello.

## Clone

```bash
git clone https://github.com/Zinale/ev-powertrain-control.git
```

## Authors

- **Alessandro Zingaretti**, Head of Software Division, Polimarche Racing Team, UNIVPM

## Contacts

- LinkedIn: [polimarcheracingteam](https://www.linkedin.com/company/polimarcheracingteam/) | [Alessandro Zingaretti](https://www.linkedin.com/in/alessandro-zingaretti/)
- Instagram: [polimarcheracingteam](https://www.instagram.com/polimarcheracingteam/)
- Email: [formulasae@sm.univpm.it](mailto:formulasae@sm.univpm.it)