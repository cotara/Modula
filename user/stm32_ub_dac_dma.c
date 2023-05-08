//--------------------------------------------------------------
// File     : stm32_ub_dac_dma.c
// Datum    : 25.03.2013
// Version  : 1.1
// Autor    : UB
// EMail    : mc-4u(@)t-online.de
// Web      : www.mikrocontroller-4u.de
// CPU      : STM32F4
// IDE      : CooCox CoIDE 1.7.0
// Module   : GPIO, DAC, TIM, DMA
// Funktion : DA-Wandler (DAC1 und DAC2) per DMA1
//
// Hinweis  : DAC1-Ausgang an PA4
//            DAC2-Ausgang an PA5
//--------------------------------------------------------------


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32_ub_dac_dma.h"



//--------------------------------------------------------------
// Глобальные переменные
//--------------------------------------------------------------
DAC_InitTypeDef  DAC_InitStructure;
DAC_DMA_MODE_t akt_dac_dma_mode;

uint32_t curBufOffset=0,curWavIdx=0;
extern uint8_t wavReadFlag,readBufIdx,stopFlag,curBufIdx;
extern uint32_t wavDataSize;
extern uint8_t wavBuf[2][512];
extern uint8_t wavBuf1[512],wavBuf2[512];
extern uint8_t wavBuf3[1024];
//extern uint16_t wavBuf16[1024];
extern uint16_t wavBuf16[51250];
//--------------------------------------------------------------
// Внутренняя функция
//--------------------------------------------------------------
void P_DAC_DMA_InitIO1(void);
void P_DAC_DMA_InitTIM1(void);
void P_DAC_DMA_InitDAC1(void);
void P_DAC_DMA_InitIO2(void);
void P_DAC_DMA_InitTIM2(void);
void P_DAC_DMA_InitDAC2(void);



//--------------------------------------------------------------
// Стандартный сигнал DAC=OFF
//--------------------------------------------------------------
const uint16_t WaveOff[] = {0}; // постоянный низкий уровень


//--------------------------------------------------------------
// Сигнал создается в виде массива
// Размер 12 бит (от 0 до 4095)
//--------------------------------------------------------------
const uint16_t Wave1[] = { // Синусоидальный
  2047, 2447, 2831, 3185, 3498, 3750, 3939, 4056,
  4095, 4056, 3939, 3750, 3495, 3185, 2831, 2447,
  2047, 1647, 1263,  909,  599,  344,  155,   38,
     0,   38,  155,  344,  599,  909, 1263, 1647
};




//--------------------------------------------------------------
// Сигнал создается в виде массива
// Размер 12 бит (от 0 до 4095)
//--------------------------------------------------------------
const uint16_t Wave2[] = { // Пилообразный
     0,  256,  512,  768, 1024, 1280, 1536, 1792,
  2048, 2304, 2560, 2816, 3072, 3328, 3584, 3840,
  4095
};


//--------------------------------------------------------------
// Сигнал создается в виде массива
// Размер 12 бит (от 0 до 4095)
//--------------------------------------------------------------
const uint16_t Wave3[] = { // Треугольный
     0,  256,  512,  768, 1024, 1280, 1536, 1792,
  2048, 2304, 2560, 2816, 3072, 3328, 3584, 3840,
  4095, 3840, 3584, 3328, 3072, 2816, 2560, 2304,
  2048, 1792, 1536, 1280, 1024,  768,  512,  256
};


//--------------------------------------------------------------
// Сигнал создается в виде массива
// Размер 12 бит (от 0 до 4095)
//--------------------------------------------------------------
const uint16_t Wave4[] = { // Прямоугольный
     0,  0,  4095,  4095
};


//--------------------------------------------------------------
// Определение всех сигналов
// Перечисляются в DAC_DMA_WAVE_NAME_t
//--------------------------------------------------------------
const DAC_WAVE_t DAC_WAVE[] = {
  // Name             , Kurve  , Lдnge
  {DAC_WAVE_OFF       , WaveOff, 1},
  {DAC_WAVE1_SINUS    , Wave1  , 32},
  {DAC_WAVE2_SAEGEZAHN, Wave2  , 17},
  {DAC_WAVE3_DREIECK  , Wave3  , 32},
  {DAC_WAVE4_RECHTECK , Wave4  , 4},
};




//--------------------------------------------------------------
// Инициализация ЦАП в режиме DMA
// режим: [SINGLE_DAC1_DMA, SINGLE_DAC2_DMA, DUAL_DAC_DMA]
//--------------------------------------------------------------
void UB_DAC_DMA_Init(DAC_DMA_MODE_t mode)
{
  // режим работы ЦАП
  akt_dac_dma_mode=mode;

  if((mode==SINGLE_DAC1_DMA) || (mode==DUAL_DAC_DMA)) {
    P_DAC_DMA_InitIO1();
    P_DAC_DMA_InitTIM1();
    P_DAC_DMA_InitDAC1();
    UB_DAC_DMA_SetWaveform1(DAC_WAVE_OFF);
  }
  if((mode==SINGLE_DAC2_DMA) || (mode==DUAL_DAC_DMA)) {
    P_DAC_DMA_InitIO2();
    P_DAC_DMA_InitTIM2();
    P_DAC_DMA_InitDAC2();
    UB_DAC_DMA_SetWaveform2(DAC_WAVE_OFF);
  }
}


//--------------------------------------------------------------
// Выбор сигнала для DAC1
//--------------------------------------------------------------
void UB_DAC_DMA_SetWaveform1(DAC_DMA_WAVE_NAME_t wave)
{
  DMA_InitTypeDef DMA_InitStructure;

  if(akt_dac_dma_mode==SINGLE_DAC2_DMA) return; //если не разрешено

  // включение тактирования (DMA)
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE); 

  // инициализация DMA (DMA1, Channel_7, Stream5)
  DMA_Cmd(DMA1_Stream5, DISABLE);
  DMA_DeInit(DMA1_Stream5);
  DMA_InitStructure.DMA_Channel = DMA_Channel_7;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)DAC1_DHR_12R1_ADDRESS;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)DAC_WAVE[wave].ptr;
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
  DMA_InitStructure.DMA_BufferSize = DAC_WAVE[wave].wave_len;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA1_Stream5, &DMA_InitStructure);

  // включение DMA
  DMA_Cmd(DMA1_Stream5, ENABLE);
  // включение DMA-DAC 
  DAC_DMACmd(DAC_Channel_1, ENABLE);
}


//--------------------------------------------------------------
// Выбор сигнала для DAC2
//--------------------------------------------------------------
void UB_DAC_DMA_SetWaveform2(DAC_DMA_WAVE_NAME_t wave)
{
  DMA_InitTypeDef DMA_InitStructure;

  if(akt_dac_dma_mode==SINGLE_DAC1_DMA) return; //если не разрешено

  // включение тактирования (DMA)
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);

  // инициализация DMA (DMA1, Channel7, Stream6)
  DMA_Cmd(DMA1_Stream6, DISABLE);
  DMA_DeInit(DMA1_Stream6);
  DMA_InitStructure.DMA_Channel = DMA_Channel_7;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)DAC2_DHR_12R2_ADDRESS;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)DAC_WAVE[wave].ptr;
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
  DMA_InitStructure.DMA_BufferSize = DAC_WAVE[wave].wave_len;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA1_Stream6, &DMA_InitStructure);

  // включение DMA 
  DMA_Cmd(DMA1_Stream6, ENABLE);
  // включение DMA-DAC 
  DAC_DMACmd(DAC_Channel_2, ENABLE);
}


//--------------------------------------------------------------
// Установка частоты DAC1
// Основная частота = 2*APB1 (APB1=42MHz) => TIM_CLK=84MHz
// делитель: 0 bis 0xFFFF
// период: 0 bis 0xFFFF
//
// выходная частота = TIM_CLK/делитель+1)/период+1)/длинна волны
//--------------------------------------------------------------
void UB_DAC_DMA_SetFrq1(uint16_t vorteiler, uint16_t periode)
{
  TIM_TimeBaseInitTypeDef    TIM_TimeBaseStructure;

  if(akt_dac_dma_mode==SINGLE_DAC2_DMA) return; //если не разрешено

  // Конигурация таймера
  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
  TIM_TimeBaseStructure.TIM_Period = periode;
  TIM_TimeBaseStructure.TIM_Prescaler = vorteiler;
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(DAC_TIM1_NAME, &TIM_TimeBaseStructure);
}


//--------------------------------------------------------------
// Установка частоты DAC2
// Основная частота = 2*APB1 (APB1=42MHz) => TIM_CLK=84MHz
// делитель: 0 bis 0xFFFF
// период: 0 bis 0xFFFF
//
// выходная частота = TIM_CLK/делитель+1)/период+1)/длинна волны
//--------------------------------------------------------------
void UB_DAC_DMA_SetFrq2(uint16_t vorteiler, uint16_t periode)
{
  TIM_TimeBaseInitTypeDef    TIM_TimeBaseStructure;
 
  if(akt_dac_dma_mode==SINGLE_DAC1_DMA) return; //если не разрешено

  // Конигурация таймера
  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
  TIM_TimeBaseStructure.TIM_Period = periode;
  TIM_TimeBaseStructure.TIM_Prescaler = vorteiler;
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(DAC_TIM2_NAME, &TIM_TimeBaseStructure);
}


//--------------------------------------------------------------
// внутренняя функция
// Инициализация IO-выводов как DAC1
//--------------------------------------------------------------
void P_DAC_DMA_InitIO1(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;

  // Включение тактирования (GPIO)
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

  // Конфигурирование выводов ЦАП как аналоговый выход
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
}


//--------------------------------------------------------------
// внутренняя функция
// Инициализация IO-выводов как DAC2
//--------------------------------------------------------------
void P_DAC_DMA_InitIO2(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;

  // Включение тактирования (GPIO)
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

  // Конфигурирование выводов ЦАП как аналоговый выход
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
}


//--------------------------------------------------------------
// внутренняя функция
// Инициализация таймера для DAC1
//--------------------------------------------------------------
void P_DAC_DMA_InitTIM1(void)
{
  TIM_TimeBaseInitTypeDef    TIM_TimeBaseStructure;

  // Включение тактирования (Timer)
  RCC_APB1PeriphClockCmd(DAC_TIM1_CLK, ENABLE);

  // Конигурация таймера
  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
  TIM_TimeBaseStructure.TIM_Period = DAC_DMA_DEF_PERIODE;
  TIM_TimeBaseStructure.TIM_Prescaler = DAC_DMA_DEF_PRESCALE;
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(DAC_TIM1_NAME, &TIM_TimeBaseStructure);

  // Триггер
  TIM_SelectOutputTrigger(DAC_TIM1_NAME, TIM_TRGOSource_Update);

  // Включение таймера
  TIM_Cmd(DAC_TIM1_NAME, ENABLE);
}


//--------------------------------------------------------------
// внутренняя функция
// Инициализация таймера для DAC2
//--------------------------------------------------------------
void P_DAC_DMA_InitTIM2(void)
{
  TIM_TimeBaseInitTypeDef    TIM_TimeBaseStructure;

  // Включение тактирования (Timer)
  RCC_APB1PeriphClockCmd(DAC_TIM2_CLK, ENABLE);

  // Конигурация таймера
  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
  TIM_TimeBaseStructure.TIM_Period = DAC_DMA_DEF_PERIODE;
  TIM_TimeBaseStructure.TIM_Prescaler = DAC_DMA_DEF_PRESCALE;
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(DAC_TIM2_NAME, &TIM_TimeBaseStructure);

  // Триггер
  TIM_SelectOutputTrigger(DAC_TIM2_NAME, TIM_TRGOSource_Update);

  // Включение таймера
  TIM_Cmd(DAC_TIM2_NAME, ENABLE);
}


//--------------------------------------------------------------
// внутренняя функция
// Инициализация DAC1
//--------------------------------------------------------------
void P_DAC_DMA_InitDAC1(void)
{
  // Включение тактирования (DAC)
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);

  // Инициализация DAC
  DAC_InitStructure.DAC_Trigger = DAC_TIM1_TRG;
  DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
  DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
  DAC_Init(DAC_Channel_1, &DAC_InitStructure);

  // Включение DAC
  DAC_Cmd(DAC_Channel_1, ENABLE);
}


//--------------------------------------------------------------
// внутренняя функция
// Инициализация DAC2
//--------------------------------------------------------------
void P_DAC_DMA_InitDAC2(void)
{
  // Включение тактирования (DAC)
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);

  // Инициализация DAC
  DAC_InitStructure.DAC_Trigger = DAC_TIM2_TRG;
  DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
  DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
  DAC_Init(DAC_Channel_2, &DAC_InitStructure);

  // Включение DAC
  DAC_Cmd(DAC_Channel_2, ENABLE);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void noDMAPlayinit(){
    
  GPIO_InitTypeDef  GPIO_InitStructure;
  TIM_TimeBaseInitTypeDef    TIM_TimeBaseStructure;
  DAC_InitTypeDef  DAC_InitStructure;
  
  // Включение тактирования (GPIO)
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  // Конфигурирование выводов ЦАП как аналоговый выход
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
 
  // Включение тактирования (Timer)
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
  // Конигурация таймера
  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
//  TIM_TimeBaseStructure.TIM_Period = 23-1;
//  TIM_TimeBaseStructure.TIM_Prescaler = 84-1;
  TIM_TimeBaseStructure.TIM_Period = 29-1;                                      //320 кГц
  TIM_TimeBaseStructure.TIM_Prescaler = 9-1;                                    //9.3333 Мгц
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);
  TIM_SelectOutputTrigger(TIM6, TIM_TRGOSource_Update);                         // Триггер
  TIM_Cmd(TIM6, ENABLE);                                                        // Включение таймера

  TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
  TIM_ClearFlag(TIM6,TIM_IT_Update);
  
  NVIC_SetPriority (TIM6_DAC_IRQn, 0);
  NVIC_EnableIRQ (TIM6_DAC_IRQn);
  

  
  // Включение тактирования (DAC)
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);
  // Инициализация DAC
  //DAC_InitStructure.DAC_Trigger = DAC_Trigger_T6_TRGO;
   DAC_InitStructure.DAC_Trigger = DAC_Trigger_None;
  
  DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
  DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Disable;
  DAC_Init(DAC_Channel_1, &DAC_InitStructure);
  DAC_Cmd(DAC_Channel_1, ENABLE);                                               // Включение DAC 
}

void noDMACall(){
    GPIOD->ODR ^= GPIO_Pin_0;   
//    uint16_t dacData = (((wavBuf[curBufIdx][curBufOffset + 1] << 8) | wavBuf[curBufIdx][curBufOffset]) + 32767);
//    dacData /= 16;
//    DAC_SetChannel1Data(DAC_Align_12b_R, dacData);
//    
//    curBufOffset += 2;
//    curWavIdx += 2;
  
    uint8_t dacData = wavBuf[curBufIdx][curBufOffset];
    DAC_SetChannel1Data(DAC_Align_8b_R, dacData);
    
    curBufOffset += 1;
    curWavIdx += 1;

    
    if (curWavIdx >= wavDataSize)
    {
        TIM_Cmd(TIM6, DISABLE);
        stopFlag = 1;
    }
    else
    {
        if (curBufOffset == WAV_BUF_SIZE)
        {
            curBufOffset = 0;
            
            if (curBufIdx == 0)
            {
                curBufIdx = 1;
            }
            else
            {
                curBufIdx = 0;
            }
        
            wavReadFlag = 1;
        }
    }

}

void DMAPlayinit(){
    
  GPIO_InitTypeDef  GPIO_InitStructure;
  TIM_TimeBaseInitTypeDef    TIM_TimeBaseStructure;
  DAC_InitTypeDef  DAC_InitStructure;
  DMA_InitTypeDef DMA_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  
  // Включение тактирования (GPIO)
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  // Конфигурирование выводов ЦАП как аналоговый выход
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
 
  // Включение тактирования (Timer)
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
  // Конигурация таймера
  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
  //TIM_TimeBaseStructure.TIM_Period = 23-1;                                      //44100 Гц
  //TIM_TimeBaseStructure.TIM_Prescaler = 84-1;                                   //1мкс
  TIM_TimeBaseStructure.TIM_Period = 29-1;                                      //320 кГц       40кГц несущая * 8 точек в периоде синуса
  TIM_TimeBaseStructure.TIM_Prescaler = 9-1;                                    //9.3333 Мгц 
  //TIM_TimeBaseStructure.TIM_Period = 2-1;                                       //2000 кГц       40кГц несущая * 8 точек в периоде синуса
  //TIM_TimeBaseStructure.TIM_Prescaler = 21-1;                                   //4 Мгц 
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);
  TIM_SelectOutputTrigger(TIM6, TIM_TRGOSource_Update);                         // Триггер
  TIM_Cmd(TIM6, ENABLE);                                                        // Включение таймера
  
  // Включение тактирования (DAC)
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);
  // Инициализация DAC
  DAC_InitStructure.DAC_Trigger = DAC_Trigger_T6_TRGO;
  DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
  //DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
  DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Disable;
  DAC_Init(DAC_Channel_1, &DAC_InitStructure);
  DAC_Cmd(DAC_Channel_1, ENABLE);                                               // Включение DAC
  
  
  // включение тактирования (DMA)
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE); 
  // инициализация DMA (DMA1, Channel_7, Stream5)
  DMA_Cmd(DMA1_Stream5, DISABLE);
  DMA_DeInit(DMA1_Stream5);
  DMA_InitStructure.DMA_Channel = DMA_Channel_7;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)DAC1_DHR_12L1_ADDRESS;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t) &wavBuf16;
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
  //DMA_InitStructure.DMA_BufferSize = 51250;//radio
  //DMA_InitStructure.DMA_BufferSize = 1024;//noModulation
  DMA_InitStructure.DMA_BufferSize = 8192;//modulatuin
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA1_Stream5, &DMA_InitStructure);

  DMA_ITConfig(DMA1_Stream5,DMA_IT_TC | DMA_IT_HT,ENABLE);
    
  /* Configure the NVIC Preemption Priority Bits */
  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream5_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure); 
  
  // включение DMA
  DMA_Cmd(DMA1_Stream5, ENABLE);
  // включение DMA-DAC 
  DAC_DMACmd(DAC_Channel_1, ENABLE);
}