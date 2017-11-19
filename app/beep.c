#include "beep.h"




/*************************************************************************************************
*****
***** 函数名：void  Robort_Beep_Init()
*****
***** 入口参数：beep_status--蜂鸣器状蹋(ROBORT_BEEP_ON-开启, ROBORT_BEEP_OFF-关闭) 
*****
***** 功能描述：蜂鸣器状态控制
***** 
*****							
***** 调用： STM32_GPIO_SetPin()		   
*****
***** 返回值：无
*****
***** 作者：Sandy
****				
****  版本：v3.0
**************************************************************************************************/
void RobortBeepThreadServer(uint32 argAddr)
{
    S_Internal_Interface_Tag p2b = {0};
    S_Internal_Interface_Tag *pInternMsg = &p2b; 
    uint8 beepStatus = 0;
    uint8 beepNo = 0;
	TState state = eError;
	TError error = 0;
	
    while (1)
    {
        state = TclReceiveMessage(&InternMsgInQueue, (void**)(&pInternMsg),
                                  TCLO_IPC_WAIT, 0, &error);
        if (state == eError)
        {
            return ;
        }

        if (!(pInternMsg->Entry_Type==PARSER_BEEP_DAT_TAG))
            continue ;
       

        beepStatus = pInternMsg->data.p2b_tag.beep_status;
        beepNo = pInternMsg->data.p2b_tag.beep_no;

        if (beepNo == BEEP_0)
        {
            if (BEEP_ON==beepStatus) 
                STM32_GPIO_SetPin( U_GPIO_A, U_PIN_12, 1);
            else
                STM32_GPIO_SetPin( U_GPIO_A, U_PIN_12, 0); 
        }
        else if(beepNo == BEEP_1)
        {
        
        }
    }
}



/*************************************************************************************************
*****
***** 函数名：void  Robort_Beep_Init()
*****
***** 入口参数：无
*****
***** 功能描述：蜂鸣器初始化, GPIOA-Pin12-50M, 
***** 
*****							
***** 调用： STM32_GPIO_ConfgPin(), STM32_GPIO_SetPin()		   
*****
***** 返回值：无
*****
***** 作者：Sandy
****				
****  版本：v3.0
**************************************************************************************************/
void Robort_Beep_Init(void)
{
	STM32_GPIO_ConfgPin(U_GPIO_A, U_PIN_12, U_MODE_OUT_PP, U_SPEED_50M);
	
    STM32_GPIO_SetPin( U_GPIO_A, U_PIN_12, 0); 
}

