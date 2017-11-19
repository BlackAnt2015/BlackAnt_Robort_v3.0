#ifndef _BSP_TIMER_H
#define _BSP_TIMER_H

#include "BSP_Common.h"
//**************************************BSP_Timer 公共函数******************************************
void TIMER_Init(void);
extern u8 STM32_TIMER3_GetCounterFlag(void);

//**************************************BSP_Timer 公共变量区******************************************
extern unsigned char Task_20ms,Task_50ms,Task_100ms,Task_500ms,Task_1s;



#endif

















/*********************************************END OF FILE**********************/
