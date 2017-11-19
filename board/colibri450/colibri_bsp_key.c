/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2017 LIUXUMING                                  *
 *                                        www.trochili.com                                       *
 *************************************************************************************************/
#include "gd32f4xx.h"
#include "colibri_bsp_key.h"

void EvbKeyConfig(void)
{
    rcu_periph_clock_enable(RCU_GPIOE);
	  rcu_periph_clock_enable(RCU_SYSCFG);
    gpio_mode_set(GPIOE, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_PIN_0 | GPIO_PIN_1);

    /* enable and set key EXTI interrupt to the lowest priority */
    nvic_irq_enable(EXTI0_IRQn, 2, 0);
    nvic_irq_enable(EXTI1_IRQn, 2, 1);
	
    /* connect key EXTI line to key GPIO pin */
    syscfg_exti_line_config(EXTI_SOURCE_GPIOE, EXTI_SOURCE_PIN0);
    syscfg_exti_line_config(EXTI_SOURCE_GPIOE, EXTI_SOURCE_PIN1);

    /* configure key EXTI line */
    exti_init(EXTI_0, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
    exti_init(EXTI_1, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
    exti_interrupt_flag_clear(EXTI_0);
    exti_interrupt_flag_clear(EXTI_1);
}

int EvbKeyScan(void)
{
    int count = 0xffff;

    if (RESET != exti_interrupt_flag_get(EXTI_0))
    {
        while(count--);
        if (RESET != exti_interrupt_flag_get(EXTI_0))
        {
            exti_interrupt_flag_clear(EXTI_0);
            return 1;
        }
    }

    if (RESET != exti_interrupt_flag_get(EXTI_1))
    {
        while(count--);
        if (RESET != exti_interrupt_flag_get(EXTI_1))
        {
            exti_interrupt_flag_clear(EXTI_1);
            return 2;
        }
    }

    return 0;
}
