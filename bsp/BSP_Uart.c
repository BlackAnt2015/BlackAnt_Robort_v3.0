#include "BSP_Common.h" 


#include "BSP_Uart.h"

uchar Tx_Buffer[Tx_Array_Size];			     //发送缓存区
uchar Rx_Buffer[Rx_Array_Size];			     //接受缓存区
uchar Rx_Process_Buffer[Rx_Array_Size];	     //接受处理缓存区
uchar Tx_Ptr,Rx_Ptr; 	 		             //发送接收指针
uchar Tx_Tail,Rx_Tail;    			   	     //发送接收尾指针
uchar Rx_Step=0;							 //接收步骤
uchar Tx_Count,Rx_Count;				     //发送接收计时
uchar Temp=0;
uchar i=0;
uchar DMA1_Uart_Status=0;

/* 用户信号量定义 */
static TSemaphore Uart1Semaphore;

void UART2_Init(ulong bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	/* config USART2 clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	/* USART2 GPIO config */
	/* Configure USART2 Tx (PA.02) as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure USART2 Rx (PA.03) as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* USART2 mode config */
	USART_InitStructure.USART_BaudRate = bound;                //GPS模块默认使用波特率：9600
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(USART2, &USART_InitStructure); 

	USART_Cmd(USART2, ENABLE);
}

/*************************************************************************************************
*****
***** 函数名:void  UART3_Init()
*****
***** 入口参数：无
*****
***** 功能描述：uart3初始化, 9600B+StartBit+8BitsData+StopBit
*****
***** 调用： 无
*****
***** 返回值：
*****
***** 作者:Sandy
****	
****  版本：v2.0
****
**************************************************************************************************/
void UART3_Init(ulong bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	/* config USART2 clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

	/* USART2 GPIO config */
	/* Configure USART2 Tx (PA.02) as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* Configure USART2 Rx (PA.03) as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* USART2 mode config */
	USART_InitStructure.USART_BaudRate = bound;                //GPS模块默认使用波特率：9600
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(USART3, &USART_InitStructure); 

	USART_Cmd(USART3, ENABLE);
}

/*************************************************************************************************
*****
***** 函数名:void  UART1_Init()
*****
***** 入口参数：无
*****
***** 功能描述：uart1初始化, 9600B+StartBit+8BitsData+StopBit
*****
***** 调用： 无
*****
***** 返回值：
*****
***** 作者:Sandy
****	
****  版本：v2.0
****
**************************************************************************************************/
void UART1_Init( ulong bound)
{
	//GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//使能USART1，GPIOA时钟
	USART_DeInit(USART1);  //复位串口1
	//USART1_TX   PA.9
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure); //初始化PA9

	//USART1_RX	  PA.10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);  //初始化PA10

	//Usart1 NVIC 配置

	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器

	//USART 初始化设置

	USART_InitStructure.USART_BaudRate = bound;//一般设置为9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

	USART_Init(USART1, &USART_InitStructure); //初始化串口
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启中断
	USART_Cmd(USART1, ENABLE);                    //使能串口 
}

/*************************************************************************************************
*****
***** 函数名:void  STM32_UART_Init()
*****
***** 入口参数：无
*****
***** 功能描述：uart1, uart2, uart3的初始化
*****
***** 调用： 无
*****
***** 返回值：
*****
***** 作者:Sandy
****	
****  版本：v2.0
****
**************************************************************************************************/
void STM32_UART_Init(void)
{
	UART1_Init(9600);
	//UART2_Init(9600);
	
	DMA_Uart1_Init();

    UART1_Semphore_Init(); 
}

static void UART1_Semphore_Init(void) 
{
    TState state;
    TError error;

    /* 初始化信号量 */
    state = TclCreateSemaphore(&Uart1Semaphore, "uart1 semaphore", 0, 1, TCLP_IPC_DEFAULT, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IPC_NONE), "");
}


/*************************************************************************************************
*****
***** 函数名:void  USART1_IRQHandler()
*****
***** 入口参数：无
*****
***** 功能描述：UART1的中断处理
*****
***** 调用： 无
*****
***** 返回值：
*****
***** 作者:Sandy
****	
****  版本：v2.0
****
**************************************************************************************************/
void USART1_IRQHandler(void)                	//串口1中断服务程序
{
	TState state = eError;
	TError error = 0;
	
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //接收中断
	{
		Rx_Buffer[Rx_Ptr] =USART_ReceiveData(USART1);//(USART1->DR);	//读取接收到的数据
		if(Rx_Ptr>(Rx_Array_Size-1)) Rx_Ptr=0;
		Rx_Ptr++;
		switch (Rx_Step)							//开始标志
		{
			case 0x00:								//校验帧头
				Rx_Count=0;
				if(Rx_Buffer[0]==0xFF) 
					Rx_Step++; 
   				else 
					Rx_Ptr=0;
				break;
			case 0x01:								//本机模块号：0x01
				Rx_Count=0;
				if(Rx_Buffer[1]==0x01)
				{
					Rx_Step++;
				}	
				else 
				{           
					Rx_Ptr=0;
					Rx_Step=0;
				}
				break;
			case 0x02:											
			    Rx_Count=0;
				if(Rx_Ptr==4)
				{
					Temp=Rx_Buffer[2];
					Rx_Tail= ((u16)Rx_Buffer[3]<<8 | Temp) +7;			
					if (Rx_Tail>(Rx_Array_Size-1))			// 长度无效
					{
						Rx_Ptr=0;
						Rx_Step=0;
					}
					else 
					{
						Rx_Step++;
					}
				}
				break;
			case 0x03:										//提取数据
				Rx_Count=0;
				if(Rx_Ptr==Rx_Tail)
				{
					for ( i=0;i<Rx_Tail;i++)
					{
						Rx_Process_Buffer[i]=Rx_Buffer[i]; 
					}
					Rx_Ptr=0;
					Rx_Tail=0;
					Rx_Step=0;

					/* ISR Release semaphore for receiver thread */
                    state = TclReleaseSemaphore(&Uart1Semaphore, 0, 0, &error);
                    TCLM_ASSERT((state == eSuccess), "");
                    TCLM_ASSERT((error == TCLE_IPC_NONE), "");
				}
				break;
			default: break;  											// 
			}
	}
		
}


/*************************************************************************************************
*****
***** 函数名:void  STM32_UART_Write()
*****
***** 入口参数：uartNo--UART号; src--用户输入缓冲区; len--数据长度(无用)
*****
***** 功能描述：从APP处理模块发送数据给UART
*****
***** 调用： 无
*****
***** 返回值：
*****
***** 作者:Sandy
****	
****  版本：v2.0
****
**************************************************************************************************/
void STM32_UART_Write(u8 uartNo, void *src, u16 len)
{
	u8 t=0;
	if( 1 ==uartNo)
	{
		for(t=0;t<len;t++)
		{
			USART1->DR =*((u8*)src+t);
			while((USART1->SR&0X40)==0);//查询发送是否结束
		}
	}
	else if( 2==uartNo)
	{
		for(t=0;t<len;t++)
		{
			USART2->DR =*((u8*)src+t);
			while((USART2->SR&0X40)==0);//查询发送是否结束
		}
	}
}


/*************************************************************************************************
*****
***** 函数名:void  STM32_UART_Read()
*****
***** 入口参数：uartNo--UART号; dst--用户输出缓冲区; len--数据长度(无用)
*****
***** 功能描述：从UART接收模块读取数据返回给APP处理模块
*****
***** 调用： 无
*****
***** 返回值：0--失败；1--成功；
*****
***** 作者:Sandy
****	
****  版本：v2.0
****
**************************************************************************************************/

u8 STM32_UART_Read(u8 uartNo, void *dst, u16 len)
{
	u16 dat_len 	= 0;
	u16 crc_code 	= 0;
	u16 msg_len 	= 0; 
	TState state = eError;
	TError error = 0;
	App_Robort_Dat_Tag *pRecDat = (App_Robort_Dat_Tag *)dst;
	
    /* Receiver线程以阻塞方式获取信号量 */
    state = TclObtainSemaphore(&Uart1Semaphore, TCLO_IPC_WAIT, 0, &error);
    if (state == eError)
    {
        return 0;
    }
		
    //1. 数据信息的长度
   	msg_len = ((u16)Rx_Process_Buffer[3]<<8 | Rx_Process_Buffer[2]);
		
	//2. 数据帧总长度
	dat_len = msg_len + FRAME_INFO_SIZE;
		
	//3. CRC校验
	crc_code=Crc16(&Rx_Process_Buffer[0], dat_len-2); 
	if (( (crc_code&0xff) != Rx_Process_Buffer[dat_len-1]) || 
		(((crc_code>>8)&0xff) != Rx_Process_Buffer[dat_len-2]))
	{
		//return 0;
	}		
		
	//4. 数据帧信息拷备
	pRecDat->pMsg = (uint8 *)malloc(msg_len);
		
	//(1) copy frame info
	pRecDat->Frame_Head 	= Rx_Process_Buffer[0];
	pRecDat->Addr 			= Rx_Process_Buffer[1];
	pRecDat->Msg_L			= Rx_Process_Buffer[2];
	pRecDat->Msg_H			= Rx_Process_Buffer[3];
	pRecDat->Cmd 			= Rx_Process_Buffer[4];
	pRecDat->Crc_L 			= Rx_Process_Buffer[dat_len-2]; //CRC_L
	pRecDat->Frame_Head 	= Rx_Process_Buffer[dat_len-1]; //CRC_H
		
	//(2) copy message info
	memcpy(pRecDat->pMsg, &Rx_Process_Buffer[5], msg_len);
		
	return 1;
}

 

/*************************************************************************************************
*****
***** 函数名:void  DMA_Uart1_Init()
*****
***** 入口参数：无
*****
***** 功能描述：设置UART1为DMA发送
*****
***** 调用： 无
*****
***** 返回值：
*****
***** 作者:Sandy
****	
****  版本：v2.0
****
**************************************************************************************************/
 void DMA_Uart1_Init( void)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//使能DMA传输
	DMA_DeInit(DMA1_Channel4);   //将DMA的通道1寄存器重设为缺省值
	DMA_InitTypeDef DMA_InitStructure;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART1->DR;  //DMA外设ADC基地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)Tx_Buffer;  //DMA内存基地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;  //数据传输方向，从内存读取发送到外设
	DMA_InitStructure.DMA_BufferSize = 10;  //DMA通道的DMA缓存的大小
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //外设地址寄存器不变
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //内存地址寄存器递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //数据宽度为8位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //数据宽度为8位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  //工作在正常缓存模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; //DMA通道 x拥有中优先级 
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMA通道x没有设置为内存到内存传输
	DMA_Init(DMA1_Channel4, &DMA_InitStructure);  //根据DMA_InitStruct中指定的参数初始化DMA的通道USART1_Tx_DMA_Channel所标识的寄存器
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

	// DMA2 Channel Interrupt ENABLE
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	DMA_ITConfig(DMA1_Channel4,DMA_IT_TC,ENABLE);  //配置DMA发送完成后产生中断

	/*使能DMA*/
	DMA_Cmd (DMA1_Channel4,ENABLE);		
    
  /* 配置串口 向 DMA发出TX请求 */
	USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
}

/*设置 UART1-DMA为打开*/
void DMA1_DATA_Trans_Task_ON()
{
	DMA1_Uart_Status=1;
}

/*设置 UART1-DMA为关闭*/
void DMA1_DATA_Trans_Task_OFF()
{
	DMA1_Uart_Status=0;
}

u8 DMA1_Uart1_Status_Get( void)
{
	return DMA1_Uart_Status;
}

/*************************************************************************************************
*****
***** 函数名:void  STM32_UART1_Write()
*****
***** 入口参数：Tx_Buff--发送数据缓冲区；
*****           Data_Len--发送数据的长度
*****
***** 功能描述：UART1发送函数
*****
***** 调用： 无
*****
***** 返回值：
*****
***** 作者:Sandy
****	
****  版本：v2.0
****
**************************************************************************************************/
void STM32_UART1_Write(uint8* Tx_Buff,uint16 Data_Len)
{
	DMA1_Channel4->CCR&=~(1<<0);
	DMA1_Channel4->CMAR=(u32)Tx_Buff;
	DMA1_Channel4->CNDTR=Data_Len;
	DMA1_Channel4->CCR|=(1<<0);
	DMA1_DATA_Trans_Task_ON();
}

/*************************************************************************************************
*****
***** 函数名:void  Upload_ProcessDMAIRQ()
*****
***** 入口参数：无
*****
***** 功能描述：UART1上传数据结束中断
*****
***** 调用： 无
*****
***** 返回值：
*****
***** 作者:Sandy
****	
****  版本：v2.0
****
**************************************************************************************************/
void Upload_ProcessDMAIRQ( void)
{
	if( DMA_GetITStatus(DMA1_IT_TC4))
	{
		DMA1_DATA_Trans_Task_OFF();
		DMA_ClearFlag(DMA1_FLAG_TC4);//清除通道4传输完成标志
	}
}
 
/******************* (C) COPYRIGHT 2015 X-SPACE *****END OF FILE*********************/
