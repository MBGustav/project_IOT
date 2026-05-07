
#ifndef I2C_CMD_H
#define I2C_CMD_H

#include "nrf52833.h"
#include <stdbool.h>
#include <stdio.h>

#define I2C_ADDR (0x10)
#define CB_WHEEL_LEFT           (0x01)
#define CB_WHEEL_RIGHT          (0x02)
#define CB_WHEEL_BOTH           (0x03)
#define CB_TURN_LEFT            (0)
#define CB_TURN_RIGHT           (1)
#define CB_TURN_LEFT_IN_PLACE   (2)
#define CB_TURN_RIGHT_IN_PLACE  (3)


//Decl.
void i2c_send(uint8_t* buf, uint8_t buflen);
void i2c_init(void) ;
uint8_t i2c_read_u8(uint8_t addr);


uint8_t read_speed_raw(uint8_t motor);
void trolley_steering(uint8_t turn, uint16_t angle);
void angle_run(uint8_t wheel, int16_t angle_deg);
void delay_ms(uint32_t ms);
void delay_us(uint32_t ms);

void move_distance(uint8_t direction, uint8_t distance_cm);

#endif