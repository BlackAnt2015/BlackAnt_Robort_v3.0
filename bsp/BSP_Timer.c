#include "BSP_Timer.h" 

static uint32 Task_10ms=0;
uchar Task_20ms=0,Task_50ms=0,Task_100ms=0,Task_500ms=0,Task_1s=0;


void TIM1_Int_Init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE); //时钟使能

	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	 计数到5000为500ms
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值  10Khz的计数频率  
	TIM_TimeBaseStructure.TIM_ClockDivision = 1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
 
	TIM_ITConfig(  //使能或者失能指定的TIM中断
		TIM1, //TIM2
		TIM_IT_Update ,
		ENABLE  //使能
		);
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;  //TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器

	TIM_Cmd(TIM1, ENABLE);  //使能TIMx外设
							 
}
void NVIC_Configuration(void)
{

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//设置NVIC中断分组2:2位抢占优先级，2位响应优先级
}

void TIMER_Init(void)
{
	NVIC_Configuration();
	TIM1_Int_Init(39,3599);//10Khz的计数频率，计数到99为10ms 
}

void TIM1_UP_IRQHandler(void)   //TIM3中断
{
//	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) //检查指定的TIM中断发生与否:TIM 中断源 
//		{
			
	TIM_ClearITPendingBit(TIM1, TIM_IT_Update);  //清除TIMx的中断待处理位:TIM 中断源 
	Task_10ms++;
	if( Task_10ms>=1000){
		Task_10ms=0;
	}

	if( Task_10ms%2==0)
		Task_20ms=1;

	if( Task_10ms%5==0)
		Task_50ms=1;

	if( Task_10ms%10==0)
		Task_100ms=1;

	if( Task_10ms%50==0)
		Task_500ms=1;

	if( Task_10ms%100==0)
		Task_1s=1;
	
//		}
}

u8 STM32_TIMER3_GetCounterFlag(void)
{
	
	u8 temp;
	temp=Task_20ms+(Task_50ms<<1)+(Task_100ms<<2)+(Task_500ms<<3)+ (Task_1s<<4);
	return temp;
	
}
/*********************************************END OF FILE**********************/
