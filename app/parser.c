#include "parser.h"

S_Internal_Interface_Tag  *pInterMsg;
App_Robort_Dat_Tag *pExternMsg;

void Robort_Parser_Init(void)
{
	TState state = eError;
	TError error = 0;
	
    /* 初始化消息队列 */
    state =TclCreateMsgQueue(&InternMsgInQueue, "InterMsgInQueue",(void**)(&InternMsgInPool),
                           MQ_POOL_LEN, TCLP_IPC_DEFAULT, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IPC_NONE), ""); 
}

void RobortParserThreadServer(uint32 argAddr)
{ 
    TState state;    
    uint8 ret;    
	TError error = 0;
	S_Internal_Interface_Tag InternMsg = {0};
	
	pInterMsg = &InternMsg;

    while (1)
    {
        /* 1. Get a protocol entry */
        state = TclReceiveMessage(&ExternInMsgQueue, (void**)(pExternMsg),
                                  TCLO_IPC_WAIT, 0, &error);

        if (state == eSuccess)
        {
            /* 2. Parser the protocol entry */
            ret = Protocol_Parser(pExternMsg, &pInterMsg);
            TCLM_ASSERT((ret == eSuccess), "");

            /* 3. Broadcast internal message to function threads */
            ret = BroadcastInternalMessage(&pInterMsg);
        }
    }
}

static TState BroadcastInternalMessage(S_Internal_Interface_Tag **pInternMsg)
{
    TState state;
    TError error;

    TclBroadcastMessage(&InternMsgInQueue, (TMessage*)pInternMsg, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IPC_NONE), "");
	
	return eSuccess;
}


static unsigned short Resolve_Message_Data(App_Robort_Dat_Tag *pdata, uint8 *pDst)
{
	uint16 msg_len = 0;
	msg_len = Assemble_Message_Length(pdata);
	
	if (msg_len == 0)
		return 0;
	
	memcpy(pDst, pdata->pMsg, msg_len);
	
	return msg_len;
}


/* Get msg length */ 
static unsigned short Assemble_Message_Length(App_Robort_Dat_Tag *pdata)
{
	unsigned short len;
	len = (pdata->Msg_H << 8) | (pdata->Msg_L);
	
	return len;
}

/* Compare local addr & slave addr */ 
static uint8 Robort_Adress_Veritfy(App_Robort_Dat_Tag *pdata)
{
	return (pdata->Addr == ROBORT_LOCAL_ADDRESS) ;
}


/*************************************************************************************************
*****
***** 函数名：void  Process_Robort_Direction_Cmd()
*****
***** 入口参数：无
*****
***** 功能描述：将运动方向参数解析并送到motor控制程序
*****
***** 调用： Resolve_Message_Data()恢复数据信息
*****		 Robort_Direction_Control() 控制机器人运行方向
*****
***** 返回值：无
*****
***** 作者：Sandy
****				
****
****  版本：v2.0
****
**************************************************************************************************/
uint8 Process_Robort_Direction_Cmd(App_Robort_Dat_Tag *pExternMsg, S_Internal_Interface_Tag **pInternMsg)
{
	unsigned short msg_len = 0;
    uint8 speed = 0;
    
	if (pExternMsg == NULL)
		return 0;
	
	msg_len = Resolve_Message_Data(pExternMsg, &speed);
	if (msg_len == 0)
	{
		speed = ROBORT_SPEED_DEFAULT;
	}
	
	(*pInternMsg)->data.p2m_tag.type = MOTOR_DIR_TYPE;
	(*pInternMsg)->data.p2m_tag.speed = speed;
	
	if (pExternMsg->Cmd == APP_ROBORT_CMD_STOP)
	{
		(*pInternMsg)->data.p2m_tag.direction = ROBORT_STOP;
	}
	else if (pExternMsg->Cmd == APP_ROBORT_CMD_FORWARD)
	{
		(*pInternMsg)->data.p2m_tag.direction = ROBORT_FORWARD;
	}
	else if (pExternMsg->Cmd == APP_ROBORT_CMD_BACK)
	{
		(*pInternMsg)->data.p2m_tag.direction = ROBORT_BACK;
	}
	else if (pExternMsg->Cmd == APP_ROBORT_CMD_LEFT)
	{
		(*pInternMsg)->data.p2m_tag.direction = ROBORT_LEFT;
	}
	else if (pExternMsg->Cmd == APP_ROBORT_CMD_RIGHT)
	{
		(*pInternMsg)->data.p2m_tag.direction = ROBORT_RIGHT;
	}
	
	
	return 1;
}



/*************************************************************************************************
*****
***** 函数名：void  Process_Robort_Steer_Cmd()
*****
***** 入口参数：无
*****
***** 功能描述：将舵机转动方向参数解析并送到steer控制程序
*****
***** 调用： Resolve_Message_Data()恢复数据信息
*****		 Robort_Steer_Control() 控制舵机转动
*****
***** 返回值：无
*****
***** 作者：Sandy
****				
****
****  版本：v2.0
****
**************************************************************************************************/
static uint8 Process_Robort_Steer_Cmd(App_Robort_Dat_Tag *pExternMsg, S_Internal_Interface_Tag **pInternMsg)
{
	unsigned short msg_len = 0;
	uint8 angle = 0;
	
	if (pExternMsg == NULL)
		return 0;
	
	msg_len = Resolve_Message_Data(pExternMsg, &angle);
	if (msg_len == 0)
	{
		return 0;
	}
	
	(*pInternMsg)->data.p2s_tag.angle;
	
	if (pExternMsg->Cmd == APP_ROBORT_CMD_STEER1)
	{
		(*pInternMsg)->data.p2s_tag.steer_no = ROBORT_STEER1;
	}
	else if (pExternMsg->Cmd == APP_ROBORT_CMD_STEER2)
	{
		(*pInternMsg)->data.p2s_tag.steer_no = ROBORT_STEER2;
	}
	else if (pExternMsg->Cmd == APP_ROBORT_CMD_STEER3)
	{
		(*pInternMsg)->data.p2s_tag.steer_no = ROBORT_STEER3;
	}
	else if (pExternMsg->Cmd == APP_ROBORT_CMD_STEER4)
	{
		(*pInternMsg)->data.p2s_tag.steer_no = ROBORT_STEER4;
	}
	else if (pExternMsg->Cmd == APP_ROBORT_CMD_STEER5)
	{
		(*pInternMsg)->data.p2s_tag.steer_no = ROBORT_STEER5;
	}
	else if (pExternMsg->Cmd == APP_ROBORT_CMD_STEER6)
	{
		(*pInternMsg)->data.p2s_tag.steer_no = ROBORT_STEER6;
	}
	else if (pExternMsg->Cmd == APP_ROBORT_CMD_STEER7)
	{
		(*pInternMsg)->data.p2s_tag.steer_no = ROBORT_STEER7;
	}
	else if (pExternMsg->Cmd == APP_ROBORT_CMD_STEER8)
	{
		(*pInternMsg)->data.p2s_tag.steer_no = ROBORT_STEER8;
	}
	
	return 1;
}

/*************************************************************************************************
*****
***** 函数名：void  Process_Robort_Speed_Cmd()
*****
***** 入口参数：无
*****
***** 功能描述：调整机器人运动速度
*****
***** 调用： Resolve_Message_Data()恢复数据信息
*****		 Robort_Speed_Control() 调整机器人速度
*****
***** 返回值：无
*****
***** 作者：Sandy
****				
****  版本：v2.0
****
**************************************************************************************************/
static uint8 Process_Robort_Speed_Cmd(App_Robort_Dat_Tag *pExternMsg, S_Internal_Interface_Tag **pInternMsg)
{
	uint8 speed = 0;
    unsigned short msg_len = 0;
    
    if (pExternMsg == NULL)
		return 0;
	
	msg_len = Resolve_Message_Data(pExternMsg, &speed);
	if (msg_len == 0)
	{
		return 0;
	}

    if (speed>100)
    {
        return 0;
    }
    
	(*pInternMsg)->data.p2m_tag.type = MOTOR_SPEED_TYPE;
	(*pInternMsg)->data.p2m_tag.speed = speed;
	
	if (pExternMsg->Cmd == APP_ROBORT_CMD_SPEED_L)
	{
		(*pInternMsg)->data.p2m_tag.speed_dir = ROBORT_SPEED_L;
	}
	else if (pExternMsg->Cmd == APP_ROBORT_CMD_SPEED_R)
	{
		(*pInternMsg)->data.p2m_tag.speed_dir = ROBORT_SPEED_R;
	}
    
    return 1;
}

/*************************************************************************************************
*****
***** 函数名：void  Process_Robort_Light_Cmd()
*****
***** 入口参数：无
*****
***** 功能描述：前照灯开启或关闭
*****
***** 调用： Resolve_Message_Data()恢复数据信息
*****		 Robort_Light_Control() 前照灯控制
*****
***** 返回值：无
*****
***** 作者：Sandy
****				
****  版本：v2.0
****
**************************************************************************************************/
static uint8 Process_Robort_Light_Cmd(App_Robort_Dat_Tag *pExternMsg, S_Internal_Interface_Tag **pInternMsg)
{
    uint8 lightNum = 0;
    unsigned short msg_len = 0;   

	if (pExternMsg == NULL)
		return 0;
      
	msg_len = Resolve_Message_Data(pExternMsg, &lightNum);
	if (msg_len == 0)
	{
		return 0;
	}

    (*pInternMsg)->Entry_Type = PARSER_LIGHT_DAT_TAG; 
    (*pInternMsg)->data.p2b_tag.beep_no = lightNum;

	if (pExternMsg->Cmd == APP_ROBORT_CMD_LIGHT_ON)
	{
        (*pInternMsg)->data.p2l_tag.light_status = LIGHT_ON; 
	}
	else
	{
        (*pInternMsg)->data.p2l_tag.light_status = LIGHT_OFF; 
	}
     
	return 1;
}

/*************************************************************************************************
*****
***** 函数名：void  Process_Robort_Beep_Cmd()
*****
***** 入口参数：无
*****
***** 功能描述：前照灯开启或关闭
*****
***** 调用： Resolve_Message_Data()恢复数据信息
*****		 Robort_Beep_Control() 喇叭控制
*****
***** 返回值：无
*****
***** 作者：Sandy
****				
****  版本：v2.0
****
**************************************************************************************************/
static uint8 Process_Robort_Beep_Cmd(App_Robort_Dat_Tag *pExternMsg, S_Internal_Interface_Tag **pInternMsg)
{
	uint16  msg_len = 0;
	uint8 beepNum = 0;
	
	if (pExternMsg == NULL)
		return 0;
	
   	msg_len = Resolve_Message_Data(pExternMsg, &beepNum);
	if (msg_len == 0)
	{
		return 0;
	} 

    (*pInternMsg)->Entry_Type = PARSER_BEEP_DAT_TAG; 
    (*pInternMsg)->data.p2b_tag.beep_no = beepNum;

	if (pExternMsg->Cmd == APP_ROBORT_CMD_BEEP_ON)
	{
        (*pInternMsg) ->data.p2b_tag.beep_status = BEEP_ON; 
	}
	else
	{
        (*pInternMsg) ->data.p2b_tag.beep_status = BEEP_OFF; 
	}

	return 1;
}


 

/*************************************************************************************************
*****
***** function：uint8  Protocol_Parser()
*****
***** input：pdata -- pointer to protocol command received
*****        
*****
***** output: none
*****
***** description：parse the protocol based on APP_ROBORT_CMD_TYPE
*****
*****
***** call：Process_Robort_Direction_Cmd() -- parse robort's direction cmd 
*****       Process_Robort_Steer_Cmd() -- parse robort's steer cmd
*****       Process_Robort_Light_Cmd() -- parse robort's light cmd
*****       Process_Robort_Beep_Cmd()  --  parse robort's beep cmd
*****       Process_Robort_Sensor_Cmd() -- parse robort's sensor cmd
*****       
*****   
*****
***** return:   0 -- error
*****           1 -- ok
*****
***** author：Sandy
*****	
***** version: 2.0
**************************************************************************************************/
static uint8 Protocol_Parser(App_Robort_Dat_Tag *pExternMsg, S_Internal_Interface_Tag **pInternMsg)
{
	if (0 == Robort_Adress_Veritfy(pExternMsg))
	{
		return 0;
	}
	
	switch (pExternMsg->Cmd)
	{
		case APP_ROBORT_CMD_STOP:
		case APP_ROBORT_CMD_FORWARD:
		case APP_ROBORT_CMD_BACK:
		case APP_ROBORT_CMD_LEFT:
		case APP_ROBORT_CMD_RIGHT:
			Process_Robort_Direction_Cmd(pExternMsg, pInternMsg);
			break;
		case APP_ROBORT_CMD_STEER1:
		case APP_ROBORT_CMD_STEER2:
		case APP_ROBORT_CMD_STEER3:
		case APP_ROBORT_CMD_STEER4:
		case APP_ROBORT_CMD_STEER5:
		case APP_ROBORT_CMD_STEER6:
		case APP_ROBORT_CMD_STEER7:
		case APP_ROBORT_CMD_STEER8:
			Process_Robort_Steer_Cmd(pExternMsg, pInternMsg);
			break;
		case APP_ROBORT_CMD_SPEED_L:
		case APP_ROBORT_CMD_SPEED_R:
			Process_Robort_Speed_Cmd(pExternMsg, pInternMsg);
			break;
		case APP_ROBORT_CMD_LIGHT_ON:
		case APP_ROBORT_CMD_LIGHT_OFF:
			Process_Robort_Light_Cmd(pExternMsg, pInternMsg);
			break;
	    case APP_ROBORT_CMD_BEEP_ON:
		case APP_ROBORT_CMD_BEEP_OFF:
			Process_Robort_Beep_Cmd(pExternMsg, pInternMsg);
			break;
		//case APP_ROBORT_CMD_SENSOR:
			//Process_Robort_Sensor_Cmd(pExternMsg, pInternMsg); //暂时没用到
			//break;
//		case APP_ROBORT_CMD_GPS_ON:
//	    case APP_ROBORT_CMD_GPS_OFF:
//		       	Process_Robort_GPS_Cmd(pExternMsg, pInternMsg); 
//			break;	
		default: return 0;
	}
	
	return 1;
}





























