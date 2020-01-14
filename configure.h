#ifndef CONFIGURE_H_INCLUDED
#define CONFIGURE_H_INCLUDED

#include <stm32.h>
#include <gpio.h>

// USART defines
#define HSI_HZ 16000000U
#define I2C_SPEED_HZ 100000
#define LIS35DE_ADDR 0x1C
#define PCLK1_HZ HSI_HZ
#define PCLK1_MHZ 16

void configureUSART();
void configureNVIC();
void configureDMA();
void configureI2C();
void configureTIM();

inline void waitTillDeadline(uint16_t condition)
{
    int i = 0;
    const int max_deadline = 1000000;

    while (!(I2C1->SR1 & condition))
    {
        ++i;

        if (i > max_deadline)
        {
            I2C1->CR1 |= I2C_CR1_STOP;
            return;
        }
    }
}

#endif