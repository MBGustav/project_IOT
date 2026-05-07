# Cutebot Pro — IoT Lab

## Overview
This project controls a **Cutebot Pro** robot on a square map.


## Requirement
To build and flash this project you need **SEGGER Embedded Studio (SES)** according to your microbit.


### Install SEGGER Embedded Studio
1. Download SES from: https://www.segger.com/downloads/embedded-studio/
2. Install it for your OS (Windows/macOS/Linux).
3. Ensure the required toolchains are installed:
    - **GNU ARM Embedded Toolchain** (for ARM targets)
    - **J-Link Software Pack** (for flashing/debugging with SEGGER)
4. Launch SES and verify it starts correctly.

### Architecture Used:

<img src=arch-project.png>


## Communication (I2C)
We used **I2C** to send commands from the controller to the Cutebot Pro, such as basic motion and control instructions.

## Sensing (Ultrasonic)
We used an **ultrasonic sensor** to detect and locate objects within the square map by measuring distance to nearby obstacles.


## How it works

When the robot starts, it begins at the base position **(0,0)**. It then sweeps across the entire grid map to search for objects. Once an object is detected, its position is displayed on the **LED map**. After completing the scan, the robot returns to the initial base at **(0,0)**.


### Logic Architecture
 ```text
 ├── 00std_cutebot_pro
     ├── 00std_cutebot_pro  - Main function
     ├── i2c_cmd            - Abstraction i2c protocol to move robot
     ├── map_led            - Abstraction map_led
     ├── Sonar              - Abstraction Sonar Control
     └── robot_conf         - Logic to object finder

```




## Notes
- I2C is used for robot command/control.
- The I2C comm. do not have a queue. So, in order to handle different commands(left/right turn)
- Ultrasonic ranging is used for object detection on the map.
- The controller runs a simple state machine (e.g., **explore → avoid → scan → approach**) and sends the corresponding I2C commands to the robot.
- Motion commands include **forward/backward**, **turn left/right**, **stop**, and **speed control**; these are encoded as small command packets over I2C.




- **Limitations**: ultrasonic sensing has blind spots on angled/soft surfaces; tight corners can produce unstable readings, so conservative thresholds are used.



