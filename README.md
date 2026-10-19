# Smart Fan and Greenhouse Controller using ESP32

## Project Overview

An ESP32-based embedded systems project developed using ESP-IDF and Embedded C. The system monitors temperature using an LM75 digital temperature sensor and uses a potentiometer to adjust the target temperature.

The firmware implements temperature-based fan control using PWM, LED status indication, GPIO interrupt handling for manual control, and UART messages for debugging.

## Features

- Temperature monitoring using the LM75 sensor over I²C.
- Adjustable temperature setpoint using an ADC input.
- PWM-based fan speed control.
- Three status LEDs for operating-condition indication.
- GPIO interrupt-based button input.
- UART output for debugging and status monitoring.
- FreeRTOS-based application execution using ESP-IDF.

## Hardware Components

| Component | Purpose |
|---|---|
| ESP32 | Main microcontroller |
| LM75 | Digital temperature sensor |
| Potentiometer | Temperature setpoint adjustment |
| DC fan | Temperature regulation |
| Three LEDs | Status indication |
| Push button | Manual control input |

## Pin Configuration

| Function | ESP32 GPIO / Setting |
|---|---|
| Green LED | GPIO 4 |
| Yellow LED | GPIO 16 |
| Red LED | GPIO 17 |
| Fan PWM | GPIO 18 |
| Button input | GPIO 34 |
| I²C SDA | GPIO 21 |
| I²C SCL | GPIO 22 |
| LM75 I²C address | 0x48 |
| ADC input | ADC1 Channel 0 |
| PWM frequency | 1 kHz |
| PWM resolution | 10-bit |
| UART baud rate | 115200 |

**Note:** Confirm the GPIO mapping against your exact ESP32 variant. On the original ESP32, ADC1 Channel 0 maps to GPIO 36, and GPIO 34 requires external button biasing because it has no internal pull-up or pull-down.

## Working Principle

### 1. Temperature Measurement
The ESP32 communicates with the LM75 temperature sensor through the I²C interface and reads the temperature register.

### 2. Temperature Setpoint
The potentiometer provides an analog voltage to the ADC. The firmware maps the ADC reading to a target temperature range of approximately 20°C to 40°C.

### 3. Fan Control
The firmware compares the measured temperature with the target temperature and selects a PWM duty level.

| Temperature condition | Intended fan duty |
|---|---:|
| At or below target | 0% |
| Above target by less than 2°C | 30% |
| 2°C to less than 5°C above target | 60% |
| 5°C or more above target | 100% |

The manual button and automatic temperature-control logic require further refinement to ensure that manual operation behaves consistently.

### 4. LED Status Indication
Three LEDs indicate the temperature and fan operating conditions.

### 5. UART Monitoring
UART messages provide status information for debugging and monitoring the control logic.

## Software and Tools

- ESP-IDF
- Embedded C
- FreeRTOS
- GPIO driver
- ADC one-shot driver
- LEDC PWM driver
- I²C master driver
- UART driver

## Learning Outcomes

- Embedded C programming with ESP-IDF.
- I²C sensor interfacing.
- ADC-based analog input processing.
- PWM configuration and actuator control.
- GPIO interrupt handling.
- UART-based debugging.
- Integration of sensors and peripheral drivers.

## Project Status

The main firmware implementation has been drafted. Code review, manual-control refinements, error handling, and hardware validation remain pending.

## Future Improvements

- Organize firmware into reusable source and header files.
- Add button debounce and robust sensor-error handling.
- Refine manual and automatic operating modes.
- Improve temperature-control stability.
- Add hardware test results and demonstration images.

## Author

**Priya Udhayakumar**

Embedded Systems and PCB Design

