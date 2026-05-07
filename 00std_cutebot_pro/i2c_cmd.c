#include "i2c_cmd.h"
#include "nrf52833.h"
#include <stdbool.h>
#include <stdio.h>



void i2c_init(void) {
   //  3           2            1           0
    // 1098 7654 3210 9876 5432 1098 7654 3210
    // .... .... .... .... .... .... .... ...A A: DIR:   0=Input
    // .... .... .... .... .... .... .... ..B. B: INPUT: 1=Disconnect
    // .... .... .... .... .... .... .... CC.. C: PULL:  0=Disabled
    // .... .... .... .... .... .DDD .... .... D: DRIVE: 6=S0D1
    // .... .... .... ..EE .... .... .... .... E: SENSE: 0=Disabled
    // xxxx xxxx xxxx xx00 xxxx x110 xxxx 0010 
    //    0    0    0    0    0    6    0    2 0x00000602
    NRF_P0->PIN_CNF[26]           = 0x00000602; // SCL (P0.26)
    NRF_P1->PIN_CNF[0]            = 0x00000602; // SDA (P1.00)

    //  3           2            1           0
    // 1098 7654 3210 9876 5432 1098 7654 3210
    // .... .... .... .... .... .... .... AAAA A: ENABLE: 5=Enabled
    // xxxx xxxx xxxx xxxx xxxx xxxx xxxx 0101 
    //    0    0    0    0    0    0    0    5 0x00000005
    NRF_TWI0->ENABLE              = 0x00000005;

    //  3           2            1           0
    // 1098 7654 3210 9876 5432 1098 7654 3210
    // .... .... .... .... .... .... ...A AAAA A: PIN:    26 (P0.26)
    // .... .... .... .... .... .... ..B. .... B: PORT:    0 (P0.26)
    // C... .... .... .... .... .... .... .... C: CONNECT: 0=Connected
    // 0xxx xxxx xxxx xxxx xxxx xxxx xx01 1010 
    //    0    0    0    0    0    0    1    a 0x0000001a
    NRF_TWI0->PSEL.SCL            = 0x0000001a;

    //  3           2            1           0
    // 1098 7654 3210 9876 5432 1098 7654 3210
    // .... .... .... .... .... .... ...A AAAA A: PIN:    00 (P1.00)
    // .... .... .... .... .... .... ..B. .... B: PORT:    1 (P1.00)
    // C... .... .... .... .... .... .... .... C: CONNECT: 0=Connected
    // 0xxx xxxx xxxx xxxx xxxx xxxx xx10 0000 
    //    0    0    0    0    0    0    2    0 0x00000020
    NRF_TWI0->PSEL.SDA            = 0x00000020;

    //  3           2            1           0
    // 1098 7654 3210 9876 5432 1098 7654 3210
    // AAAA AAAA AAAA AAAA AAAA AAAA AAAA AAAA A: FREQUENCY: 0x01980000==K100==100 kbps
    NRF_TWI0->FREQUENCY           = 0x01980000;

    //  3           2            1           0
    // 1098 7654 3210 9876 5432 1098 7654 3210
    // .... .... .... .... .... .... .AAA AAAA A: ADDRESS: 16
    // xxxx xxxx xxxx xxxx xxxx xxxx x001 0000 
    //    0    0    0    0    0    0    1    0 0x00000010
    NRF_TWI0->ADDRESS             = 0x10;
}

void i2c_send(uint8_t* buf, uint8_t buflen) 
{
    uint8_t i;

    i=0;
    NRF_TWI0->TXD                 = buf[i];
    NRF_TWI0->EVENTS_TXDSENT      = 0;
    NRF_TWI0->TASKS_STARTTX       = 1;
    i++;
    while(i<buflen) {
        while(NRF_TWI0->EVENTS_TXDSENT==0);
        NRF_TWI0->EVENTS_TXDSENT  = 0;
        NRF_TWI0->TXD             = buf[i];
        i++;
    }
    while(NRF_TWI0->EVENTS_TXDSENT==0);
    NRF_TWI0->TASKS_STOP     = 1;
}


uint8_t i2c_read_u8(uint8_t addr)
{
    uint8_t value = 0;

    // lê 1 byte do slave I2C
    // (NRF / bare-metal style genérico)

    NRF_TWI0->ADDRESS = addr;
    NRF_TWI0->TASKS_STARTRX = 1;
    // wait for byte
    while (NRF_TWI0->EVENTS_RXDREADY == 0);

    NRF_TWI0->EVENTS_RXDREADY = 0;

    value = (uint8_t)NRF_TWI0->RXD;

    NRF_TWI0->TASKS_STOP = 1;

    return value;
}





uint8_t read_speed_raw(uint8_t motor)
{
    uint8_t buf[7];

    buf[0] = 0x99;
    buf[1] = 0x05;
    buf[2] = motor; // 0x01 : left, 0x02: right
    buf[3] = 0x00;
    buf[4] = 0x00;
    buf[5] = 0x00;
    buf[6] = 0x88;

    i2c_send(buf, 7);

    return i2c_read_u8(I2C_ADDR);
}


void trolley_steering(uint8_t turn, uint16_t angle)
{
    uint8_t buf[7];

    uint16_t temp_angle = 0;
    uint8_t orientation = 0;
    uint8_t cmd = 0;

    // equivalente a pwmCruiseControl(0,0)
    delay_ms(1000);

    //---------------------------------------------------
    // mesma calibração da biblioteca oficial
    //---------------------------------------------------
    if (angle == 45)
        temp_angle = 150;

    else if (angle == 90)
        temp_angle = 320; //316

    else if (angle == 135)
        temp_angle = 450;

    else if (angle == 180)
        temp_angle = 630;

    else if (angle < 180)
        temp_angle = (uint16_t)(3.51f * angle); // total_pulses = 3.51 * angles

    else
        temp_angle = (uint16_t)(3.45f * angle);


    //---------------------------------------------------
    // seleciona comando
    //---------------------------------------------------
    if (turn == CB_TURN_LEFT)
    {
        orientation = CB_WHEEL_RIGHT;
        cmd = 0x04;
    }
    else if (turn == CB_TURN_RIGHT)
    {
        orientation = CB_WHEEL_LEFT;
        cmd = 0x04;
    }
    else
    {
        // left/right in place
        orientation = CB_WHEEL_BOTH;

        // comando escondido do firmware
        cmd = 0x17;

        temp_angle += 20;//ajuste para 90 degree
    }


    //---------------------------------------------------
    // monta payload
    //---------------------------------------------------
    buf[0] = 0x99;
    buf[1] = cmd;
    buf[2] = orientation;

    buf[3] = (temp_angle >> 8) & 0xFF;
    buf[4] = temp_angle & 0xFF;

    // direção
    if (turn == CB_TURN_RIGHT_IN_PLACE)
        buf[5] = 0x00;
    else
        buf[5] = 0x01;

    buf[6] = 0x88;


    //---------------------------------------------------
    // envia
    //---------------------------------------------------
    i2c_send(buf, 7);


    //---------------------------------------------------
    // espera firmware concluir
    //---------------------------------------------------
    delay_ms(500);

    while (1)
    {
        uint8_t speedL = read_speed_raw(1);
        uint8_t speedR = read_speed_raw(2);

        if ((speedL == 0) && (speedR == 0))
        {
            delay_ms(800);

            speedL = read_speed_raw(1);
            speedR = read_speed_raw(2);

            if ((speedL == 0) && (speedR == 0))
                break;
        }

        delay_ms(20);
    }

    delay_ms(500);
}




void angle_run(uint8_t wheel, int16_t angle_deg)
{
    uint8_t buf[7];

    uint16_t abs_angle;
    uint8_t direction;

    // para motores antes do comando
    // equivalente a pwmCruiseControl(0,0)
    // (se você já tiver essa função pode chamar)
    //stop_motor_step();

    if (angle_deg < 0)
    {
        abs_angle = (uint16_t)(-angle_deg);
        direction = 0x00;
    }
    else
    {
        abs_angle = (uint16_t)(angle_deg);
        direction = 0x01;
    }

    buf[0] = 0x99;
    buf[1] = 0x04;
    buf[2] = wheel;

    buf[3] = (abs_angle >> 8) & 0xFF;
    buf[4] = abs_angle & 0xFF;

    buf[5] = direction;

    buf[6] = 0x88;

    // envia comando
    i2c_send(buf, 7);

    // firmware original espera estabilizar
    delay_ms(1000);

    // espera ambos motores pararem
    while (1)
    {
        uint8_t speedL = read_speed_raw(1);
        uint8_t speedR = read_speed_raw(2);

        if ((speedL == 0) && (speedR == 0))
        {
            delay_ms(1000);

            speedL = read_speed_raw(1);
            speedR = read_speed_raw(2);

            if ((speedL == 0) && (speedR == 0))
            {
                break;
            }
        }

        delay_ms(20);
    }
}
void delay_us(uint32_t us)
{
    volatile uint32_t i;

    while (us--)
    {
        // 16 MHz → ~16 ciclos por us
        // loop precisa de compensação do overhead
        for (i = 0; i < 3; i++)
        {
            __asm__("nop");
        }
    }
}

void delay_ms(uint32_t ms)
{
    while (ms--)
    {
        delay_us(1000);
    }
}


void move_distance(uint8_t direction, uint8_t distance_cm)
{
    uint8_t buf[7];

    // equivalente ao pwmCruiseControl(0,0)
    // opcional: parar motores antes
    // cutebot_stop();

    buf[0] = 0x99;
    buf[1] = 0x03;
    buf[2] = direction;
    buf[3] = distance_cm;
    buf[4] = 0x00;
    buf[5] = 0x00;
    buf[6] = 0x88;

    // envia payload
    i2c_send(buf, 7);

    // velocidade nominal = 2 cm/s
    //delay_ms((distance_cm * 100) / 20);

    // margem extra igual ao código original
    delay_ms(800);
}