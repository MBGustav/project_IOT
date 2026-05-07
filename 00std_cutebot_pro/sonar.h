#ifndef SONAR_H
#define SONAR_H

#include <stdbool.h>
#include <stdint.h>
#include "i2c_cmd.h"


void sonar_init();
int get_sonar_distance_cm(void); 

#endif // SONAR_H
