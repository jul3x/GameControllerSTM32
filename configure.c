#include <delay.h>
#include <stm32.h>
#include "configure.h"

void configureUSART()
{
    GPIOafConfigure(GPIOA, 2, GPIO_OType_PP, GPIO_Fast_Speed, GPIO_PuPd_NOPULL, GPIO_AF_USART2);
    GPIOafConfigure(GPIOA, 3, GPIO_OType_PP, GPIO_Fast_Speed, GPIO_PuPd_UP, GPIO_AF_USART2);

    USART2->CR1 = USART_CR1_TE;
    USART2->CR2 = 0;

    uint32_t const baudrate = 9600U;
    USART2->BRR = (PCLK1_HZ + (baudrate / 2U)) / baudrate;
    USART2->CR3 = USART_CR3_DMAT;

    // USART Enabling
    USART2->CR1 |= USART_CR1_UE;
}

void configureNVIC()
{
    NVIC_EnableIRQ(EXTI15_10_IRQn);

    NVIC_EnableIRQ(DMA1_Stream6_IRQn);

    NVIC_EnableIRQ(I2C1_EV_IRQn);
    NVIC_EnableIRQ(I2C1_ER_IRQn);

    NVIC_EnableIRQ(TIM3_IRQn);
}

void configureTIM()
{
    TIM3->CR1 = 0;
    TIM3->PSC = 400;
    TIM3->ARR = 1000;
    TIM3->EGR = TIM_EGR_UG;
    TIM3->SR = ~(TIM_SR_UIF | TIM_SR_CC1IF | TIM_SR_CC2IF);
    TIM3->DIER = TIM_DIER_UIE | TIM_DIER_CC1IE | TIM_DIER_CC2IE;
    TIM3->CCR1 = 333; // 1/3 flag
    TIM3->CCR2 = 666; // 2/3 flag
    TIM3->CR1 |= TIM_CR1_CEN;
}

void configureDMA()
{
    DMA1_Stream6->CR = 4U << 25 |
        DMA_SxCR_PL_1 |
        DMA_SxCR_MINC |
        DMA_SxCR_DIR_0 |
        DMA_SxCR_TCIE;
    DMA1_Stream6->PAR = (uint32_t)&USART2->DR;

    DMA1->HIFCR = DMA_HIFCR_CTCIF6;
}

void configureI2C()
{
    GPIOafConfigure(GPIOB, 8, GPIO_OType_OD, GPIO_Low_Speed, GPIO_PuPd_NOPULL, GPIO_AF_I2C1);
    GPIOafConfigure(GPIOB, 9, GPIO_OType_OD, GPIO_Low_Speed, GPIO_PuPd_NOPULL, GPIO_AF_I2C1);

    I2C1->CR1 = 0;

    I2C1->CR2 = PCLK1_MHZ;
    I2C1->CCR = PCLK1_HZ / (I2C_SPEED_HZ << 1);
    I2C1->TRISE = PCLK1_MHZ + 1;
    I2C1->CR1 |= I2C_CR1_PE;

    // CTRL_REG1 0
    {
        I2C1->CR1 |= I2C_CR1_START;
        waitTillDeadline(I2C_SR1_SB);

        I2C1->DR = LIS35DE_ADDR << 1;
        waitTillDeadline(I2C_SR1_ADDR);
        I2C1->SR2;

        uint8_t reg = 0x20;
        I2C1->DR = reg;
        waitTillDeadline(I2C_SR1_TXE);

        uint8_t value = 0b01000111; // ENABLED 
        I2C1->DR = value;
        waitTillDeadline(I2C_SR1_BTF);
        I2C1->CR1 |= I2C_CR1_STOP;
    }

    Delay(10000);

    // CTRL_REG3 0
    {
        I2C1->CR1 |= I2C_CR1_START;
        waitTillDeadline(I2C_SR1_SB);

        I2C1->DR = LIS35DE_ADDR << 1;
        waitTillDeadline(I2C_SR1_ADDR);
        I2C1->SR2;

        uint8_t reg = 0x22;
        I2C1->DR = reg;
        waitTillDeadline(I2C_SR1_TXE);

        uint8_t value = 0b00000100;
        I2C1->DR = value;
        waitTillDeadline(I2C_SR1_BTF);
        I2C1->CR1 |= I2C_CR1_STOP;
    }
}

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