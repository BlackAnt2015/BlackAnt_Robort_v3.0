#include "BSP_ADC.h"

u16 ADC_Vaule_Buff[ADC_Ch_Num][ADC_Point]={0};   //电压采集缓冲区
u16 ADC_Vaule[ADC_Ch_Num]={0};                   //电压值,单位：mv
static u8  ADC_Ch=0;
static u8  ADC_Count=0;
 
																   
void  ADC1_Init(void)
{ 	
	ADC_InitTypeDef ADC_InitStructure; 
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOC |RCC_APB2Periph_ADC1, ENABLE );	  //使能ADC1通道时钟,同时开启PA\PC端口时钟
 

	RCC_ADCCLKConfig(RCC_PCLK2_Div6);                       //设置ADC分频因子6 72M/6=12,ADC最大时间不能超过14M

                       
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5;	//PA4、PA5 作为模拟通道输入引脚  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		    //模拟输入引脚
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5;	      //PA0~PC5 作为模拟通道输入引脚  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		    //模拟输入引脚
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	ADC_DeInit(ADC1);                                   //复位ADC1,将外设 ADC1 的全部寄存器重设为缺省值

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	//ADC工作模式:ADC1和ADC2工作在独立模式
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;	//模数转换工作在单通道模式
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	//模数转换工作在单次转换模式
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//转换由软件而不是外部触发启动
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//ADC数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel = 1;	//顺序进行规则转换的ADC通道的数目
	ADC_Init(ADC1, &ADC_InitStructure);	//根据ADC_InitStruct中指定的参数初始化外设ADCx的寄存器   

  
	ADC_Cmd(ADC1, ENABLE);	//使能指定的ADC1
	
	ADC_ResetCalibration(ADC1);	//使能复位校准  
	 
	while(ADC_GetResetCalibrationStatus(ADC1));	//等待复位校准结束
	
	ADC_StartCalibration(ADC1);	 //开启AD校准
 
	while(ADC_GetCalibrationStatus(ADC1));	 //等待校准结束
 

}	

//获得ADC值
u16 STM32_ADC_Gather(u8 ch)   
{
  	//设置指定ADC的规则组通道，一个序列，采样时间
	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5 );	//ADC1,ADC通道,采样时间为239.5周期	  			    
  
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//使能指定的ADC1的软件转换启动功能	
	 
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//等待转换结束

	return ADC_GetConversionValue(ADC1);	//返回最近一次ADC1规则组的转换结果
}

void ADC_Vaule_Handle(void)
{
	u32 Temp=0;
	u8  i=0,j=0;
	for(j=0;j<ADC_Ch_Num;j++)
	{
		for( i=0;i<ADC_Point;i++)
		 Temp = Temp+ADC_Vaule_Buff[j][i];
		
		Temp=Temp/ADC_Point;           //取平均

		
		ADC_Vaule[j]=Temp;
		Temp=0;
	}

}


void Task_ADC_Handle(void)
{
	
  if( ADC_Ch>=ADC_Ch_Num)
			ADC_Ch=0;
	switch( ADC_Ch)
		{
		case 0: ADC_Vaule_Buff[0][ADC_Count]=STM32_ADC_Gather(4); break;
		case 1: ADC_Vaule_Buff[1][ADC_Count]=STM32_ADC_Gather(5); break;
		case 2: ADC_Vaule_Buff[2][ADC_Count]=STM32_ADC_Gather(10);break;
		case 3: ADC_Vaule_Buff[3][ADC_Count]=STM32_ADC_Gather(11);break;
		case 4: ADC_Vaule_Buff[4][ADC_Count]=STM32_ADC_Gather(12);break;
		case 5: ADC_Vaule_Buff[5][ADC_Count]=STM32_ADC_Gather(13);break;
		case 6: ADC_Vaule_Buff[6][ADC_Count]=STM32_ADC_Gather(14);break;
		case 7: ADC_Vaule_Buff[7][ADC_Count]=STM32_ADC_Gather(15); 
		ADC_Count++;break;     //采集完最后一个通道，ADC_Count++
		default:                                                break;
	} 
		
	if(ADC_Count>=ADC_Point)                                             //采集完，进行平均滤波处理
	{
		ADC_Vaule_Handle();
		ADC_Count=0;
	}
	ADC_Ch++;
}
 
u16 STM32_ADC_Read( u8 ch)
{
	return ADC_Vaule[ch];
}	
/*********************************************END OF FILE*********************a*/
