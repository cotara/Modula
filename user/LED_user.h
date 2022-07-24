#include "stm32f4xx_gpio.h"

#define TOSENDSIZE 10000

void LEDInit(void);
void LEDOn(void);
void LEDOff(void);
void LEDToggle(void);
void Blink (int x, int on, int off);
