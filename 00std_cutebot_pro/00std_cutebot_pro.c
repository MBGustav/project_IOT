#include "nrf52833.h"
#include <stdbool.h>
#include <stdio.h>
#include "i2c_cmd.h"
#include "sonar.h"
#include "robot_conf.h"
#include "map_led.h"





int main(void) 

{
    // initialization setup
    i2c_init();
    sonar_init();
    led_init();
    map_init();
    
    // light up the (0,0) cell on the map to show that we are at the base
    map_led_location(0,0);

    delay_ms(1000);

    clear_leds();

    while(1)
    {
        
        printf("Exploring grid...\n");
    
        clear_leds();
        explore_grid(); 
        
        int dist = (int)get_sonar_distance_cm();
        printf("Distance: %d cm\n", dist);
        delay_ms(1000);
    }  
    
}
