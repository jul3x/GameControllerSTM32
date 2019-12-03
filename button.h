#ifndef BUTTON_H_INCLUDED
#define BUTTON_H_INCLUDED

#include <stm32.h>
#include <gpio.h>

// Button functions
typedef struct {
    uint16_t pin;
    GPIO_TypeDef * gpio;
    char *pressed_str;
    char *released_str;
    uint16_t is_reverse_logic;
} Button;

void configure(Button *button);

#endif // BUTTON_H_INCLUDED