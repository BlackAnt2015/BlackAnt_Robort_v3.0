#include "sender.h"


void RobortSenderThreadServer(uint32 argAddr)
{
    S_Internal_Interface_Tag *pSensorInfo = NULL;
	TState state = eError;
	TError error = 0;
	
    while (1)
    {
        state = TclReceiveMessage(&InternMsgOutQueue, (void**)(&pSensorInfo),
                                  TCLO_IPC_WAIT, 0, &error);
        if (state == eError)
        {
            return ;
        }

        if (pSensorInfo->Entry_Type == PARSER_GPS_DAT_TAG)
        {
           Protocol_Upload(ROBORT_APP_CMD_GPS, pSensorInfo->data.g2s_tag.pmsg, pSensorInfo->data.g2s_tag.msg_len);  
        }
        //else if 

    }
}

void Robort_Sender_Init(void)
{
	TState state = eError;
	TError error = 0;
	
    /* 初始化消息队列 */
    state =TclCreateMsgQueue(&InternMsgOutQueue, "InternMsgOutQueue",(void**)(&InternMsgOutPool),
                           MQ_POOL_LEN, TCLP_IPC_DEFAULT, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IPC_NONE), "");
}


/*************************************************************************************************
*****
***** 函数名：void  Protocol_Upload()
*****
***** 入口参数：无
*****
***** 功能描述：前照灯开启或关闭
*****
***** 调用： Robort_Fetch_GPS_Data()获取GPS数据
*****		 Robort_Send_Frame() 发送数据帧
*****
***** 返回值：无
*****
***** 作者：Sandy
****				
****  版本：v2.0
****
**************************************************************************************************/
int Protocol_Upload(uint8 cmd_type, uint8 *pMsg, uint16 uMsgLen )
{
	uint8 *pFrame 		= NULL;
	uint16 Frame_Length = 0;
	uint16 crc_code 	= 0;

	Frame_Length = FRAME_INFO_SIZE + uMsgLen;
	pFrame = (uint8*) malloc(Frame_Length);
	if (pFrame == NULL) //Malloc fail!
		return 0;
	
	*pFrame 	= 0xff; 			        //Frame Head
	*(pFrame+1) = APP_TERMINAL_ADDRESS; 	//App Address
	*(pFrame+2)	= uMsgLen & 0xff;		    //Msg Length Low 
	*(pFrame+3)	= (uMsgLen>>8) & 0xff;		//Msg Length High 
	*(pFrame+4)	= cmd_type;   	//Cmd type

	memcpy(pFrame+5, pMsg, sizeof(uint8)*uMsgLen);	//Copy message

	crc_code = Crc16(pFrame, Frame_Length-2);       //Crc Calculation	
	
	*(pFrame+Frame_Length-2) = (crc_code>>8) & 0xff;//Crc high
	*(pFrame+Frame_Length-1) = crc_code & 0xff; 	//Crc low

	Robort_Send_Frame(pFrame, Frame_Length);
	delay_ms(200);	//必须延时，否则释过快释放内存会导致发送数据不准确
	
	free(pFrame);		//Must do!!!
	
	return 1;
}

/*************************************************************************************************
*****
***** 函数名：void  Robort_Send_Frame()(v1.0没有用到)
*****
***** 入口参数：pdata--数据帧的指针
*****
***** 功能描述：向UART发送机器人获取的传感数据
*****
***** 调用： STM32_UART_Write()读取数据
*****
***** 返回值：无
*****
***** 作者：Sandy
****				
****  版本：v2.0
****
**************************************************************************************************/
static void Robort_Send_Frame(void *pdata, uint16 len)
{
     //STM32_UART_Write(STM32_UART1, pdata, sizeof(App_Robort_Dat_Tag));
     STM32_UART1_Write(pdata, len);
}


static void delay_ms(uint32 ms)
{
	uint32 us = 0;
	for ( ;ms>0; ms--)
	{
		for (us=0; us<7000; us++)
			;
	}
}