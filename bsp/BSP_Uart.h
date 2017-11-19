#ifndef _BSP_UART_H
#define  _BSP_UART_H

#include "../app/common.h"
#include "BSP_Uart.h"

#define Tx_Array_Size 			 	 50	                //发送缓存区大小
#define Rx_Array_Size 			 	 50		              //接受缓存区大小


#define uint8 	unsigned char
#define uint16 	unsigned short
#define uint32 	unsigned int
	
typedef enum
{
    STM32_UART1  = 0,
    STM32_UART2,
    STM32_UART3,
    STM32_UART4
}UART_NO_TAG;


void UART2_Init(ulong bound);
void UART3_Init(ulong bound);
void STM32_UART_Init(void);
void UART_Rx_process(void);
void STM32_UART_Write(u8 uartNo, void *src, u16 len);
u8 STM32_UART_Read(u8 uartNo, void *dst, u16 len);
void STM32_UART1_Write(uint8* Tx_Buff,uint16 Data_Len);
void Upload_ProcessDMAIRQ( void);
//void DMA1_DATA_Trans_Task_ON();
//void DMA1_DATA_Trans_Task_OFF();
u8 DMA1_Uart1_Status_Get( void);
void DMA_Uart1_Init( void);
static void UART1_Semphore_Init(void); 
	
extern unsigned char Task_Uart1_Rx;


#endif























/*********************************************END OF FILE**********************/
