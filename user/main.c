
#include "LED_user.h"
#include "time_user.h"

int main (){
    
    uint32_t temp_t = HSE_VALUE;
    
    SysTick_Config(SystemCoreClock/1000);
  
    timers_init();
    LEDInit();
   
    //Blink(10,1000,1000);
    while(1){}

}