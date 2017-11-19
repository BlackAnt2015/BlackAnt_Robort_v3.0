/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2017 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.cpu.h"
#include "tcl.kernel.h"

/* 重写库函数 */
void SysTick_Handler(void)
{
    OsKernelEnterIntrState();
    OsKernelTickISR();
    OsKernelLeaveIntrState();
}

/* 重写库函数 */
void EXTI4_15_IRQHandler(void)
{
#if (TCLC_IRQ_ENABLE)
    OsKernelEnterIntrState();
    OsIrqEnterISR(EXTI4_15_IRQID);
    OsKernelLeaveIntrState();
#else
    return;
#endif
}


/* 重写库函数 */
void TIM2_IRQHandler(void)
{
#if (TCLC_IRQ_ENABLE)
    OsKernelEnterIntrState();
    OsIrqEnterISR(TIMER2_IRQID);
    OsKernelLeaveIntrState();
#else
    return;
#endif
}

/* 重写库函数 */
void USART2_IRQHandler(void)
{
#if (TCLC_IRQ_ENABLE)
    OsKernelEnterIntrState();
    OsIrqEnterISR(USART2_IRQID);
    OsKernelLeaveIntrState();
#else
    return;
#endif

}
