#include "steer.h"



void RobortSteerThreadServer(uint32 argAddr)
{
    S_Internal_Interface_Tag *pInternMsg; 
    uint8 SteerNo = 0;
    uint8 TurnAngle = 0;
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

        if (!(pInternMsg->Entry_Type==PARSER_STEER_DAT_TAG))
            continue;

        SteerNo = pInternMsg->data.p2s_tag.steer_no;
        TurnAngle = pInternMsg->data.p2s_tag.angle;
        
        Robort_Steer_Control(SteerNo, TurnAngle);       
        
    }
}


/*************************************************************************************************
*****
***** 函数名：void  Robort_Steer_Controller()
*****
***** 入口参数：steerNum  --- ROBORT_STEER1 or ROBORT_STEER2
*****			turnAngle --- 0-180
*****
***** 功能描述：舵机的角度控制
***** 
*****							
***** 调用： 无		   
*****
***** 返回值：无
*****
***** 作者：Sandy
****				
****  版本：v2.0
**************************************************************************************************/
static char Robort_Steer_Control(ROBORT_STEER_TAG steerNo,  short turnAngle)
{

	if (turnAngle<0 || turnAngle>180)
		return 0;

	if (steerNo == ROBORT_STEER1)
	{
		SteerOneControl(turnAngle); //1号舵机控制
	}
	else if( steerNo== ROBORT_STEER2)
	{
		SteerTwoControl(turnAngle); //2号舵机控制
	}
	else if( steerNo== ROBORT_STEER3)
	{
		SteerThreeControl(turnAngle); //3号舵机控制
	}
	else if( steerNo== ROBORT_STEER4)
	{
		SteerFourControl(turnAngle); //4号舵机控制
	}
	else if( steerNo== ROBORT_STEER5)
	{
		SteerFiveControl(turnAngle); //5号舵机控制
	}
	else if( steerNo== ROBORT_STEER6)
	{
		SteerSixControl(turnAngle); //6号舵机控制
	}
	else if( steerNo== ROBORT_STEER7)
	{
		SteerSevenControl(turnAngle); //7号舵机控制
	}
	else if( steerNo== ROBORT_STEER8)
	{
		SteerEightControl(turnAngle); //8号舵机控制
	}
	


	return 0;
}



//根据角度值计算输出占空比
static unsigned char calDutyCycle(short turnAngle)
{
	uint8 out_dutycycle = 0;
	out_dutycycle = turnAngle * ((ANGLE180_DUTY_CYCLE-ANGLE0_DUTY_CYCLE) * 1.0 / 
                    (ANGLE_MAX_ANGLE - ANGLE_MIN_ANGLE)) + ANGLE0_DUTY_CYCLE;
	
    return out_dutycycle;
}



//1号舵机角度控制
static void  SteerOneControl(short turnAngle)
{
	uint8 dutyCycle= 0;
	
	dutyCycle = calDutyCycle(turnAngle); //计算转动后对应的脉冲cnt	

	STM32_TIMER3_PWMControl(TIM3_PWM1, CLOUD_STAGE_STEER_PERIOD, dutyCycle); //20ms/1.5ms 
}


//2号舵机角度控制
static void  SteerTwoControl(short turnAngle)
{
	unsigned char dutyCycle = 0;

	dutyCycle= calDutyCycle(turnAngle); //;//计算转动后对应的脉冲cnt	

	STM32_TIMER3_PWMControl(TIM3_PWM2, CLOUD_STAGE_STEER_PERIOD, dutyCycle); 
}

//3号舵机角度控制
static void  SteerThreeControl(short turnAngle)
{
	unsigned char dutyCycle = 0;

	dutyCycle= calDutyCycle(turnAngle); //;//计算转动后对应的脉冲cnt	

	STM32_TIMER3_PWMControl(TIM3_PWM3, CLOUD_STAGE_STEER_PERIOD, dutyCycle); 
}
//4号舵机角度控制
static void  SteerFourControl(short turnAngle)
{
	unsigned char dutyCycle = 0;

	dutyCycle= calDutyCycle(turnAngle); //;//计算转动后对应的脉冲cnt	

	STM32_TIMER3_PWMControl(TIM3_PWM4, CLOUD_STAGE_STEER_PERIOD, dutyCycle); 
}
//5号舵机角度控制
static void  SteerFiveControl(short turnAngle)
{
	unsigned char dutyCycle = 0;

	dutyCycle= calDutyCycle(turnAngle); //;//计算转动后对应的脉冲cnt	

	STM32_TIMER3_PWMControl(TIM4_PWM1, CLOUD_STAGE_STEER_PERIOD, dutyCycle); 
}
//6号舵机角度控制
static void  SteerSixControl(short turnAngle)
{
	unsigned char dutyCycle = 0;

	dutyCycle= calDutyCycle(turnAngle); //;//计算转动后对应的脉冲cnt	

	STM32_TIMER3_PWMControl(TIM4_PWM2, CLOUD_STAGE_STEER_PERIOD, dutyCycle); 
}
//7号舵机角度控制
static void  SteerSevenControl(short turnAngle)
{
	unsigned char dutyCycle = 0;

	dutyCycle= calDutyCycle(turnAngle); //;//计算转动后对应的脉冲cnt	

	STM32_TIMER3_PWMControl(TIM4_PWM3, CLOUD_STAGE_STEER_PERIOD, dutyCycle); 
}
//8号舵机角度控制
static void  SteerEightControl(short turnAngle)
{
	unsigned char dutyCycle = 0;

	dutyCycle= calDutyCycle(turnAngle); //;//计算转动后对应的脉冲cnt	

	STM32_TIMER3_PWMControl(TIM4_PWM4, CLOUD_STAGE_STEER_PERIOD, dutyCycle); 
}



/*************************************************************************************************
*****
***** 函数名：void  Robort_Steer_Init()
*****
***** 入口参数：无
*****
***** 功能描述：舵机初始化控制
***** 
*****							
***** 调用： 无		   
*****
***** 返回值：无
*****
***** 其它说明：舵机使用了TIME3的PWM输出，Steer0-PA6, Steer1-PA7, Steer2-PB0, Steer3-PB1
***** 作者：Sandy
****				
****  版本：v2.0
**************************************************************************************************/
void Robort_Steer_Init(void)
{
	TIM3_PWM_Init(20000,71);
	STM32_TIMER3_PWMControl(TIM3_PWM1, CLOUD_STAGE_STEER_PERIOD, 75); //steer1-PA6,Period/H-Voltage 20ms/1.5ms 
	STM32_TIMER3_PWMControl(TIM3_PWM2, CLOUD_STAGE_STEER_PERIOD, 75); //steer2-PA7:Period/H-Voltage 20ms/1.5ms 
	STM32_TIMER3_PWMControl(TIM3_PWM3, CLOUD_STAGE_STEER_PERIOD, 75); //steer3-PB0:Period/H-Voltage 20ms/1.5ms 
	STM32_TIMER3_PWMControl(TIM3_PWM4, CLOUD_STAGE_STEER_PERIOD, 75); //steer4-PB1:Period/H-Voltage 20ms/1.5ms 
	TIM4_PWM_Init(20000,71);
	STM32_TIMER4_PWMControl(TIM4_PWM1, CLOUD_STAGE_STEER_PERIOD, 75); //steer1-PA6: 20ms/1.5ms 
     
}



