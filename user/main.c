#include "LED_user.h"
#include "time_user.h"
#include "ff.h"
#include "sdio_sd.h"
#include "stm32f4xx_sdio.h"
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_dac.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_dma.h"
#include "stm32_ub_dac_dma.h"

/////////////////////////////FATFS//////////////////////////////////////////////
FATFS filesystem; /* volume lable */
FRESULT res; /* Result code */
FIL file; /* File object */
UINT bw, br;
SD_CardStatus card_stat;
SD_CardInfo card_info;
uint8_t detection;
uint8_t wavBuf1[512];
uint8_t wavBuf2[512];
uint8_t wavBuf3[1024];
//uint16_t wavBuf16[51250];
uint16_t wavBuf16[8192];
//uint16_t wavBuf16[1024];
uint8_t wavBuf[2][512];

uint32_t wavDataSize=0,fullwavDataSize=0;

unsigned int readBytes;
uint32_t dataOffset = 0;
////////////////////////////////////////////////////////////////////////////////

uint8_t wavReadFlag=0,readBufIdx=0,stopFlag=0,curBufIdx=0;

int main (){
        
    
    GPIO_InitTypeDef  GPIO_InitStructure;
  
    /* Enable the GPIO_LED Clock */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

    /* Configure the GPIO_LED pin */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
  
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

    /* Configure the GPIO_LED pin */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
  
    SysTick_Config(SystemCoreClock/1000);
  
    //timers_init();
    //LEDInit();
   
    res = f_mount(0, &filesystem);
    res = f_open(&file, "Fionnghuala8.wav", FA_OPEN_ALWAYS | FA_READ);   

    //Читаем первые 512 байт файла, ищем заголовок WAV, чтобы понять, откуда начинаются файлы
    res = f_read(&file,wavBuf[readBufIdx], WAV_BUF_SIZE, &readBytes);
    for (uint16_t i = 0; i < (WAV_BUF_SIZE - 3); i++)
    {
        if ((wavBuf[0][i] == 'd') && (wavBuf[0][i + 1] == 'a') &&
            (wavBuf[0][i + 2] == 't') && (wavBuf[0][i + 3] == 'a'))
        {
            dataOffset = i + 8;
            break;
        }
    }
    //Смещаем указатель чтения на начало файлов
    res = f_lseek(&file, dataOffset);
    fullwavDataSize = f_size(&file) - dataOffset;
    wavDataSize=fullwavDataSize;
    
    //NO DMA PLAYER
//   res = f_read(&file, wavBuf[0], WAV_BUF_SIZE, &readBytes);
//   res = f_read(&file, wavBuf[1], WAV_BUF_SIZE, &readBytes);
//  
//             
// 
//    noDMAPlayinit();
//    while(1){
//    if (wavReadFlag == 1)
//    {
//      uint8_t readBufIdx = 0;
//      
//      if (curBufIdx == 0)
//      {
//          readBufIdx = 1;
//      }
//      res = f_read(&file, wavBuf[readBufIdx], WAV_BUF_SIZE, &readBytes);
//      wavReadFlag = 0;
//      
//    }
//    if (stopFlag == 1)
//    {
//      res = f_close(&file);
//      stopFlag = 0;
//    }
//   }
    
    //DMA PLAYER
    res = f_read(&file, wavBuf3, WAV_BUF_SIZE*2, &readBytes);
    DMAPlayinit();
    
    while(1){
        
            
    }
    
    
}
