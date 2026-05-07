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

### Architecture overview:

<img src=arch-project.png>

### Logic Architecture
 ```text
 ├── 00std_cutebot_pro
     ├── 00std_cutebot_pro  - Main function
     ├── i2c_cmd            - Abstraction i2c protocol to move robot
     ├── map_led            - Abstraction map_led
     ├── Sonar              - Abstraction Sonar Control
     └── robot_conf         - Logic to object finder

```

## Communication (I2C)
We used **I2C** to send commands from the controller to the Cutebot Pro, such as basic motion and control instructions.

## API Overview (I²C + Motion Control Helpers)

This header-style snippet declares a small set of low-level I²C helpers plus higher-level motor/steering primitives. Implementations are typically split between:
- **I²C transport**: initializes the bus, sends raw buffers, and reads single bytes from a device register/address.
- **Motion layer**: converts user-friendly units (degrees, centimeters) into actuator commands, often via I²C writes to a motor/servo controller.

### I²C functions

- **`i2c_init()`**  
    Initializes the I²C peripheral and configures the required GPIO pins/pull-ups. Implementation usually sets bus speed (e.g., 100 kHz / 400 kHz) and enables the controller.

- **`i2c_send(uint8_t* buf, uint8_t buflen)`**  
    Sends a raw byte buffer over I²C.

- **`i2c_read_u8(uint8_t addr)`**  
    Reads and returns one byte from the given I²C device/register.

### Motion / control functions

- **`read_speed_raw(uint8_t motor)`**  
    Returns a motor speed value in **raw units** (controller/encoder dependent). Implementation typically queries a motor driver over I²C and returns the unscaled register value.

- **`trolley_steering(uint8_t turn, uint16_t angle)`**  
    Commands the steering mechanism: `turn` likely encodes direction (e.g., left/right/center) and `angle` is a target angle in controller units (or degrees if mapped). Implementation generally clamps to safe limits and writes a servo position setpoint.

- **`angle_run(uint8_t wheel, int16_t angle_deg)`**  
    Commands a specific wheel to rotate to/through an angle in **degrees**. Implementation usually converts degrees to ticks/steps, applies sign for direction, and issues a motion command.

- **`move_distance(uint8_t direction, uint8_t distance_cm)`**  
    Moves the platform a given distance in **centimeters**, with `direction` selecting forward/backward. Implementation typically converts distance to encoder counts and performs a closed-loop or timed move.

### Timing helpers

- **`delay_ms(uint32_t ms)` / `delay_us(uint32_t us)`**  
    Busy-wait or timer-backed delays used to meet peripheral timing requirements (e.g., I²C settle times, actuator update pacing). Implementations should be used carefully in real-time code to avoid blocking critical tasks.

> Notes: Exact units and valid ranges depend on the underlying motor/servo controller and how the I²C protocol is defined. Consider documenting expected ranges (e.g., max steering angle, motor indices) once confirmed from the implementation.
### API (quick)


```c
// I2C
void i2c_init(void);
void i2c_send(uint8_t* buf, uint8_t buflen);
uint8_t i2c_read_u8(uint8_t addr);

// Motion
uint8_t read_speed_raw(uint8_t motor);
void trolley_steering(uint8_t turn, uint16_t angle);
void angle_run(uint8_t wheel, int16_t angle_deg);
void move_distance(uint8_t direction, uint8_t distance_cm);

// Delays
void delay_ms(uint32_t ms);
void delay_us(uint32_t us);
```

## Sensing (Ultrasonic)
We used an **ultrasonic sensor** to detect and locate objects within the square map by measuring distance to nearby obstacles.


In this library, he provides distance measurement using an ultrasonic sensor (e.g. HC-SR04) with the Nordic Semiconductor timer for microsecond timing.

- **`sonar_init()`**: 
    Configures trigger and echo GPIO pins, then initializes TIMER0 at 1 MHz (1 µs resolution).

- **`fetch_current_time_us()` (internal helper)**
    Captures and returns the current timer value in microseconds.

- **`get_sonar_distance_cm()`**
    Sends a 10 µs trigger pulse, measures the echo pulse width, and converts it to distance in centimeters.
Returns the measured distance, or 999 if a timeout occurs (no echo / out of range).

### Notes:

Echo timeout: 5 ms (signal start), 25 ms (signal end, ~430 cm max range).
Distance is calculated using: distance_cm = echo_time_us / 58.
Includes a 100 ms delay between measurements.



## Led Matrix - Overview

Each LED in the matrix is located at the intersection of **Row pins (Y-axis)** select the active row and after **Column pins (X-axis)**  that select the active column. An LED lights up when:  Row = HIGH, Column = LOW  

## API Reference

**`void led_init(void)`**:
- Configures all row pins as outputs and sets them LOW (off), then  configures all column pins as outputs and sets them HIGH (off)

**`void clear_leds(void)`**
Turns off all LEDs on the matrix.  Setting all row pins LOW and all column pins HIGH.
**`void map_led_location(int x, int y)`**
Lights a single LED at position `(x, y)`.

**Behavior:**
1. Clears the entire display
2. Activates the selected row (sets it HIGH)
3. Activates the selected column (sets it LOW)
4. Lights only the LED at the intersection

### Pin Mapping

#### Rows (Y-axis)
| Y | GPIO Pin |
|---|----------|
| 0 | P0.19 |
| 1 | P0.24 |
| 2 | P0.15 |
| 3 | P0.22 |
| 4 | P0.21 |

#### Columns (X-axis)
| X | GPIO Pin |
|---|----------|
| 0 | P0.30 |
| 1 | P1.05 |
| 2 | P0.31 |
| 3 | P0.11 |
| 4 | P0.28 |



# Robot Mapping & Navigation Module

This module implements a simple **grid-based robot navigation and exploration system** using: 
- A global map representation
- Heading-based movement logic
- Obstacle detection via sonar
- Automatic return-to-base behavior

The robot operates on a **2D grid (`MAP_GRID_SIZE × MAP_GRID_SIZE`)** and maintains:

- Current position: `(robot_pos_x, robot_pos_y)`
- Current heading: `NORTH`, `SOUTH`, `EAST`, `WEST`
- A global map storing explored/unknown/occupied cells

It integrates with:
- Motor control (`move_distance`, `trolley_steering`)
- Ultrasonic sensor (`get_sonar_distance_cm`)
- LED display (`map_led_location`)


## Global State

- `robot_pos_x, robot_pos_y` → current robot position
- `robot_heading` → current direction
- `global_map[][]` → grid map of environment

## Map Functions

- **`map_init()`**
Initializes the entire grid as `unknown`.

- **`mark_field(x, y, field)`**
Marks a cell in the map, ignores invalid coordinates, updates `global_map[x][y]`.

- **`get_field(x, y)`**
Returns the value of a map cell and returns `-1` if out of bounds

- **`is_valid_cell(x, y)`**
Checks whether a coordinate is inside the grid.


## Movement Helpers

- **`get_next_cell(x, y, dir, *next_x, *next_y)`**
Computes the next grid cell based on direction:

| Heading | Next cell update |
|---------|-------------------|
| NORTH   | `y--`             |
| SOUTH   | `y++`             |
| EAST    | `x++`             |
| WEST    | `x--`             |


## Robot Navigation

- **`rotate_to(target)`**
Rotates robot to face a target direction.

**Logic:**
- Computes difference between current and target heading
- Chooses - right turn (90°), Left turn (90°), and 180° turn.
- Updates `robot_heading`

- **`update_position()`**
Updates robot coordinates after movement.
- Predicts next cell based on heading
- Rejects moves outside grid
- Updates position if valid
- Marks new cell as `explored`


- **`move_one_cell_forward()`**
Moves robot one grid cell forward.
Steps:
1. Ensures correct orientation
2. Moves physical distance (`CELL_CM`)
3. Updates internal position


## Obstacle Detection

### `check_obstacle_ahead()`
Uses ultrasonic sensor to detect obstacles.


## Exploration Behavior

### `explore_grid()`
Main exploration algorithm (zig-zag scan).

**Flow:**
1. Start at `(0,0)` facing EAST
2. Traverse row by row:
   - Move across full row
   - At row end:
     - Move down one cell
     - Reverse direction (zig-zag pattern)
3. Continuously checks for obstacles

**If obstacle detected:**
- Stops exploration
- Marks LED position
- Calls `return_to_base()`

## Return-to-Base

### `return_to_base()`
Navigates robot back to `(0,0)`.

Steps:
1. Rotate NORTH and move to top row
2. Rotate WEST and move to left column
3. Reach origin `(0,0)`
4. Perform a simple “celebration” rotation sequence


## Map States

Typical `field_t` values:
- `unknown` → unexplored
- `explored` → visited
- `occuppied` → obstacle detected


## Key Behaviors

- Grid-based navigation (no continuous coordinates)
- Zig-zag exploration pattern
- Immediate obstacle reaction
- Automatic recovery by returning to base
- Real-time map updates


## Notes

- The robot assumes a fully discrete grid world.
- Obstacle detection is threshold-based (not probabilistic).
- Exploration stops immediately on first detected obstacle.
- Map updates are internal only (no persistence).


## How it works

When the robot starts, it begins at the base position **(0,0)**. It then sweeps across the entire grid map to search for objects. Once an object is detected, its position is displayed on the **LED map**. After completing the scan, the robot returns to the initial base at **(0,0)**.


## Notes
- I2C is used for robot command/control.
- The I2C comm. do not have a queue. So, in order to handle different commands(left/right turn)
- Ultrasonic ranging is used for object detection on the map.
- The controller runs a simple state machine (e.g., **explore → avoid → scan → approach**) and sends the corresponding I2C commands to the robot.
- Motion commands include **forward/backward**, **turn left/right**, **stop**, and **speed control**; these are encoded as small command packets over I2C.



- **Limitations**: ultrasonic sensing has blind spots on angled/soft surfaces; tight corners can produce unstable readings, so conservative thresholds are used.



