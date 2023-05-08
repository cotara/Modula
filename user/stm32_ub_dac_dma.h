//--------------------------------------------------------------
// File     : stm32_ub_dac_dma.h
//--------------------------------------------------------------

//--------------------------------------------------------------
#ifndef __STM32F4_UB_DAC_DMA_H
#define __STM32F4_UB_DAC_DMA_H


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_dac.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_dma.h"



typedef enum {
  SINGLE_DAC1_DMA = 0, // ������������ ������ DAC1 �� PA4
  SINGLE_DAC2_DMA,     // ������������ ������ DAC2 �� PA5
  DUAL_DAC_DMA         // ������������ DAC1 (PA4) � DAC2 (PA5)
}DAC_DMA_MODE_t;



//--------------------------------------------------------------
// ������ ���� ���-��������
// (�� �������������, ������� � 0)
//--------------------------------------------------------------
typedef enum {
  DAC_WAVE_OFF          = 0,    // ���������� ������ �������
  DAC_WAVE1_SINUS       = 1,    // ����� ������� 1 (�����)
  DAC_WAVE2_SAEGEZAHN   = 2,    // ����� ������� 2 (������������)
  DAC_WAVE3_DREIECK     = 3,    // ����� ������� 3 (�����������)
  DAC_WAVE4_RECHTECK    = 4     // ����� ������� 4 (�������������)
}DAC_DMA_WAVE_NAME_t;


//--------------------------------------------------------------
// �������� ��������� �������
//--------------------------------------------------------------
#define  DAC_DMA_DEF_PERIODE   255  // ������
#define  DAC_DMA_DEF_PRESCALE   10  // ��������


//--------------------------------------------------------------
// ��������� ����� ������ ���
//--------------------------------------------------------------
typedef struct {
  DAC_DMA_WAVE_NAME_t name;
  const uint16_t *ptr;
  uint16_t wave_len;
}DAC_WAVE_t;


//--------------------------------------------------------------
// ��������� �������
// ��������� ������� ��� �������� DAC1:
// Timer2, Timer4, Timer5, Timer6, Timer7
// (Timer8 ������� �� APB2)
//--------------------------------------------------------------

// ����� �������
//#define DAC_DMA_T1_TIM2            2
//#define DAC_DMA_T1_TIM4            4
//#define DAC_DMA_T1_TIM5            5
#define DAC_DMA_T1_TIM6            6
//#define DAC_DMA_T1_TIM7            7


#ifdef DAC_DMA_T1_TIM2
  #define DAC_TIM1_TRG            DAC_Trigger_T2_TRGO
  #define DAC_TIM1_CLK            RCC_APB1Periph_TIM2
  #define DAC_TIM1_NAME           TIM2
#elif defined DAC_DMA_T1_TIM4
  #define DAC_TIM1_TRG            DAC_Trigger_T4_TRGO
  #define DAC_TIM1_CLK            RCC_APB1Periph_TIM4
  #define DAC_TIM1_NAME           TIM4
#elif defined DAC_DMA_T1_TIM5
  #define DAC_TIM1_TRG            DAC_Trigger_T5_TRGO
  #define DAC_TIM1_CLK            RCC_APB1Periph_TIM5
  #define DAC_TIM1_NAME           TIM5
#elif defined DAC_DMA_T1_TIM6
  #define DAC_TIM1_TRG            DAC_Trigger_T6_TRGO
  #define DAC_TIM1_CLK            RCC_APB1Periph_TIM6
  #define DAC_TIM1_NAME           TIM6
#elif defined DAC_DMA_T1_TIM7
  #define DAC_TIM1_TRG            DAC_Trigger_T7_TRGO
  #define DAC_TIM1_CLK            RCC_APB1Periph_TIM7
  #define DAC_TIM1_NAME           TIM7
#endif


//--------------------------------------------------------------
// ��������� �������
// ��������� ������� ��� �������� DAC2:
// Timer2, Timer4, Timer5, Timer6, Timer7
// (Timer8 ������� �� APB2)
//--------------------------------------------------------------

// ����� �������
//#define DAC_DMA_T2_TIM2            2
//#define DAC_DMA_T2_TIM4            4
//#define DAC_DMA_T2_TIM5            5
//#define DAC_DMA_T2_TIM6            6
#define DAC_DMA_T2_TIM7            7


#ifdef DAC_DMA_T2_TIM2
  #define DAC_TIM2_TRG            DAC_Trigger_T2_TRGO
  #define DAC_TIM2_CLK            RCC_APB1Periph_TIM2
  #define DAC_TIM2_NAME           TIM2
#elif defined DAC_DMA_T2_TIM4
  #define DAC_TIM2_TRG            DAC_Trigger_T4_TRGO
  #define DAC_TIM2_CLK            RCC_APB1Periph_TIM4
  #define DAC_TIM2_NAME           TIM4
#elif defined DAC_DMA_T2_TIM5
  #define DAC_TIM2_TRG            DAC_Trigger_T5_TRGO
  #define DAC_TIM2_CLK            RCC_APB1Periph_TIM5
  #define DAC_TIM2_NAME           TIM5
#elif defined DAC_DMA_T2_TIM6
  #define DAC_TIM2_TRG            DAC_Trigger_T6_TRGO
  #define DAC_TIM2_CLK            RCC_APB1Periph_TIM6
  #define DAC_TIM2_NAME           TIM6
#elif defined DAC_DMA_T2_TIM7
  #define DAC_TIM2_TRG            DAC_Trigger_T7_TRGO
  #define DAC_TIM2_CLK            RCC_APB1Periph_TIM7
  #define DAC_TIM2_NAME           TIM7
#endif


//--------------------------------------------------------------
// ����� ���
// (��� 55 ����������� �����������)
//--------------------------------------------------------------
#define DAC_BASE_ADR        ((uint32_t)0x40007400)


//--------------------------------------------------------------
// ������ ���������
// (��� 325 ����������� �����������)
//--------------------------------------------------------------
#define DAC1_REG_DHR_12R1_OFFSET   0x08  // 12bit, ������� ������
#define DAC1_REG_DHR_12L1_OFFSET 0x0C  // 12bit, ������� ����� (� ������� �������� ������ 12 ��� �� ������ ����������)
#define DAC1_REG_DHR_8R_OFFSET   0x10  // 8bit, ������� �����
#define DAC2_REG_DHR_12R2_OFFSET   0x14  // 12bit, ������� ������

#define DAC1_DHR_8R1_ADDRESS    (DAC_BASE_ADR | DAC1_REG_DHR_8R_OFFSET)
#define DAC1_DHR_12L1_ADDRESS    (DAC_BASE_ADR | DAC1_REG_DHR_12L1_OFFSET)
#define DAC1_DHR_12R1_ADDRESS    (DAC_BASE_ADR | DAC1_REG_DHR_12R1_OFFSET)
#define DAC2_DHR_12R2_ADDRESS    (DAC_BASE_ADR | DAC2_REG_DHR_12R2_OFFSET)




//--------------------------------------------------------------
// ���������� �������
//--------------------------------------------------------------
void UB_DAC_DMA_Init(DAC_DMA_MODE_t mode);
void UB_DAC_DMA_SetWaveform1(DAC_DMA_WAVE_NAME_t wave);
void UB_DAC_DMA_SetWaveform2(DAC_DMA_WAVE_NAME_t wave);
void UB_DAC_DMA_SetFrq1(uint16_t vorteiler, uint16_t periode);
void UB_DAC_DMA_SetFrq2(uint16_t vorteiler, uint16_t periode);

#define WAV_BUF_SIZE 512
void noDMACall();
void noDMAPlayinit();

void DMAPlayinit();

//--------------------------------------------------------------
#endif // __STM32F4_UB_DAC_DMA_H
