//#include "Common.h" 

#include "BSP_Gpio.h"

/*************************************************************************************************
*****
***** 函数名：void  STM32_GPIO_SetPin()
*****
***** 入口参数：gpio--GPIO号, pin--引脚号(0~15), mode--输入/输出, speed--输出速度
*****
***** 功能描述：对GPIO引脚进行设置
***** 
*****							
***** 调用： GPIO_SetBits()--置位引脚PIO_ResetBits()--复位引脚  		   
*****
***** 返回值：无
*****
***** 作者：Sandy
****				
****  版本：v2.0
****
**************************************************************************************************/
void STM32_GPIO_SetPin( uint8_t gpio, uint8_t pin, uint8_t val)
{
	GPIO_TypeDef* GPIOx = GPIOA;
	uint16_t GPIO_Pin	= GPIO_Pin_0;
	
	switch (gpio)
	{
		case U_GPIO_A: GPIOx = GPIOA; break;
		case U_GPIO_B: GPIOx = GPIOB; break;
		case U_GPIO_C: GPIOx = GPIOC; break;
		case U_GPIO_D: GPIOx = GPIOD; break;
		case U_GPIO_E: GPIOx = GPIOE; break;
		case U_GPIO_F: GPIOx = GPIOF; break;
		case U_GPIO_G: GPIOx = GPIOG; break;
		default:
			assert_param(0);
		break;
	}
	
	switch (pin)
	{
		case U_PIN_0:  GPIO_Pin = GPIO_Pin_0; break;
		case U_PIN_1:  GPIO_Pin = GPIO_Pin_1; break;
		case U_PIN_2:  GPIO_Pin = GPIO_Pin_2; break;
		case U_PIN_3:  GPIO_Pin = GPIO_Pin_3; break;
		case U_PIN_4:  GPIO_Pin = GPIO_Pin_4; break;
		case U_PIN_5:  GPIO_Pin = GPIO_Pin_5; break;
		case U_PIN_6:  GPIO_Pin = GPIO_Pin_6; break;
		case U_PIN_7:  GPIO_Pin = GPIO_Pin_7; break;
		case U_PIN_8:  GPIO_Pin = GPIO_Pin_8; break;
		case U_PIN_9:  GPIO_Pin = GPIO_Pin_9; break;
		case U_PIN_10: GPIO_Pin = GPIO_Pin_10; break;
		case U_PIN_11: GPIO_Pin = GPIO_Pin_11; break;
		case U_PIN_12: GPIO_Pin = GPIO_Pin_12; break;
		case U_PIN_13: GPIO_Pin = GPIO_Pin_13; break;
		case U_PIN_14: GPIO_Pin = GPIO_Pin_14; break;
		case U_PIN_15: GPIO_Pin = GPIO_Pin_15; break;

		default:	break;
	}
	if( 1==val)
		GPIO_SetBits(GPIOx, GPIO_Pin);						 
	else if( 0== val)
		GPIO_ResetBits( GPIOx, GPIO_Pin);
}


/*************************************************************************************************
*****
***** 函数名：void  STM32_GPIO_ConfgPin()
*****
***** 入口参数：gpio--GPIO号, pin--引脚号(0~15), mode--输入/输出, speed--输出速度
*****
***** 功能描述：对GPIO引脚进行设置
***** 
*****							
***** 调用： RCC_APB2PeriphClockCmd(), GPIO_Init()		   
*****
***** 返回值：无
*****
***** 作者：Sandy
****				
****  版本：v2.0
****
**************************************************************************************************/
void STM32_GPIO_ConfgPin(uint8_t gpio, uint8_t pin, uint8_t mode, uint8_t speed)
{
	GPIO_TypeDef *in_gpio = GPIOA;
	uint16_t in_pin = GPIO_Pin_0;
	GPIOMode_TypeDef in_mode = GPIO_Mode_Out_PP;
	GPIOSpeed_TypeDef in_speed = GPIO_Speed_10MHz;
	GPIO_InitTypeDef GPIO_InitStructure;  

	//1. Enable GPIO clock
	switch (gpio)
	{
		case U_GPIO_A:
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
			in_gpio = GPIOA;
			break;
		case U_GPIO_B:
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
			in_gpio = GPIOB;
		break;
		case U_GPIO_C:
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
			in_gpio = GPIOC;
		break;
		case U_GPIO_D:
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
			in_gpio = GPIOD;
		break;
		case U_GPIO_E:
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
			in_gpio = GPIOE;
		break;
		case U_GPIO_F:
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);
			in_gpio = GPIOF;
		break;
		case U_GPIO_G:
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG, ENABLE);
			in_gpio = GPIOG;
		break;
		default:
			assert_param(0);
		break;
	}
	
	//2. Select pin
	switch (pin)
	{
		case U_PIN_0: in_pin = GPIO_Pin_0; break;
		case U_PIN_1: in_pin = GPIO_Pin_1; break;
		case U_PIN_2: in_pin = GPIO_Pin_2; break;
		case U_PIN_3: in_pin = GPIO_Pin_3; break;
		case U_PIN_4: in_pin = GPIO_Pin_4; break;
		case U_PIN_5: in_pin = GPIO_Pin_5; break;
		case U_PIN_6: in_pin = GPIO_Pin_6; break;
		case U_PIN_7: in_pin = GPIO_Pin_7; break;
		case U_PIN_8: in_pin = GPIO_Pin_8; break;
		case U_PIN_9: in_pin = GPIO_Pin_9; break;
		case U_PIN_10: in_pin = GPIO_Pin_10; break;
		case U_PIN_11: in_pin = GPIO_Pin_11; break;
		case U_PIN_12: in_pin = GPIO_Pin_12; break;
		case U_PIN_13: in_pin = GPIO_Pin_13; break;
		case U_PIN_14: in_pin = GPIO_Pin_14; break;
		case U_PIN_15: in_pin = GPIO_Pin_15; break;

		default:		break;
	}

	//3. Select mode
	switch (mode)
	{
		case U_MODE_AIN: 		in_mode = GPIO_Mode_AIN; 			break;
		case U_MODE_FLOATING: 	in_mode = GPIO_Mode_IN_FLOATING; 	break;
		case U_MODE_IPD: 		in_mode = GPIO_Mode_IPD; 			break;
		case U_MODE_IPU: 		in_mode = GPIO_Mode_IPU; 			break;
		case U_MODE_OUT_OD: 	in_mode = GPIO_Mode_Out_OD; 		break;
		case U_MODE_OUT_PP:		in_mode = GPIO_Mode_Out_PP; 		break;
		case U_MODE_AF_OD: 		in_mode = GPIO_Mode_AF_OD; 			break;
		case U_MODE_AF_PP: 		in_mode = GPIO_Mode_AF_PP; 			break;
		default:
			//assert(0);
		break;
	}

	//3. Set speed
	switch (speed)
	{
		case U_SPEED_10M: 		in_speed = GPIO_Speed_10MHz; 		break;
		case U_SPEED_2M: 		in_speed = GPIO_Speed_2MHz; 		break;
		case U_SPEED_50M: 		in_speed = GPIO_Speed_50MHz; 		break;
		default: break;
	}
	
	GPIO_InitStructure.GPIO_Pin   = in_pin;      
  	GPIO_InitStructure.GPIO_Mode  = in_mode;		
  	GPIO_InitStructure.GPIO_Speed = in_speed;
	
	GPIO_Init(in_gpio, &GPIO_InitStructure); 
}
