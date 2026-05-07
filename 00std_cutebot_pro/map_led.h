#ifndef MAP_LED_H
#define MAP_LED_H

#include <stdbool.h>
#include <stdint.h>
#include "i2c_cmd.h"


void led_init();
void clear_leds();
void map_led_location(int x, int y); 

#endif // MAP_LED_H