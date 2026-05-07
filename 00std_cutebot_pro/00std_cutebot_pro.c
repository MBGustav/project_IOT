#include "nrf52833.h"
#include <stdbool.h>
#include <stdio.h>
#include "i2c_cmd.h"
#include "sonar.h"
#include "robot_conf.h"
#include "map_led.h"


/*
sources:
https://github.com/elecfreaks/pxt-Cutebot-Pro/blob/master/main.ts
I2C:
same a Cutebot
*/



int main(void) 

{
    i2c_init();
    sonar_init();
    led_init();
    map_led_location(2,2);
    delay_ms(1000);
    map_init();

    clear_leds();

    while(1)
    {
        // button to start exploration
        printf("Exploring grid...\n");
    
        clear_leds();
        explore_grid(); 
        
        int dist = (int)get_sonar_distance_cm();
        printf("Distance: %d cm\n", dist);
        delay_ms(1000);
//        delay_ms(10000);
    }  
    
}
