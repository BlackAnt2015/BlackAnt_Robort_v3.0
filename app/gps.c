#include "nmea/nmea.h"
#include "gps.h"

struct GPS_Info GPS_Info_Data;
nmeaINFO info;          //GPS解码后得到的信息
nmeaPARSER parser;      //解码时使用的数据结构  
uint8_t new_parse=0;    //是否有新的解码数据标志

void RobortGpsThreadServer(uint32 argAddr)
{
    S_Internal_Interface_Tag g2s = {0};
    S_Internal_Interface_Tag *pOutMsg = &g2s;
    S_Internal_Interface_Tag *pInMsg; 
	TState state = eError;
	TError error = 0;
	
    while (1)
    {
        state = TclReceiveMessage(&InternMsgInQueue, (void**)(&pInMsg),
                                  TCLO_IPC_WAIT, 0, &error);
        if (state == eError)
        {
            return ;
        }

        if (!(pInMsg->Entry_Type==PARSER_GPS_DAT_TAG))
            continue;

        if (pInMsg->data.p2g_tag.gps_cmd == GPS_ON)
        {
            //Open URAR3 or GPS
            
            while (1)
            {
                g2s.data.g2s_tag.msg_len = Robort_Fetch_GPS_Data(&g2s.data.g2s_tag.pmsg);
                if (g2s.data.g2s_tag.msg_len == 0)
                    continue ;

                state =TclSendMessage(&InternMsgOutQueue, (TMessage*)(&pOutMsg), TCLO_IPC_WAIT, 0, &error);
                TCLM_ASSERT((state == eSuccess), "");
                TCLM_ASSERT((error == TCLE_IPC_NONE), "");

                state = TclReceiveMessage(&InternMsgInQueue, (void**)(&pInMsg),
                                 TCLO_IPC_DEFAULT, 0, &error);

                if ((pInMsg->Entry_Type==PARSER_GPS_DAT_TAG) &&
                    (pInMsg->data.p2g_tag.gps_cmd == GPS_OFF))
                {
                    //Close UART3 or GPS

                }
            }
        } 

    }
}




static void GPS_DATA_Init( void)
{
    nmea_zero_INFO(&info);
    nmea_parser_init(&parser);
}


/*************************************************************************************************
*****
***** 函数名：void  STM32_GET_GPS_HalfTransferEnd_FLAG()
*****
***** 入口参数：无
*****
***** 功能描述：判断前半buffer传输是否完成
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
static u8 STM32_GET_GPS_HalfTransferEnd_FLAG()
{
	if( GPS_HalfTransferEnd)
	{
		return 1;
	}
	return 0;

}


/*************************************************************************************************
*****
***** 函数名：void  STM32_GET_GPS_TransferEnd_FLAG()
*****
***** 入口参数：无
*****
***** 功能描述：判断后半buffer传输是否完成
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
static u8 STM32_GET_GPS_TransferEnd_FLAG()
{
	if( GPS_TransferEnd)
	{
		return 1;
	}
	return 0;
}



/*************************************************************************************************
*****
***** 函数名：void  GPS_HalfTransferEnd_Nema()
*****
***** 入口参数：无
*****
***** 功能描述：解析前半buffer数据
*****
***** 调用： nmea_parse()解析出一个GPS数据实体
*****
***** 返回值：无
*****
***** 作者：Sandy
****				
****  版本：v2.0
****
**************************************************************************************************/
static void GPS_HalfTransferEnd_Nema()
{
	nmea_parse(&parser, (const char*)&gps_rbuff[0], HALF_GPS_RBUFF_SIZE, &info);  /* 进行nmea格式解码 */       
	GPS_HalfTransferEnd 		= 0;   					//清空标志位
	GPS_Info_Data.lat.fLat		= info.lat*100000; 		//纬度
	GPS_Info_Data.lon.fLon		= info.lon*100000;		//经度
	GPS_Info_Data.direction.fDir= info.direction*100000;//航向
	GPS_Info_Data.speed.fSpeed	= info.speed*100000;	//航速
	GPS_Info_Data.sAtinfo_iNuse	= info.satinfo.inuse;	//正在使用的
	GPS_Info_Data.sAtinfo_iNview= info.satinfo.inview;
	  
//	printf("\r\n纬度：%f\r\n",info.lat);
//	printf("\r\n纬度：%d\r\n",GPS_Info_Data.lat.fLat);
//	printf("\r\n纬度:%d %d %d %d\r\n",GPS_Info_Data.lat.uLat[0],GPS_Info_Data.lat.uLat[1],GPS_Info_Data.lat.uLat[2],GPS_Info_Data.lat.uLat[3]);

//	printf("\r\n正在使用的卫星：%d,可见卫星：%d",info.satinfo.inuse,info.satinfo.inview);
//	printf("\r\n海拔高度：%f 米 ", info.elv);
//	printf("\r\n速度：%f km/h ", info.speed);
//	printf("\r\n航向：%f 度", info.direction);

//	nmea_parser_destroy(&parser);
}



/*************************************************************************************************
*****
***** 函数名：void  GPS_TransferEnd_Nema()
*****
***** 入口参数：无
*****
***** 功能描述：解析后半buffer数据
*****
***** 调用： nmea_parse()解析出一个GPS数据实体
*****
***** 返回值：无
*****
***** 作者：Sandy
****				
****  版本：v2.0
****
**************************************************************************************************/
static void GPS_TransferEnd_Nema()
{

	nmea_parse(&parser, (const char*)&gps_rbuff[HALF_GPS_RBUFF_SIZE], HALF_GPS_RBUFF_SIZE, &info);
	GPS_TransferEnd = 0;
	GPS_Info_Data.lat.fLat		= info.lat*100000;
	GPS_Info_Data.lon.fLon		= info.lon*100000;
	GPS_Info_Data.direction.fDir	= info.direction*100000;
	GPS_Info_Data.speed.fSpeed	= info.speed*100000;
	GPS_Info_Data.sAtinfo_iNuse	= info.satinfo.inuse;
	GPS_Info_Data.sAtinfo_iNview	= info.satinfo.inview;
	
//    printf("\r\n纬度：%f,经度%f\r\n",info.lat,info.lon);
//    printf("\r\n正在使用的卫星：%d,可见卫星：%d",info.satinfo.inuse,info.satinfo.inview);
//    printf("\r\n速度：%f km/h ", info.speed);
//    printf("\r\n航向：%f 度", info.direction);
}

/*************************************************************************************************
*****
***** 函数名：void  Robort_Fetch_GPS_Data()
*****
***** 入口参数：pMsg -- 指向GPS数据首地址
*****
***** 功能描述：从GPS解析器链表中获取一个GPS信息实体
*****
***** 调用： GPS_HalfTransferEnd_Nema--解析GPS buffer中前一半数据
*****        GPS_TransferEnd_Nema -- 解析GPS buffer中后一半数据
*****        STM32_GET_GPS_HalfTransferEnd_FLAG() -- 前一半数据接收结束
*****        STM32_GET_GPS_TransferEnd_FLAG()  -- 后一半数据接收结束
*****
***** 返回值：msg_len -- GPS数据字节长度
*****
***** 作者：Sandy
****				
****
****  版本：v2.0
****
**************************************************************************************************/
uint16 Robort_Fetch_GPS_Data(void **pMsg)
{
	uint16 msg_len = 0;
	
	if(STM32_GET_GPS_HalfTransferEnd_FLAG())     /* 接收到GPS_RBUFF_SIZE一半的数据 */
	{     
		GPS_HalfTransferEnd_Nema();

		*pMsg = (void *)&GPS_Info_Data;
		msg_len = sizeof(GPS_Info_Data);
	}
	else if(STM32_GET_GPS_TransferEnd_FLAG())    /* 接收到另一半数据 */
	{

		GPS_TransferEnd_Nema();
		*pMsg = (void *)&GPS_Info_Data;
		msg_len = sizeof(GPS_Info_Data);
	}
	else
	{
		
		*pMsg = NULL;
		msg_len = 0;
	}

	return msg_len;
}

/*************************************************************************************************
*****
***** 函数名：void  Robort_GPS_Init()
*****
***** 入口参数：无
*****
***** 功能描述：GPS相关数据初始化及UART3初始化
*****
***** 调用： UART3_Init--UART3初始化
*****        STM32_UART3_DMA_Init -- 采用DMA传输UART3数据
*****        GPS_DATA_Init -- GPS数据解析器初始化
*****
***** 返回值：无
*****
***** 作者：Sandy
****				
****
****  版本：v2.0
****
**************************************************************************************************/
void Robort_GPS_Init(void)
{
    UART3_Init(9600);
	STM32_UART3_DMA_Init();
	GPS_DATA_Init();
}



/******************* (C) COPYRIGHT 2015 X-SPACE *****END OF FILE*********************/

