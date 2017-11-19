#include "BSP_Common.h"  
static u8  fac_us=0;//us延时倍乘数
static u32 fac_ms=0;//ms延时倍乘数
static u32 fac_systic = 0;


/*************************************************************************************************
*****
***** 函数名:void  delay_init()
*****
***** 入口参数：无
*****
***** 功能描述：采用系统时钟作为延时时钟，初始化系统时钟，SYSTICK的时钟固定为HCLK时钟的1/8
*****
***** 调用： 无
*****
***** 返回值：
*****
***** 作者:Sandy
****	
****  版本：v1.0
****
**************************************************************************************************/
void delay_init()	 
{
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);	//选择外部时钟  HCLK/8
	fac_us=SystemCoreClock/8000000;	//system ticks per us
	fac_ms=(u32)fac_us*1000;//代表每个ms需要的systick时钟数   
	fac_systic = 1;
}								    


/*************************************************************************************************
*****
***** 函数名:void  delay_us()
*****
***** 入口参数：nus--延时的us数
*****
***** 功能描述：延时n us
*****
***** 调用： 无
*****
***** 返回值：
*****
***** 作者:Sandy
****	
****  版本：v1.0
****
**************************************************************************************************/		    								   
void delay_us(u32 nus)
{		
	u32 temp;	    	 
	SysTick->LOAD=nus*fac_us; //时间加载	  		 
	SysTick->VAL=0x00;        //清空计数器
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;          //开始倒数	 
	do
	{
		temp=SysTick->CTRL;
	}
	while(temp&0x01&&!(temp&(1<<16)));//等待时间到达   
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;       //关闭计数器
	SysTick->VAL =0X00;       //清空计数器	 
}


/*************************************************************************************************
*****
***** 函数名:void  delay_us()
*****
***** 入口参数：nms--延时的nms数,SysTick->LOAD为24位寄存器,所以,最大延时为:nms<=1864 
*****
***** 功能描述：延时n us
*****
***** 调用： 无
*****
***** 返回值：
*****
***** 作者:Sandy
****	
****  版本：v1.0
****
**************************************************************************************************/	
void delay_ms(u32 nms)
{	 		  	  
	u32 temp;		   
	SysTick->LOAD=nms*fac_ms;//时间加载(SysTick->LOAD为24bit)
	SysTick->VAL =0x00;           //清空计数器
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;          //开始倒数  
	do
	{
		temp=SysTick->CTRL;
	}
	while(temp&0x01&&!(temp&(1<<16)));//等待时间到达   
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;       //关闭计数器
	SysTick->VAL =0X00;       //清空计数器	  	    
} 


void delay_systick()
{
	u32 temp;	    	 
	SysTick->LOAD=fac_systic; //时间加载	  		 
	SysTick->VAL=0x00;        //清空计数器
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;          //开始倒数	 
	do
	{
		temp=SysTick->CTRL;
	}
	while(temp&0x01&&!(temp&(1<<16)));//等待时间到达   
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;       //关闭计数器
	SysTick->VAL =0X00;       //清空计数器	 
}

/*********************************************END OF FILE**********************/
