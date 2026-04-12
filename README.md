# MCU (Motor Control Unit) - Peacock Elettrica ⚡

This repository contains the Motor Control Unit firmware for the Polimarche Racing Team electric vehicle. It includes code for managing dual AMK inverters via CAN Bus, reading sensors via ADC with DMA, FSAE-compliant safety logic, single-pedal regenerative braking, and vehicle dynamics models in MATLAB/Simulink. The board used is the STM32 Nucleo-F756ZG.

> **Note:** This repository covers only the **MCU** and the MATLAB/Simulink vehicle dynamics models. The ECU (sensor acquisition board) has been moved to its own repository.

## Features
- ✅ **AMK Inverter Control**: Full state machine (IDLE → LV → HV → READY → RUNNING) with AMK 8.2.6 error recovery sequence and automatic retry logic.
- ✅ **Regenerative Braking**: Four-stage single-pedal regen with speed fade-out, battery power limit, and DC bus voltage derating.
- ✅ **FSAE Safety Compliance**: APPS plausibility checks (T11.8.9, T11.8.10) and MCU state machine with torque interlock.
- ✅ **ADC with DMA**: Continuous multi-channel reading (APPS1, APPS2, Steering) without CPU overhead.
- ✅ **Dual CAN Bus**: CAN1 for inverter protocol, CAN2 for vehicle telemetry (dashboard, BMS, IMU, R2D).
- ✅ **Signal Filtering**: Median + Moving Average cascade on APPS and SAS for noise rejection.
- ✅ **FreeRTOS Real-Time Architecture**: 1 kHz motor control task, 1 kHz sensor task, 10 Hz logging task.
- ✅ **Vehicle Dynamics Models**: MATLAB/Simulink models for slip control, torque vectoring, and straight-line control.

## 🔗 Repository Structure

The repository is organized into three main sections: **mcu-stm32** (STM32 firmware), **Controls** (MATLAB/Simulink models), and **Tests** (bench test infrastructure and data).

```
├── mcu-stm32/                       # STM32F756ZG firmware (main project)
│   ├── mcu-stm32.ioc                # STM32CubeMX configuration
│   └── Core/
│       ├── Inc/                     # Headers organized by subsystem
│       │   ├── Config.h             # Build modes, feature flags, parameters
│       │   ├── Communication/       # CAN bus & UART serial
│       │   ├── Drive/               # Inverter, torque control, regen, error recovery
│       │   ├── Safety/              # MCU state machine, device state, watchdog
│       │   ├── Sensors/             # ADC/DMA, APPS, steering angle
│       │   └── Tasks/               # FreeRTOS task definitions
│       └── Src/                     # Implementations (mirrors Inc/ structure)
│
├── Controls/                        # MATLAB/Simulink vehicle dynamics models
│   ├── SlipControl/                 # Traction/slip ratio control
│   ├── StraightLineControl/         # Straight-line stability
│   └── TorqueVectoring/             # Torque vectoring with 3D car model
│
├── AIPEX/                           # AMK inverter configuration files (.aipex)
│   ├── FIXED_speed/                 # Speed control mode configurations
│   ├── FREE_torque/                 # Torque control mode configurations
│   ├── Scope Templates/             # AipexPRO oscilloscope templates
│   └── Errors/                      # Error log files
│
├── Documents/                       # Datasheets, manuals, AipexPRO guides
│
├── Tests/                           # Test infrastructure & bench data
│   ├── DataCollectorSoftware/       # Python/MATLAB data logging & analysis
│   ├── ControlUnit_Nucleo_F756ZG/   # Alternative board test project
│   ├── ESP32-WIFI-BT-communication/ # Wireless telemetry via ESP32
│   ├── FeatherDataCollectorBancoMotori/ # Feather CAN data logger
│   └── FixedCan - Banco Loccioni/   # Last bench test in Speed-Control mode
│
└── Regen_Pedal_Behavior.md          # Detailed regen pedal behavior documentation
```

### MCU (Motor Control Unit)
The firmware on the STM32F756ZG manages AMK inverter control in real time:
- **Drive modules**: AMK state machine, torque control loop, error recovery, regenerative braking
- **Sensor acquisition**: DMA-based ADC reading, APPS and steering processing
- **CAN communication**: Inverter setpoints on CAN1, vehicle telemetry on CAN2
- **Safety**: MCU state machine with torque interlock, APPS plausibility, watchdog

### Controls
MATLAB/Simulink models developed to study and simulate vehicle dynamics:
- **SlipControl**: Traction/slip ratio control algorithms
- **StraightLineControl**: Straight-line torque distribution
- **TorqueVectoring**: Full torque vectoring model with 3D vehicle dynamics

### Tests
Development, bench testing, and data collection infrastructure:
- **DataCollectorSoftware**: Python CSV logger + MATLAB analysis scripts
- **ControlUnit_Nucleo_F756ZG**: Alternative MCU implementation for testing
- **ESP32-WIFI-BT-communication**: Wireless data streaming from bench
- **FixedCan - Banco Loccioni**: Last bench test configuration (Speed-Control mode, Loccioni dyno)

## 🏗️ Detailed Control Unit Structure

The control unit implementation is located in `mcu-stm32/`. The architecture follows a modular design with feature-specific subfolders. Subfolders within `Inc/` contain headers (.h), while subfolders within `Src/` provide the corresponding sources (.c).

```
└── mcu-stm32/
    ├── mcu-stm32.ioc                  # STM32CubeMX configuration file
    └── Core/
        ├── Inc/
        │   ├── Config.h               # Build modes, feature flags, all parameters
        │   ├── Communication/
        │   │   ├── Can.h              # CAN1 (inverters) + CAN2 (vehicle) TX/RX
        │   │   └── Serial.h           # DMA UART logging (UART3, UART5, UART6)
        │   ├── Drive/
        │   │   ├── Inverter.h         # AMK state machine + CAN message decode
        │   │   ├── BaseControlMotor.h # Torque control loop, rate limiting, LEDs
        │   │   ├── ErrorRecovery.h    # AMK 8.2.6 error reset state machine
        │   │   └── Regen.h            # 4-stage regenerative braking
        │   ├── Safety/
        │   │   ├── MCU_State.h        # System state machine (INIT→READY→RUNNING/ERROR)
        │   │   ├── device_state.h     # Device state tracking
        │   │   └── watchdog.h         # IWDG refresh
        │   ├── Sensors/
        │   │   ├── ADC_Manager.h      # Centralized DMA ADC handling
        │   │   ├── APPS.h             # Accelerator Pedal Position Sensors (FSAE T11.8)
        │   │   └── SAS.h              # Steering Angle Sensor with filtering
        │   └── Tasks/
        │       ├── motors_manager.h   # 1 kHz inverter control task
        │       ├── data_manager.h     # 1 kHz sensor sampling task
        │       └── data_logger.h      # 10 Hz UART/CAN2 telemetry task
        └── Src/
            ├── Communication/
            │   ├── Can.c
            │   └── Serial.c
            ├── Drive/
            │   ├── Inverter.c
            │   ├── BaseControlMotor.c
            │   ├── ErrorRecovery.c
            │   └── Regen.c
            ├── Safety/
            │   ├── MCU_State.c
            │   ├── device_state.c
            │   └── watchdog.c
            ├── Sensors/
            │   ├── ADC_Manager.c
            │   ├── APPS.c
            │   └── SAS.c
            ├── Tasks/
            │   ├── motors_manager.c
            │   ├── data_manager.c
            │   └── data_logger.c
            └── main.c
```

## 🧪 Build Modes & Testing

The project supports different build modes configured in `Config.h`. This allows testing with limited hardware availability.

### Test Mode: Single ADC (`TEST_MODE_SINGLE_ADC`)
Enable when only **ADC1 CH0 (PA0)** is available:

```c
// In Config.h — uncomment to enable:
#define TEST_MODE_SINGLE_ADC
```

**Effects:**
- Uses only 1 ADC channel instead of 3
- APPS sensor 2 is simulated (duplicates sensor 1 value)
- Sensor mismatch plausibility check **disabled** (can't verify with 1 sensor)
- Range check **disabled**
- Deadzone 30% still active for realistic pedal behavior

### Test Mode: Full (`TEST_MODE_FULL`)
Enable for bench testing with all ADC channels but relaxed safety thresholds:

```c
// In Config.h — uncomment to enable:
#define TEST_MODE_FULL
```

**Effects:**
- All 3 ADC channels active (APPS1, APPS2, Steering)
- Implausibility threshold relaxed to 20% (instead of 10%)
- Implausibility timeout extended to 500 ms (instead of 100 ms)

### Production Mode (`RELEASE_MODE`)
Comment out all test modes for full FSAE compliance:

```c
// In Config.h — comment out all test modes:
//#define TEST_MODE_SINGLE_ADC
//#define TEST_MODE_FULL
#define RELEASE_MODE
```

**Effects:**
- All 3 ADC channels active
- Full APPS plausibility check: mismatch >10% for >100 ms → shutdown
- Full range check (open/short circuit detection)
- Deadzone 30% active
- All FSAE safety rules enforced

## 🎮 Module Descriptions

### APPS (Accelerator Pedal Position Sensors)
Located in `Sensors/APPS.h` and `Sensors/APPS.c`:
- **Dual sensor reading**: Two redundant potentiometers with independent transfer functions
- **Signal filtering**: Median filter (11 samples) + Moving Average (40 samples) cascade
- **Normalization**: Converts raw ADC values to 0–100% pedal position
- **FSAE Plausibility checks**:
  - **T11.8.9**: Sensor mismatch >10% for >100 ms → motor shutdown
  - **T11.8.10**: Auto-reset when pedal returns below 5%
  - **Out-of-range detection**: Open/short circuit with ADC bounds checking

### ADC Manager
Located in `Sensors/ADC_Manager.h` and `Sensors/ADC_Manager.c`:
- **DMA circular mode**: Continuous multi-channel ADC without CPU intervention
- **Thread-safe access**: Getter functions for other modules
- **Channels**: APPS1 (PA0), APPS2 (PA1), Steering (PC3)

### Inverter Control
Located in `Drive/Inverter.h` and `Drive/Inverter.c`:
- **AMK state machine**: IDLE → LV_ACTIVE → HV_ACTIVE → READY → RUNNING
- **CAN message decoding**: Status words (SM1–SM5), speed, torque, temperatures, power
- **AMK protocol**: Control word bits for enable, DC activation, error reset

### Error Recovery
Located in `Drive/ErrorRecovery.h` and `Drive/ErrorRecovery.c`:
- **AMK 8.2.6 sequence**: `bInverterOn=0` → `bErrorReset=1` (50 ms hold) → `bErrorReset=0` (falling edge triggers reset)
- **Retry logic**: Up to 5 automatic recovery attempts with 100 ms cooldown between tries
- **Per-inverter context**: Independent recovery state for each motor node

### Base Control Motor
Located in `Drive/BaseControlMotor.h` and `Drive/BaseControlMotor.c`:
- **Dynamic torque limiting**: AMK formula based on speed and DC bus voltage (`M = P / (2π × N/60)`)
- **Rate limiting**: Smooth torque transitions (10 units per ms)
- **Status LED management**: Visual feedback for error and running states

### Regenerative Braking
Located in `Drive/Regen.h` and `Drive/Regen.c`:
- **4-stage calculation**: Pedal-dependent → speed fade-out → battery power limit → DC bus derating
- **Latch hysteresis**: Entry/exit thresholds to prevent regen chattering at low speed
- **Configurable intensity**: CONSERVATIVE / BALANCED / AGGRESSIVE modes in `Config.h`

### CAN Communication
Located in `Communication/Can.h` and `Communication/Can.c`:
- **CAN1**: Inverter protocol — setpoints TX at 1 kHz, status RX at 1 kHz
- **CAN2**: Vehicle telemetry — BMS, IMU, R2D RX; error snapshots and sensor data TX
- **Hardware**: SN65HVD230 CAN transceiver (×2)

### Serial Communication
Located in `Communication/Serial.h` and `Communication/Serial.c`:
- **DMA-based transmission**: Non-blocking TX on UART3, UART5, UART6
- **Per-channel enable**: Each UART can be independently enabled/disabled in `Config.h`
- **Mutex-protected**: Thread-safe printf-style logging from FreeRTOS tasks

## ⏱️ FreeRTOS Task Architecture

The system uses FreeRTOS with three dedicated tasks. All shared data is protected via FreeRTOS mutexes.

### Task Overview

| Task               | Period  | Responsible for                                           |
|--------------------|---------|-----------------------------------------------------------|
| `MotorsManagerTask` | 10 ms  | Inverter state machine, torque command, error recovery, regen |
| `DataManagerTask`  | 4 ms    | ADC reading, APPS/SAS processing, sensor filtering        |
| `DataLoggerTask`   | 100 ms  | UART serial output and CAN2 telemetry                     |

### Motors Manager Task (10 ms)
The core control loop executed every 10 ms:

```c
void MotorsManagerTask(void *argument)
{
    for (;;)
    {
        // 1. Read sensor data (pedal %, steering %)
        // 2. For each inverter:
        //    a. Decode CAN status (Inverter_ProcessReceivedMessage)
        //    b. Run state machine + error recovery (Motor_ProcessInverterControl)
        //    c. If RUNNING: compute torque setpoint with regen + rate limit
        //    d. Send CAN command (Inverter_BuildCommand + Can_SendInverterCommand)
        osDelay(INVERTERS_TASK_PERIOD_MS);
    }
}
```

### Data Manager Task (4 ms)
Runs ADC acquisition and sensor processing every 1 ms:

```c
void DataManagerTask(void *argument)
{
    for (;;)
    {
        // 1. Read DMA ADC buffer (ADC_Manager_GetValues)
        // 2. Process APPS (APPS_Process) — filtering + plausibility
        // 3. Process SAS  (SAS_Process)  — filtering + normalization
        // 4. Update MCU state machine
        osDelay(1);
    }
}
```

### Data Logger Task (100 ms)
Outputs diagnostics and telemetry every 100 ms:

```c
void DataLoggerTask(void *argument)
{
    for (;;)
    {
        // 1. Log inverter data via UART (Serial_Log)
        // 2. Transmit CAN2 telemetry messages (APPS%, steering%, inverter data, errors)
        osDelay(100);
    }
}
```

## 🛠️ Commands & Installation

### MCU Control Unit (STM32)

1. **Clone the repository**:
   ```bash
   git clone https://github.com/PolimarcheRacingTeam/ev-powertrain-control.git
   ```
2. **Open the project with STM32CubeIDE**
3. **Navigate to `mcu-stm32/`** (the main firmware project)
4. **Select build mode** in `Core/Inc/Config.h` (see Build Modes section)
5. **Compile and upload** to the Nucleo-F756ZG board
6. **Monitor serial output** at 115200 baud (e.g., using Termite or PuTTY on UART3)

### AMK Inverter Configuration (AIPEX)

1. **Install AipexPRO** (refer to `Documents/AipexPRO/`)
2. **Load configuration** from `AIPEX/FREE_torque/` (torque control) or `AIPEX/FIXED_speed/` (speed control)
3. **Connect to AMK inverter** via CAN bus
4. **Use scope templates** from `AIPEX/Scope Templates/` for live monitoring

### Data Collection (Bench / Tests)

1. **Navigate to `Tests/DataCollectorSoftware/`**
2. **Run the Python logger** `csv_socket_logger.py` — streams CSV data from ESP32 over Wi-Fi
3. **Analyze data** using `analyze_test_log.m` in MATLAB

## 🔧 Hardware Configuration

### STM32F756ZG Peripherals
(see `mcu-stm32/mcu-stm32.ioc` for full CubeMX configuration)

- **ADC1**: 3-channel DMA circular mode — APPS1 (PA0), APPS2 (PA1), Steering (PC3)
- **CAN1**: Inverter communication at 1 Mbit/s (SN65HVD230 transceiver)
- **CAN2**: Vehicle telemetry at 500 kbit/s (SN65HVD230 transceiver)
- **UART3**: PC debug serial at 115200 baud (DMA TX)
- **UART5**: Data logger / ESP32 at 115200 baud (DMA TX)
- **UART6**: External device (DMA TX)
- **GPIO**: LD2 (green), LD3 (red) status LEDs; R2D signal input

### CAN Bus Hardware
- **Transceiver**: SN65HVD230 (3.3 V compatible), ×2 for CAN1 + CAN2
- **Baud Rates**: CAN1 @ 1 Mbit/s (AMK inverter), CAN2 @ 500 kbit/s (vehicle)
- **Termination**: 120 Ω at both ends of each bus

### AMK Motor & Inverter
- **Motor**: AMK DD series — 9.8 Nm nominal, 21 Nm peak, 16000 RPM max
- **Inverter**: AMK KW-R25 — CAN-based torque control
- **CAN node IDs**: Right inverter = 1 (0x184/0x284), Left inverter = 2 (0x185/0x285)

## 🏎️ FSAE Rules Compliance

### APPS Plausibility (T11.8.9)
- Dual redundant sensors with independent transfer functions
- If sensor mismatch exceeds 10% for more than 100 ms → motor power immediately cut
- Configurable in `APPS.h`:
  ```c
  #define APPS_IMPLAUSIBILITY_THRESHOLD_PERCENT   10U   // Difference threshold [%]
  #define APPS_IMPLAUSIBILITY_TIMEOUT_MS          100U  // Time before failure [ms]
  ```

### APPS Reset (T11.8.10)
- After an implausibility event, torque cannot be restored until the pedal returns below 5%
- Prevents unintended torque restoration after driver removes foot

### Out-of-Range Detection (EV.4.7)
- Each ADC channel checked for open-circuit (value too low) and short-circuit (value too high)
- Detection uses configurable ADC margin counts
- Fault triggers shutdown independently from plausibility check

## 📚 Documentation References

### Internal Documentation
- **AMK Inverter**: `Documents/` — AipexPRO guides, training materials, password info
- **Regen behavior**: `Regen_Pedal_Behavior.md` — full pedal-speed regen interaction map
- **MCU Configuration**: `mcu-stm32/mcu-stm32.ioc` — STM32CubeMX project configuration

### FSAE Rules Reference
- **Formula SAE Rules**: [FSAE Online](https://www.fsaeonline.com/)
- **T11.8.9 / T11.8.10**: APPS sensor plausibility and reset conditions
- **EV.4.7**: Accelerator pedal position sensor requirements

### External Resources

- **NUCLEO F756ZG**:  
  📄 [Pinout](https://os.mbed.com/platforms/ST-Nucleo-F756ZG/)

- **SN65HVD230 CAN Transceiver**:  
  🎞️ [Video Tutorial 1](https://youtu.be/KHNRftBa1Vc?si=2C673-Au-6wTgh2l)  
  🎞️ [Video Tutorial 2](https://www.youtube.com/watch?v=-lcrrRrKdFg)

- **STM32 ADC with DMA**:  
  🎞️ [Video Tutorial](https://www.youtube.com/watch?v=zipjCtiHYr8)  
  📄 [In-depth Guide](https://deepbluembedded.com/stm32-adc-multi-channel-scan-continuous-mode-dma-poll-examples/)

- **AMK Motor & Inverter**:  
  📄 [AipexPRO Documentation](Documents/AipexPRO/)

## 👥 Authors

- **Alessandro Zingaretti** — *Polimarche Racing Team, UNIVPM*

## 👀 Interested in Learning More?

If you have any questions, would like to discuss this project further, or are interested in potential collaboration opportunities, please feel free to connect with us:

- LinkedIn: [polimarcheracingteam](https://www.linkedin.com/company/polimarcheracingteam/posts/?feedView=all)
- Instagram: [polimarcheracingteam](https://www.instagram.com/polimarcheracingteam/)
- Email: [formulasae@sm.univpm.it](mailto:formulasae@sm.univpm.it)

