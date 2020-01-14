# GameControllerSTM32
Repository contains code for microcontroller side of simple game controller.

Code was developed using `STM32F411xE` core and several additional electronic modules.

# Description
Controlling is based on accelerometer readings on microcontroller. Device continuously transmits its measurements to connected computer by using `USART` transmission. Leaning the plate with mounted device leads to change of accelerometer readings and provides an opportunity to interpret them as control events.  

Additionaly, `FIRE` button state is sent alongside with measurements. 

# Dependencies
* arm-eabi-gcc compiler
* stm32.h header file for stm32 code development
* gpio.h header file for GPIO module usage
* startup_stm32.c/.h files for startup of microcontroller handling  

Necessary paths for libraries are described in `Makefile`.
