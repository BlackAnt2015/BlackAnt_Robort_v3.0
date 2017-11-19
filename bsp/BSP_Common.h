#ifndef _BSP_COMMON_H
#define _BSP_COMMON_H
#include "stm32f10x.h"
#include "stm32f10x_conf.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_pwr.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_rtc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_dma.h"
#include "misc.h"

#include "../app/common.h"

int Crc16(unsigned char  *ptr, int count);
void _NOP(void);
#endif 
