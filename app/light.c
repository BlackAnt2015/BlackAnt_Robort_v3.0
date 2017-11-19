#include "light.h"



void RobortLightThreadServer(uint32 argAddr)
{
    S_Internal_Interface_Tag p2l = {0};
    S_Internal_Interface_Tag *pInternMsg = &p2l; 
    uint8 lightStatus = 0;
    uint8 lightNo = 0;
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

        if (!(pInternMsg->Entry_Type==PARSER_LIGHT_DAT_TAG))
            continue ;
       
        lightStatus = pInternMsg->data.p2l_tag.light_status;
        lightNo     = pInternMsg->data.p2l_tag.light_no;

        if (lightNo == LIGHT_0)
        {
            if (LIGHT_ON==lightStatus) 
                STM32_GPIO_SetPin(U_GPIO_A, U_PIN_11, 1);
            else
                STM32_GPIO_SetPin(U_GPIO_A, U_PIN_11, 0); 
        }
        else if(lightNo == LIGHT_1)
        {
        
        }
    } 
}




/*************************************************************************************************
***** 函数名：void Robort_Light_Init(void)
*****
***** 入口参数：无
*****
***** 功能描述：初始化Lihgt相关的GPIO，初始状态时灯为熄灭状态
*****
***** 返回值：无
*****
***** 作者：
**************************************************************************************************/
void Robort_Light_Init(void)
{
	STM32_GPIO_ConfgPin(U_GPIO_A, U_PIN_11, U_MODE_OUT_PP, U_SPEED_50M);

    STM32_GPIO_SetPin(U_GPIO_A, U_PIN_11, 0);             //GPIOA_Pin11设置为低
}

