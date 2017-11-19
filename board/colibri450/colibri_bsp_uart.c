/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2017 LIUXUMING                                  *
 *                                        www.trochili.com                                       *
 *************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "gd32f4xx.h"
#include "colibri_bsp_uart.h"

void EvbUart1Config(void)
{
    rcu_periph_clock_enable(RCU_GPIOD);
    rcu_periph_clock_enable(RCU_USART2);

    /* connect port to USART2_Tx */
    gpio_af_set(GPIOD, GPIO_AF_7, GPIO_PIN_8);

    /* connect port to USARTx_R1 */
    gpio_af_set(GPIOD, GPIO_AF_7, GPIO_PIN_9);

    /* configure USART2 Tx as alternate function push-pull */
    gpio_mode_set(GPIOD, GPIO_MODE_AF, GPIO_PUPD_PULLUP,GPIO_PIN_8);
    gpio_output_options_set(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ,GPIO_PIN_8);

    /* configure USART2 Rx as alternate function push-pull */
    gpio_mode_set(GPIOD, GPIO_MODE_AF, GPIO_PUPD_PULLUP,GPIO_PIN_9);
    gpio_output_options_set(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ,GPIO_PIN_9);

    /* USART2 configure */
    usart_deinit(USART2);
    usart_baudrate_set(USART2,115200);
    usart_receive_config(USART2, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART2, USART_TRANSMIT_ENABLE);
    usart_enable(USART2);
}


/*************************************************************************************************
 *  功能：向串口1发送一个字符                                                                    *
 *  参数：(1) 需要被发送的字符                                                                   *
 *  返回：                                                                                       *
 *  说明：                                                                                       *
 *************************************************************************************************/
void EvbUart1WriteByte(char c)
{
    usart_data_transmit(USART2, c);
    while (RESET == usart_flag_get(USART2,USART_FLAG_TC));
}

/*************************************************************************************************
 *  功能：向串口1发送一个字符串                                                                  *
 *  参数：(1) 需要被发送的字符串                                                                 *
 *  返回：                                                                                       *
 *  说明：                                                                                       *
 *************************************************************************************************/
void EvbUart1WriteStr(const char* str)
{

    while (*str)
    {
        usart_data_transmit(USART2, * str++);
        while (RESET == usart_flag_get(USART2,USART_FLAG_TC));
    }
}


/*************************************************************************************************
 *  功能：从串口1接收一个字符                                                                    *
 *  参数：(1) 存储接收到的字符                                                                   *
 *  返回：                                                                                       *
 *  说明：                                                                                       *
 *************************************************************************************************/
void EvbUart1ReadByte(char* c)
{
    while (RESET == usart_flag_get(USART2,USART_FLAG_RBNE));
    *c = (usart_data_receive(USART2));
}


static char _buffer[256];
void EvbUart1Printf(char* fmt, ...)
{
    int i;
    va_list ap;
    va_start(ap, fmt);
    vsprintf(_buffer, fmt, ap);
    va_end(ap);

    for (i = 0; _buffer[i] != '\0'; i++)
    {
        EvbUart1WriteByte(_buffer[i]);
    }
}

