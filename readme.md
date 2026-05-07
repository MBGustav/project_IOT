# Cutebot Pro — IoT Lab

## Overview
This project controls a **Cutebot Pro** robot on a square map.

### Architecture Used:

<img src=arch-project.png>


## Communication (I2C)
We used **I2C** to send commands from the controller to the Cutebot Pro, such as basic motion and control instructions.

## Sensing (Ultrasonic)
We used an **ultrasonic sensor** to detect and locate objects within the square map by measuring distance to nearby obstacles.



## Notes
- I2C is used for robot command/control.
- Ultrasonic ranging is used for object detection on the map.
- The controller runs a simple state machine (e.g., **explore → avoid → scan → approach**) and sends the corresponding I2C commands to the robot.
- Motion commands include **forward/backward**, **turn left/right**, **stop**, and **speed control**; these are encoded as small command packets over I2C.




- **Limitations**: ultrasonic sensing has blind spots on angled/soft surfaces; tight corners can produce unstable readings, so conservative thresholds are used.