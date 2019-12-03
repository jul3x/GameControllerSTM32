#include "button.h"

void configure(Button *button)
{
    GPIOinConfigure(button->gpio, button->pin, GPIO_PuPd_UP, EXTI_Mode_Interrupt, EXTI_Trigger_Rising_Falling);
}