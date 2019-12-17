#include <gpio.h>
#include <stm32.h>
#include <delay.h>
#include <string.h>
#include "queue.h"
#include "button.h"
#include "configure.h"

Queue tx_buffer;
Button fire_button = {13, GPIOC, "P\r\n",  "F\r\n", 0};

uint8_t* i2c_buffer;
uint8_t i2c_value = 0;
uint8_t i2c_complete = 0;
int i2c_to_send_counter;
int i2c_to_receive_counter;
int i2c_current_position;
int connection_status = 0;

char *tab[256] = {
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "17",
    "18", "19", "20", "21", "22", "23", "24", "25", "26", "27", "28", "29", "30", "31", "32", "33", 
    "34", "35", "36", "37", "38", "39", "40", "41", "42", "43", "44", "45", "46", "47", "48", "49", 
    "50", "51", "52", "53", "54", "55", "56", "57", "58", "59", "60", "61", "62", "63", "64", "65", 
    "66", "67", "68", "69", "70", "71", "72", "73", "74", "75", "76", "77", "78", "79", "80", "81", 
    "82", "83", "84", "85", "86", "87", "88", "89", "90", "91", "92", "93", "94", "95", "96", "97", 
    "98", "99", "100", "101", "102", "103", "104", "105", "106", "107", "108", "109", "110", "111",
    "112", "113", "114", "115", "116", "117", "118", "119", "120", "121", "122", "123", "124", "125",
    "126", "127", "128", "129", "130", "131", "132", "133", "134", "135", "136", "137", "138", "139", 
    "140", "141", "142", "143", "144", "145", "146", "147", "148", "149", "150", "151", "152", "153",
    "154", "155", "156", "157", "158", "159", "160", "161", "162", "163", "164", "165", "166", "167",
    "168", "169", "170", "171", "172", "173", "174", "175", "176", "177", "178", "179", "180", "181",
    "182", "183", "184", "185", "186", "187", "188", "189", "190", "191", "192", "193", "194", "195",
    "196", "197", "198", "199", "200", "201", "202", "203", "204", "205", "206", "207", "208", "209",
    "210", "211", "212", "213", "214", "215", "216", "217", "218", "219", "220", "221", "222", "223",
    "224", "225", "226", "227", "228", "229", "230", "231", "232", "233", "234", "235", "236", "237",
    "238", "239", "240", "241", "242", "243", "244", "245", "246", "247", "248", "249", "250", "251",
    "252", "253", "254", "255"
 };

uint8_t registerX[] = {0x29};
uint8_t registerY[] = {0x2B};
uint8_t registerZ[] = {0x2D};

int main() {
    clear(&tx_buffer);

    // Initializations
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOCEN | RCC_AHB1ENR_DMA1EN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN | RCC_APB1ENR_I2C1EN | RCC_APB1ENR_TIM3EN;
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    TIM3->CR1 = 0;
    TIM3->PSC = 400;
    TIM3->ARR = 1000;
    TIM3->EGR = TIM_EGR_UG;
    TIM3->SR = ~(TIM_SR_UIF | TIM_SR_CC1IF | TIM_SR_CC2IF);
    TIM3->DIER = TIM_DIER_UIE | TIM_DIER_CC1IE | TIM_DIER_CC2IE;
    TIM3->CCR1 = 333;
    TIM3->CCR2 = 666;
    __NOP();

    // Buttons configuration
    configure(&fire_button);
    configureUSART();
    configureDMA();
    configureI2C();
    configureNVIC();

    TIM3->CR1 |= TIM_CR1_CEN;

    __NOP();

    // Main loop
    for (;;) {
        
      //send("\r\nZ: ");
      //i2c_send_read(1, 1, registerZ);

      //Delay(100000);
    }

    return 0;
}

void i2c_send_read(int bytes_to_send, int bytes_to_receive, uint8_t *buffer) {
  i2c_buffer = buffer;
  i2c_to_send_counter = bytes_to_send;
  i2c_to_receive_counter = bytes_to_receive;
  i2c_current_position = 0;
  connection_status = 0;
  i2c_complete = 0;

  I2C1->CR2 |= I2C_CR2_ITBUFEN | I2C_CR2_ITEVTEN | I2C_CR2_ITERREN;
  I2C1->CR1 |= I2C_CR1_START;
  I2C1->CCR = PCLK1_HZ / (I2C_SPEED_HZ << 1);
  I2C1->TRISE = PCLK1_MHZ + 1;
}

void send(char *text) {
    if ((DMA1_Stream6->CR & DMA_SxCR_EN) == 0 &&
        (DMA1->HISR & DMA_HISR_TCIF6) == 0)
    {
        DMASend(text);
    }
    else
    {
        enqueue(&tx_buffer, text);
    }
}

void DMASend(char *text) {
    DMA1_Stream6->M0AR = (uint32_t)text;
    DMA1_Stream6->NDTR = strlen(text);
    DMA1_Stream6->CR |= DMA_SxCR_EN;
}

void interrupt(uint16_t interr, uint16_t exti_line, Button *button) {
    if (interr & exti_line)
    {
        uint8_t is_released = ((button->gpio->IDR >> button->pin) & 1) ^ button->is_reverse_logic;
        char *text = is_released ? button->released_str : button->pressed_str;
        send(text);

        EXTI->PR = exti_line;
    }
}

void DMA1_Stream6_IRQHandler() {
    uint32_t isr = DMA1->HISR;
    if (isr & DMA_HISR_TCIF6)
    {
        DMA1->HIFCR = DMA_HIFCR_CTCIF6;

        if (!empty(&tx_buffer))
        {
            char *text = dequeue(&tx_buffer);
            DMASend(text);

            resetIfNeeded(&tx_buffer);
        }
    }
}

void EXTI15_10_IRQHandler(void) {
    uint16_t interr = EXTI->PR;
    interrupt(interr, EXTI_PR_PR13, &fire_button);
}

void I2C1_EV_IRQHandler() {
  if (i2c_to_send_counter > 0) {
    if (connection_status == 0 && (I2C1->SR1 & I2C_SR1_SB)) {
      connection_status = 1;
      I2C1->DR = LIS35DE_ADDR << 1;
      return;
    }

    if (connection_status == 1 && (I2C1->SR1 & I2C_SR1_ADDR)) {
      connection_status = 2;
      I2C1->SR2;

      I2C1->DR = i2c_buffer[i2c_current_position];
      i2c_current_position++;
      i2c_to_send_counter--;
      return;
    }

    if (connection_status == 2 && (I2C1->SR1 & I2C_SR1_TXE)) {
      I2C1->DR = i2c_buffer[i2c_current_position];
      i2c_current_position++;
      i2c_to_send_counter--;
      return;
    }

    send("Error [I2C1_EV_IRQHandler]: Dziwne przerwanie (if)\n\r");
    return;
  } 
  else if (i2c_to_receive_counter > 0) {

    if (connection_status == 2 && (I2C1->SR1 & I2C_SR1_BTF)) {
      I2C1->CR1 |= I2C_CR1_START;
      connection_status = 3;
      return;
    }

    if (connection_status == 3 && (I2C1->SR1 & I2C_SR1_SB)) {
      I2C1->DR = (LIS35DE_ADDR << 1) | 1U;
      I2C1->CR1 &= ~I2C_CR1_ACK;
      connection_status = 4;
      return;
    }

    if (connection_status == 4 && (I2C1->SR1 & I2C_SR1_ADDR)) {
      I2C1->SR2;
      I2C1->CR1 |= I2C_CR1_STOP;
      connection_status = 5;
      return;
    }

    if (connection_status == 5 && (I2C1->SR1 & I2C_SR1_RXNE)) {
      i2c_to_receive_counter--;
      i2c_value = I2C1->DR;
      send(tab[i2c_value]);
      i2c_complete = 1;
      return;
    }

    //printserial("Error [I2C1_EV_IRQHandler]: Dziwne przerwanie (else if)\n\r");
    return;
  } 
  else {
    send("FULL STOP\n\r");
    connection_status = 0;
    I2C1->CR1 |= I2C_CR1_STOP;
    I2C1->CR2 &= ~(I2C_CR2_ITBUFEN | I2C_CR2_ITEVTEN | I2C_CR2_ITERREN);
    i2c_complete = 1;
    return ;
  }
}

void I2C1_ER_IRQHandler() {
  send(" Err");
}

void TIM3_IRQHandler(void) {
  uint32_t it_status = TIM3->SR & TIM3->DIER;

  if (it_status & TIM_SR_UIF) {
    send("\r\nX");
    i2c_send_read(1, 1, registerX);
    TIM3->SR = ~TIM_SR_UIF;
  }
  if (it_status & TIM_SR_CC1IF) {
    send("Y");
    i2c_send_read(1, 1, registerY);
    TIM3->SR = ~TIM_SR_CC1IF;
  }
  if (it_status & TIM_SR_CC2IF) {
    send("Z");
    i2c_send_read(1, 1, registerZ);
    TIM3->SR = ~TIM_SR_CC2IF;
  }
}