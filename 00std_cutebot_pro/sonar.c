#include <stdbool.h>
#include <stdio.h>

#include "sonar.h"
#include "nrf52833.h"


// ===========================================================================
// Ultrasonic sensor (HC-SR04, TIMER0 at 1 MHz for 1us timing)
// ===========================================================================

#define US_TRIG_PIN  10        // P0.10 = micro:bit edge connector P8
#define US_ECHO_PIN  12        // P0.12 = micro:bit edge connector P12

void sonar_init(void) {
    NRF_P0->PIN_CNF[US_TRIG_PIN] = 0x00000003;   // output
    NRF_P0->OUTCLR = (1U << US_TRIG_PIN);
    NRF_P0->PIN_CNF[US_ECHO_PIN] = 0x00000000;   // input

    NRF_TIMER0->TASKS_STOP  = 1;
    NRF_TIMER0->MODE        = 0;     // timer
    NRF_TIMER0->BITMODE     = 3;     // 32-bit
    NRF_TIMER0->PRESCALER   = 4;     // 16 MHz / 2^4 = 1 MHz
    NRF_TIMER0->TASKS_CLEAR = 1;
    NRF_TIMER0->TASKS_START = 1;
}
static uint32_t now_us(void) {
    NRF_TIMER0->TASKS_CAPTURE[0] = 1;
    return NRF_TIMER0->CC[0];
}

int get_sonar_distance_cm(void) {
    // 10 us trigger pulse
    NRF_P0->OUTSET = (1U << US_TRIG_PIN);
    uint32_t t = now_us();
    while (now_us() - t < 10);
    NRF_P0->OUTCLR = (1U << US_TRIG_PIN);

    // wait for echo HIGH (timeout 5 ms)
    t = now_us();
    while ((NRF_P0->IN & (1U << US_ECHO_PIN)) == 0) {
        if (now_us() - t > 5000) return 999;
    }

    uint32_t echo_start = now_us();

    // wait for echo LOW (timeout 25 ms ~ 430 cm)
    while (NRF_P0->IN & (1U << US_ECHO_PIN)) {
        if (now_us() - echo_start > 25000) return 999;
    }

    uint32_t echo_us = now_us() - echo_start;

    delay_ms(100);
    return (int)(echo_us / 58);
}