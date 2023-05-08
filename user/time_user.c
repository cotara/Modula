#include "time_user.h"
#include "stm32f4xx_pwr.h"
volatile unsigned int Ticks,TicksUIP=0 ;
volatile uint32_t TimingDelay_1mcs,TimingDelay_1ms;


void timers_init(void)
{

TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
RCC_ClocksTypeDef RCC_ClocksStatus;

Ticks = 0;
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 |  RCC_APB1Periph_TIM5,ENABLE);
//  /***********************************************************************
//                                TIMER2              
//  ***********************************************************************/
   RCC_GetClocksFreq(&RCC_ClocksStatus);
  TIM_InternalClockConfig(TIM2);
  TIM_TimeBaseStructure.TIM_Prescaler = 168-1;                                  //Частота шины 84МГц, но 84000 поставить нельзя потому, что там 2 байта
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseStructure.TIM_Period = 3000;                                      //1мс
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
  TIM_TimeBaseInit(TIM2,&TIM_TimeBaseStructure);
   
  TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
  TIM_ClearFlag(TIM2,TIM_IT_Update);
  
  NVIC_SetPriority (TIM2_IRQn, 1);
  NVIC_EnableIRQ (TIM2_IRQn);

  //TIM_Cmd(TIM2,ENABLE);
  
 /***********************************************************************
                                TIMER6              
  ***********************************************************************/
 
  RCC_GetClocksFreq(&RCC_ClocksStatus);
  TIM_InternalClockConfig(TIM5);
  
  TIM_TimeBaseStructure.TIM_Prescaler = 168-1;                                   //Смотреть осцилом
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseStructure.TIM_Period = 22;                                         //1 мкс
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
  TIM_TimeBaseInit(TIM5,&TIM_TimeBaseStructure);
  
 
  TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);
  TIM_ClearFlag(TIM5,TIM_IT_Update);
   
  NVIC_SetPriority (TIM5_IRQn, 0);
  NVIC_EnableIRQ (TIM5_IRQn);
  
  //TIM_Cmd(TIM5,ENABLE);

  // TIM 6
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);

  TIM_TimeBaseInitTypeDef base_timer;
  TIM_TimeBaseStructInit(&base_timer);
  base_timer.TIM_Prescaler = 8400 - 1;                                          //2мс
  base_timer.TIM_Period = 1000-1;                                               //2 секунда!
  TIM_TimeBaseInit(TIM6, &base_timer);

  TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
  
  NVIC_SetPriority (TIM6_DAC_IRQn, 2);
  NVIC_EnableIRQ(TIM6_DAC_IRQn);
  TIM_Cmd(TIM6,ENABLE);
/*
   // TIM 7
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);

  TIM_TimeBaseStructInit(&base_timer);
  base_timer.TIM_Prescaler = 24000 - 1;
  base_timer.TIM_Period = 1000;
  TIM_TimeBaseInit(TIM7, &base_timer);

  TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);
  TIM_Cmd(TIM7, ENABLE);

  NVIC_EnableIRQ(TIM7_IRQn);
*/
}


void update_Time(RTC_TimeTypeDef *time, unsigned char* RTC_time)
{ 
  unsigned char h1,h2,m1,m2,s1,s2;
  
      if( SysTick->CTRL & (1<<16))
      {
        RTC_GetTime(RTC_Format_BIN,time);
        h1=48+time->RTC_Hours/10;
        h2=48+time->RTC_Hours%10;
        m1=48+time->RTC_Minutes/10;
        m2=48+time->RTC_Minutes%10;
        s1=48+time->RTC_Seconds/10;
        s2=48+time->RTC_Seconds%10;
        
        memcpy(RTC_time,&h1,1);
        memcpy(RTC_time+1,&h2,1);
        memcpy(RTC_time+2,":",1);
        memcpy(RTC_time+3,&m1,1);
        memcpy(RTC_time+4,&m2,1);
        memcpy(RTC_time+5,":",1);
        memcpy(RTC_time+6,&s1,1);
        memcpy(RTC_time+7,&s2,1);
              
      }
}

/*Задержка в nTime 1 мс*/
void delay_1_ms(volatile uint32_t nTime){
  TimingDelay_1ms = nTime;
  while(TimingDelay_1ms != 0);
}
void TimingDelay_1ms_Decrement(void){
  if (TimingDelay_1ms != 0x00)
    TimingDelay_1ms--;
}


/*Задержка в nTime 1 мкс*/
void delay_1_mcs(volatile uint32_t nTime){
  TimingDelay_1mcs = nTime;
  while(TimingDelay_1mcs != 0);
}
void TimingDelay_1mcs_Decrement(void){
  if (TimingDelay_1mcs != 0x00)
    TimingDelay_1mcs--;
}