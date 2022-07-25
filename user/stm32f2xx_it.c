/* Includes ------------------------------------------------------------------*/
#include "stm32f2xx_it.h"
#include "time_user.h"
#include "LED_user.h"
#include "stm32_ub_dac_dma.h"

extern uint32_t delay_decrement_1mcs;
uint32_t ms=0;
uint32_t m_x=1325,m_vector=0;

uint32_t counter=0;

void HardFault_Handler(void) {
    /* Go to infinite loop when Hard Fault exception occurs */
    while (1) {
    }
}

void SysTick_Handler(void) {
    /* Decrement the timeout value */
    TimingDelay_1ms_Decrement();
    //ms++;
}



void TIM2_IRQHandler(void) {
     ms++;
//    if(ms==1000){
//        ms=0;
//        LEDToggle();
//    }
     if(m_vector==0){
      m_x--;
      if(m_x==262)
          m_vector=1;
     }
     if(m_vector==1){
      m_x++;
      if(m_x==1325)
          m_vector=0;
     }
    UB_DAC_DMA_SetFrq1(1,m_x);//8749
    
    TIM_ClearFlag(TIM2, TIM_IT_Update);
    
}
void TIM5_IRQHandler(void) {
   TIM_ClearFlag(TIM5, TIM_IT_Update);  
  
     
}

void TIM6_DAC_IRQHandler() {
    if (TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM6, TIM_IT_Update);  
    }
}

void TIM7_IRQHandler(void) {
    if (TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET) {        
               
        TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
    }
}

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
