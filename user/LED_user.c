#include "LED_user.h"
#include "time_user.h"

uint8_t led_state=0;
void LEDInit(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  
  /* Enable the GPIO_LED Clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

  /* Configure the GPIO_LED pin */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
}


void LEDOn(void)
{
  GPIOC->BSRRH=GPIO_Pin_13;  
}
void LEDOff(void)
{
  GPIOC->BSRRL=GPIO_Pin_13;  
}
void LEDToggle(void)
{
  GPIOC->ODR ^= GPIO_Pin_13;
}

void Blink (int x, int on, int off)                      
{      
        while (x--)
        {
          LEDOn();
          delay_1_ms (on);
          LEDOff();
          delay_1_ms (off);
        }  
}
