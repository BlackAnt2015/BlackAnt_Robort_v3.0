#ifndef _TCL_COLIBRI_H
#define _TCL_COLIBRI_H

#include "tcl.gd32.f450ve.h"
#include "colibri_bsp_led.h"
#include "colibri_bsp_key.h"
#include "colibri_bsp_uart.h"


#define KEY0_IRQ_ID    GD32F450VE_EXTI0_IRQn
#define KEY1_IRQ_ID    GD32F450VE_EXTI1_IRQn
#define TIM_IRQ_ID     GD32F450VE_TIMER2_IRQn
#define UART_IRQ_ID    GD32F450VE_USART2_IRQn

extern void EvbSetupEntry(void);
extern void EvbTraceEntry(const char* str);
#define EVB_PRINTF     EvbTraceEntry
#endif /* _TCL_COLIBRI_H */
