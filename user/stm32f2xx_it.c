/* Includes ------------------------------------------------------------------*/
#include "stm32f2xx_it.h"
#include "time_user.h"
#include "LED_user.h"
#include "stm32_ub_dac_dma.h"
#include "sdio_sd.h" 
#include "stm32f4xx_dma.h"
#include "ff.h"

extern uint32_t delay_decrement_1mcs;
uint32_t ms=0;
uint32_t m_x=1325,m_vector=0;

uint32_t counter=0;
extern uint8_t wavBuf3[1024];
//extern uint16_t wavBuf16[51250];
extern uint16_t wavBuf16[8192];
//extern uint16_t wavBuf16[1024];
extern FIL file;
extern unsigned int readBytes;
extern uint32_t wavDataSize;
extern FRESULT res;
uint8_t firstSide=0;
uint16_t minA=65535,maxA=0;
float m_mod=1;
uint8_t st=1;
extern uint32_t fullwavDataSize;
const uint8_t WaveN[] = { // Синусоидальный
  128, 153, 177, 199, 219, 234, 246, 254,
  255, 254, 246, 234, 219, 199, 177, 153,
  128, 103, 79,  57,  37,  22,  10,   2,
     0,   2,  10,  22,  37,  57, 79, 103
};

const uint8_t Wave8[] = { // Синусоидальный
  128,219,255,219,128,37,0,37
};
const int8_t Wave8_[] = { // Синусоидальный
  0,91,127,91,0,-91,0,-91
};
const uint8_t Wave1000kHz[] = { // Синусоидальный
  0,255,0,255,0,255,0,255,0,255,0,255,0,255,0,255,0,255,0,255,
  0,255,0,255,0,255,0,255,0,255,0,255,0,255,0,255,0,255,0,255,
  0,255,0,255,0,255,0,255,0,255,
};
void HardFault_Handler(void) {
    /* Go to infinite loop when Hard Fault exception occurs */
    while (1) {
    }
}

void SysTick_Handler(void) {
    /* Decrement the timeout value */
    TimingDelay_1ms_Decrement();
    
    ms++;
    if(ms==1000){
        if(GPIO_ReadInputDataBit (GPIOD, GPIO_Pin_5)==1 && st==0){
            st=1;
            // включение DMA
            DMA_Cmd(DMA1_Stream5, ENABLE);
            // включение DMA-DAC 
            DAC_DMACmd(DAC_Channel_1, ENABLE);
        }
        else if(GPIO_ReadInputDataBit (GPIOD, GPIO_Pin_5)==0 && st==1){
            st=0;
            // включение DMA
            DMA_Cmd(DMA1_Stream5, DISABLE);
            // включение DMA-DAC 
            DAC_DMACmd(DAC_Channel_1, DISABLE);
        }
    
        ms=0;
    }

}



void TIM2_IRQHandler(void) {
     ms++;

    TIM_ClearFlag(TIM2, TIM_IT_Update);
    
}
void TIM5_IRQHandler(void) {
   TIM_ClearFlag(TIM5, TIM_IT_Update);       
}

void TIM6_DAC_IRQHandler() {
    if (TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET) {
      GPIOE->ODR ^= GPIO_Pin_0;
      //noDMACall();
      TIM_ClearITPendingBit(TIM6, TIM_IT_Update);  
    }
}



void DMA1_Stream5_IRQHandler(void)
{
  if (DMA_GetITStatus(DMA1_Stream5,DMA_IT_HTIF5)){//half of buf transmited to DMA
    DMA_ClearITPendingBit(DMA1_Stream5,DMA_IT_HTIF5);
    firstSide=1;
     res= f_read(&file,&wavBuf3[0], WAV_BUF_SIZE, &readBytes);
     for(int i=0;i<512;i++){
         for(int k=0;k<8;k++)
            wavBuf16[i*8+k] = (wavBuf3[i]*m_mod+255*(1-m_mod))*Wave8[k];
            //wavBuf16[i*8+k] = (wavBuf3[i]*m_mod)*Wave8[k];
            //wavBuf16[i*8+k] = wavBuf3[i]*(Wave8_[k])+32640;
             //wavBuf16[i*50+k] = (wavBuf3[i]*m_mod+128*(1-m_mod))*Wave1000kHz[k];//радио

     }
//     for(int i=0;i<512;i++){
//         wavBuf16[i] = wavBuf3[i]*0xFF;
//     }
//              if(wavBuf3[i]<minA) minA=wavBuf3[i];
//         if(wavBuf3[i]>maxA) maxA=wavBuf3[i];
     
  }
    
  if (DMA_GetITStatus(DMA1_Stream5,DMA_IT_TCIF5))
  {
      DMA_ClearITPendingBit(DMA1_Stream5,DMA_IT_TCIF5);
      firstSide=0;
      res = f_read(&file,&wavBuf3[512], WAV_BUF_SIZE, &readBytes); 
      for(int i=512;i<1024;i++){
          for(int k=0;k<8;k++)
            wavBuf16[i*8+k] = (wavBuf3[i]*m_mod+255*(1-m_mod))*Wave8[k];
            //wavBuf16[i*8+k] = (wavBuf3[i]*m_mod)*Wave8[k];
           //wavBuf16[i*8+k] = wavBuf3[i]*(Wave8_[k])+32640;
              //wavBuf16[i*50+k] = (wavBuf3[i]*m_mod+128*(1-m_mod))*Wave1000kHz[k];//радио

     }
//     for(int i=512;i<1024;i++){
//         wavBuf16[i] = wavBuf3[i]*0xFF;
//     }
      //         if(wavBuf3[i]<minA) minA=wavBuf3[i];
//         if(wavBuf3[i]>maxA) maxA=wavBuf3[i];
  }
  
  wavDataSize -= readBytes;
  if(wavDataSize<15000){
      wavDataSize=fullwavDataSize;
      f_lseek(&file, 50);                                         //Зацикливаем
      //TIM_Cmd(TIM6, DISABLE);
  }
  
}


void SDIO_IRQHandler(void) {

    
    /* Process All SDIO Interrupt Sources */
    SD_ProcessIRQSrc();
     GPIOD->ODR ^= GPIO_Pin_0;
//     if(firstSide){
//         for(int i=0;i<512;i+=2){
//             uint16_t dacData = (((wavBuf3[i + 1] << 8) | wavBuf3[i]) + 32767);
//              dacData /= 16;
//              wavBuf3[i + 1] = dacData>>8;
//              wavBuf3[i] = dacData&0xFF;
//              //wavBuf3[i]=wavBuf3[i]>>4;
////             if(wavBuf3[i]<128)
////                wavBuf3[i+1]=0;
//         }
//     }
//     else{
//         for(int i=512;i<1024;i+=2){
//              uint16_t dacData = (((wavBuf3[i + 1] << 8) | wavBuf3[i]) + 32767);
//              dacData /= 16;
//              wavBuf3[i + 1] = dacData>>8;
//              wavBuf3[i] = dacData&0xFF;
//             //wavBuf3[i]=wavBuf3[i]>>4;
////             if(wavBuf3[i]<128)
////                wavBuf3[i+1]=0;
//         }
//     }
}


/**
* @brief  This function handles DMA2 Stream3 or DMA2 Stream6 global interrupts
*         requests.
* @param  None
* @retval None
*/
void DMA2_Stream3_IRQHandler(void) {
    //GPIO_SetBits(GPIOE, GPIO_Pin_4);
    /* Process DMA2 Stream3 or DMA2 Stream6 Interrupt Sources */
    SD_ProcessDMAIRQ();
    //GPIO_ResetBits(GPIOE, GPIO_Pin_4);
}
/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
