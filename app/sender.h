#ifndef _APP_SENDER_H
#define _APP_SENDER_H
#include "common.h"
#include "../bsp/BSP_Uart.h"

void RobortSenderThreadServer(uint32 argAddr);
void Robort_Sender_Init(void);

static void delay_ms(uint32 ms);
static int Protocol_Upload(uint8 cmd_type, uint8 *pMsg, uint16 uMsgLen );
static void Robort_Send_Frame(void *pdata, uint16 len);
#endif

