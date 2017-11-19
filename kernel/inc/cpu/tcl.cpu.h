/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2017 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCLC_CPU_H
#define _TCLC_CPU_H

#include "tcl.config.h"

#if (TCLC_CPU_TYPE == TCLMCU_GD32F150R8)
#include "tcl.arm.cm3.h"
#include "tcl.gd32.f150.h"
#define TCLC_CPU_MINIMAL_STACK          (256U)
#define TCLC_CPU_STACK_ALIGNED          (4U)
#define TCLC_CPU_IRQ_NUM                (CAN2_SCE_IRQID)
#define TCLC_CPU_CLOCK_FREQ             (72U*1024U*1024U)

#elif (TCLC_CPU_TYPE == TCLMCU_GD32F190R8)
#include "tcl.arm.cm3.h"
#include "tcl.gd32.f190r8.h"
#define TCLC_CPU_MINIMAL_STACK          (256U)
#define TCLC_CPU_STACK_ALIGNED          (4U)
#define TCLC_CPU_IRQ_NUM                (CAN2_SCE_IRQID)
#define TCLC_CPU_CLOCK_FREQ             (72U*1024U*1024U)

#elif (TCLC_CPU_TYPE == TCLMCU_GD32F450VE)
#include "tcl.arm.cm4.h"
#include "tcl.gd32.f450ve.h"
#define TCLC_CPU_MINIMAL_STACK          (256U)
#define TCLC_CPU_STACK_ALIGNED          (4U)
#define TCLC_CPU_IRQ_NUM                (GD32F450VE_IPA_IRQn)
#define TCLC_CPU_CLOCK_FREQ             (120U*1024U*1024U)

#elif (TCLC_CPU_TYPE == TCLMCU_STM32F103RB)
#include "tcl.arm.cm3.h"
#include "tcl.stm32.f103rb.h"
#define TCLC_CPU_MINIMAL_STACK          (256U)
#define TCLC_CPU_STACK_ALIGNED          (4U)
#define TCLC_CPU_IRQ_NUM                (USBWakeUp_IRQ_ID)
#define TCLC_CPU_CLOCK_FREQ             (72U*1024U*1024U)

#else
#error "cpu type error"
#endif


#endif /* _TCLC_CPU_H */

