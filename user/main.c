#include "stm32_ub_dac_dma.h"
#include "LED_user.h"
#include "time_user.h"

int main (){
        
    SysTick_Config(SystemCoreClock/1000);
  
    timers_init();
    //LEDInit();
   
    //Blink(10,1000,1000);
    // инициализация ЦАП в режиме DMA
  UB_DAC_DMA_Init(SINGLE_DAC1_DMA);

  // выход синусоидального сигнала на DAC1 (PA4)
  UB_DAC_DMA_SetWaveform1(DAC_WAVE1_SINUS);


  // Расчет частоты синусоидального сигнала в 1 Гц
  // Синусоидальный сигнал имеет 32 значения
  // f=84MHz/300/8750/32 = 1 Hz
  

  
    while(1){
    
    }

}