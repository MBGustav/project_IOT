#include <stdbool.h>
#include <stdio.h>
#include "string.h"

#include "map_led.h"
#include "nrf52833.h"


void led_init() {
    // Configura todas as LINHAS como saída (P0)
    uint32_t rows[] = {21, 22, 15, 24, 19};
    for(int i=0; i<5; i++) {
        NRF_P0->PIN_CNF[rows[i]] = 0x00000003; 
        NRF_P0->OUTCLR = (1 << rows[i]); // Começam desligadas
    }

    // Configura todas as COLUNAS como saída
    uint32_t cols_p0[] = {30, 31, 11, 28};
    for(int i=0; i<4; i++) {
        NRF_P0->PIN_CNF[cols_p0[i]] = 0x00000003;
        NRF_P0->OUTSET = (1 << cols_p0[i]); // Coluna em HIGH = LED desligado
    }
    // Coluna 1 (P1.05)
    NRF_P1->PIN_CNF[5] = 0x00000003;
    NRF_P1->OUTSET = (1 << 5);
}

void clear_leds() {
    // Apaga todas as linhas
    NRF_P0->OUTCLR = (1 << 21) | (1 << 22) | (1 << 15) | (1 << 24) | (1 << 19);
    // Coloca todas as colunas em HIGH (desliga fluxo)
    NRF_P0->OUTSET = (1 << 30) | (1 << 31) | (1 << 11) | (1 << 28);
    NRF_P1->OUTSET = (1 << 5);
}

void map_led_location(int x, int y) {
    clear_leds(); // Limpa estado anterior

    // Ativa a Linha (X) -> HIGH
    switch (y) {
        case 0: NRF_P0->OUTSET = (1 << 19); break;
        case 1: NRF_P0->OUTSET = (1 << 24); break;
        case 2: NRF_P0->OUTSET = (1 << 15); break;
        case 3: NRF_P0->OUTSET = (1 << 22); break;
        case 4: NRF_P0->OUTSET = (1 << 21); break;
    }

    // Ativa a Coluna (Y) -> LOW (CLR)
    switch (x) {
        case 0: NRF_P0->OUTCLR = (1 << 30); break;
        case 1: NRF_P1->OUTCLR = (1 << 5);  break;
        case 2: NRF_P0->OUTCLR = (1 << 31); break;
        case 3: NRF_P0->OUTCLR = (1 << 11); break;
        case 4: NRF_P0->OUTCLR = (1 << 28); break;
    }
}