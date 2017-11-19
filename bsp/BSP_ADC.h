#ifndef _BSP_ADC_H
#define _BSP_ADC_H

#include "stm32f10x.h"
#include "BSP_Common.h"
#include <stdio.h>

#define ADC_Point 5
#define ADC_Ch_Num 8

typedef enum
{
    ADC_CHANNEL1 = 0,
    ADC_CHANNEL2, 
    ADC_CHANNEL3, 
    ADC_CHANNEL4, 
    ADC_CHANNEL5, 
    ADC_CHANNEL6,
    ADC_CHANNEL7, 
    ADC_CHANNEL8,
    ADC_CHANNEL9, 
    ADC_CHANNEL10,
    ADC_CHANNEL11, 
    ADC_CHANNEL12,
    ADC_CHANNEL13,
    ADC_CHANNEL14,
}ADC_CHANNEL_TAG;



 void  ADC1_Init(void);
 void Task_ADC_Handle(void);
 void ADC_Vaule_Update( void *dat);
 u16 STM32_ADC_Read( u8 ch);

#endif




















/*********************************************END OF FILE**********************/
