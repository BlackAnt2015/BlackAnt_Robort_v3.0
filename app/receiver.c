#include "receiver.h"


void RobortReceiverThreadServer(uint32 argAddr)
{
    App_Robort_Dat_Tag Protocol_Entry = {0};
    App_Robort_Dat_Tag *pProtocol = &Protocol_Entry; 
	TState state = eError;
	TError error = 0;
	uint8 ret = 0;
	
    while (1)
    {
        ret = STM32_UART_Read(STM32_UART1, &Protocol_Entry , sizeof(App_Robort_Dat_Tag));
        TCLM_ASSERT((ret == eSuccess), ""); 
        
        //Send protocol entry to parser thread
        state =TclSendMessage(&ExternInMsgQueue, (TMessage*)(&pProtocol), TCLO_IPC_WAIT, 0, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

    }
}

void Robort_Receiver_Init(void)
{
	TState state = eError;
	TError error = 0;
	
    STM32_UART_Init();
    
    /* 初始化消息队列 */
    state =TclCreateMsgQueue(&ExternInMsgQueue, "ExternInMsgQueue",(void**)(&ExternMsgInPool),
                           MQ_POOL_LEN, TCLP_IPC_DEFAULT, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IPC_NONE), "");
}


