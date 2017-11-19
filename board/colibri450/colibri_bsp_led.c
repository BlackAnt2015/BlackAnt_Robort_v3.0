/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2017 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#include "gd32f4xx.h"
#include "colibri_bsp_led.h"

/*************************************************************************************************
 *  功能：初始化用户Led设备                                                                      *
 *  参数：无                                                                                     *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void EvbLedConfig(void)
{
    rcu_periph_clock_enable(RCU_GPIOB);
    gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_3);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_3);
    gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_4);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_4);

    rcu_periph_clock_enable(RCU_GPIOD);
    gpio_mode_set(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_7);
    gpio_output_options_set(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_7);
}



/*************************************************************************************************
 *  功能：控制Led的点亮和熄灭                                                                    *
 *  参数：(1) index Led灯编号                                                                    *
 *        (2) cmd   Led灯点亮或者熄灭的命令                                                      *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void EvbLedControl(int index, int cmd)
{
    switch (index)
    {
        case LED1:
        {
            if (cmd == LED_ON)
            {
                gpio_bit_set(GPIOB, GPIO_PIN_4); /*点亮Led1灯*/
            }
            else
            {
                gpio_bit_reset(GPIOB, GPIO_PIN_4); /*熄灭Led1灯*/
            }
            break;
        }
        case LED2:
        {
            if (cmd == LED_ON)
            {
                gpio_bit_set(GPIOB, GPIO_PIN_3); /*点亮Led2灯*/
            }
            else
            {
                gpio_bit_reset(GPIOB, GPIO_PIN_3); /*熄灭Led2灯*/
            }
            break;
        }
        case LED3:
        {
            if (cmd == LED_ON)
            {
                gpio_bit_set(GPIOD, GPIO_PIN_7); /*点亮Led3灯*/
            }
            else
            {
                gpio_bit_reset(GPIOD, GPIO_PIN_7); /*熄灭Led3灯*/
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

