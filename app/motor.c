#include "motor.h"

static uint8 gRobortSpeed_L = 0; //代表robort左侧的车速
static uint8 gRobortSpeed_R = 0; //代表robort右侧的车速
static uint8 gFlagSpeedRset_L = 0; //代表robort左侧车速是否被远程终端修改过
static uint8 gFlagSpeedRset_R = 0; //代表robort右侧车速是否被远程终端修改过
static uint8 gRobortSysSpeed = 30;//代表robort系统初始速度


void RobortMotorThreadServer(uint32 argAddr)
{
    S_Internal_Interface_Tag p2m = {0};
    S_Internal_Interface_Tag *pInternMsg = &p2m; 
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

        if (!(pInternMsg->Entry_Type==PARSER_MOTOR_DAT_TAG))
            continue;

        if (pInternMsg->data.p2m_tag.type == MOTOR_DIR_TYPE)
        {
            ROBORT_DIRECTION_TAG dir = (ROBORT_DIRECTION_TAG)pInternMsg->data.p2m_tag.direction;
            Robort_Direction_Control(dir, 0);
        } 
        else if (pInternMsg->data.p2m_tag.type == MOTOR_SPEED_TYPE)
        {
            ROBORT_SPEED_TAG speed_dir = (ROBORT_SPEED_TAG)pInternMsg->data.p2m_tag.speed_dir;
            uint8 speed = pInternMsg->data.p2m_tag.speed;
            Robort_Speed_Control(speed_dir, speed);
        }


    }
}



/*************************************************************************************************
*****
***** 函数名：void  Robort_Direction_Control()
*****
***** 入口参数：(1)dir -- robort方向
*****           (ROBORT_STOP, ROBORT_FORWARD, ROBORT_BACK, ROBORT_LEFT, ROBORT_RIGHT)
*****
*****           (2)speed -- 速度设置, 暂未使用
*****
***** 功能描述：调整robort的运行方向
***** 
*****							
***** 调用： (1)Set_Motor_Direction() -- 设置motor的方向控制信号
*****        (2)Set_Motor_Speed()     -- 设置motor的速度
*****
***** 返回值：无
*****
***** 作者：Sandy
****				
****  版本：v2.0
**************************************************************************************************/
static void Robort_Direction_Control(ROBORT_DIRECTION_TAG dir, uint8 speed)
{
    uint8 sysSpeed_L = (gFlagSpeedRset_L==0) ? gRobortSysSpeed : gRobortSpeed_L;
    uint8 sysSpeed_R = (gFlagSpeedRset_R==0) ? gRobortSysSpeed : gRobortSpeed_R;
    
    switch (dir)
    {
     case ROBORT_STOP:
     {
           Set_Motor_Direction( MOTOR_LT,  DIR_STOPPED);   //left-top motor stop
           Set_Motor_Speed( MOTOR_LT,  0);       //speed=0 so stopped
           
           Set_Motor_Direction( MOTOR_RT,  DIR_STOPPED);   //right-top motor stop
           Set_Motor_Speed( MOTOR_RT,  0);
                      
           Set_Motor_Direction( MOTOR_LB,  DIR_STOPPED);   //left-bottom motor stop
           Set_Motor_Speed( MOTOR_LB,  0);
           
           Set_Motor_Direction( MOTOR_RT,  DIR_STOPPED);   //right-bottom motor stop
           Set_Motor_Speed( MOTOR_RB,  0);
            
           break;
     }
     case ROBORT_FORWARD:
     {
           Set_Motor_Direction( MOTOR_LT,  DIR_POSITIVE);   //left-top motor stop
           Set_Motor_Speed( MOTOR_LT,  sysSpeed_L);
           
           Set_Motor_Direction( MOTOR_RT,  DIR_POSITIVE);   //right-top motor stop
           Set_Motor_Speed( MOTOR_RT,  sysSpeed_R);
                      
           Set_Motor_Direction( MOTOR_LB,  DIR_POSITIVE);   //left-bottom motor stop
           Set_Motor_Speed( MOTOR_LB,  sysSpeed_L);
           
           Set_Motor_Direction( MOTOR_RB,  DIR_POSITIVE);   //right-bottom motor stop
           Set_Motor_Speed( MOTOR_RB,  sysSpeed_R);
            
        break;
     }
     case ROBORT_BACK:
     {
           Set_Motor_Direction( MOTOR_LT,  DIR_NEGTIVE);   //left-top motor stop
           Set_Motor_Speed( MOTOR_LT,  sysSpeed_L);
           
           Set_Motor_Direction( MOTOR_RT,  DIR_NEGTIVE);   //right-top motor stop
           Set_Motor_Speed( MOTOR_RT,  sysSpeed_R);
                      
           Set_Motor_Direction( MOTOR_LB,  DIR_NEGTIVE);   //left-bottom motor stop
           Set_Motor_Speed( MOTOR_LB,  sysSpeed_L);
           
           Set_Motor_Direction( MOTOR_RB,  DIR_NEGTIVE);   //right-bottom motor stop
           Set_Motor_Speed( MOTOR_RB,  sysSpeed_R);
            
        break;
     }
     case ROBORT_LEFT:
     {
           Set_Motor_Direction( MOTOR_LT,  DIR_NEGTIVE);   //left-top motor stop
           Set_Motor_Speed( MOTOR_LT,  sysSpeed_L);
           
           Set_Motor_Direction( MOTOR_RT,  DIR_POSITIVE);   //right-top motor stop
           Set_Motor_Speed( MOTOR_RT,  sysSpeed_R);
                      
           Set_Motor_Direction( MOTOR_LB,  DIR_NEGTIVE);   //left-bottom motor stop
           Set_Motor_Speed( MOTOR_LB,  sysSpeed_L);
           
           Set_Motor_Direction( MOTOR_RB,  DIR_POSITIVE);   //right-bottom motor stop
           Set_Motor_Speed( MOTOR_RB,  sysSpeed_R);
            
        break;
     } 
     case ROBORT_RIGHT:
     {
           Set_Motor_Direction( MOTOR_LT,  DIR_POSITIVE);   //left-top motor stop
           Set_Motor_Speed( MOTOR_LT,  sysSpeed_L);
           
           Set_Motor_Direction( MOTOR_RT,  DIR_NEGTIVE);   //right-top motor stop
           Set_Motor_Speed( MOTOR_RT,  sysSpeed_R);
                      
           Set_Motor_Direction( MOTOR_LB,  DIR_POSITIVE);   //left-bottom motor stop
           Set_Motor_Speed( MOTOR_LB,  sysSpeed_L);
           
           Set_Motor_Direction( MOTOR_RB,  DIR_NEGTIVE);   //right-bottom motor stop
           Set_Motor_Speed( MOTOR_RB,  sysSpeed_R);
            
        break;
     } 
     
     default: break;
    }  
}

/*************************************************************************************************
*****
***** 函数名：void  Set_Motor_Direction()
*****
***** 入口参数：(1)motorNo -- 电机号
*****           0号-MOTOR_LT(左上), 1号-MOTOR_RT(右上), 2号-MOTOR_LB(左下), 3号-MOTOR_RB(右下)
*****
*****           (2)dir -- 方向
*****
***** 功能描述：设置robort的运行方向信号
*****            GPIOC_PIN9 - Motor1_dir, GPIOC_PIN8 - Motor2_dir
*****            GPIOC_PIN7 - Motor2_dir, GPIOC_PIN6 - Motor3_dir
***** 
*****							
***** 调用： STM32_GPIO_SetPin()		   
*****
***** 返回值：无
*****
***** 作者：Sandy
****				
****  版本：v2.0
**************************************************************************************************/
static void Set_Motor_Direction(MOTOR_TAG motorNo, DIRECTION_TAG  dir)
{
    static MOTOR_TAG preMotorNo = MOTOR_NONE;
    static DIRECTION_TAG preDir = DIR_STOPPED;
    
    if (preMotorNo==motorNo && preDir==dir)
    {
        return ;
    }
    
    preMotorNo = motorNo;
    preDir = dir;
    
    switch (motorNo)
    {
     case MOTOR_LT:
     {
        if (dir == DIR_POSITIVE)
        {
             STM32_GPIO_SetPin(U_GPIO_C, U_PIN_9, 1);   // DIR1=HIGH
        }
        else
        {
             STM32_GPIO_SetPin(U_GPIO_C, U_PIN_9, 0);  // DIR1=LOW
        }
     }
     break;
     
     case MOTOR_RT:
        if (dir == DIR_POSITIVE)
        {
             STM32_GPIO_SetPin(U_GPIO_C, U_PIN_8, 1);   // DIR2=HIGH
        }
        else
        {
             STM32_GPIO_SetPin(U_GPIO_C, U_PIN_8, 0);    // DIR2=LOW
        }
     break;
     case MOTOR_LB:
        if (dir == DIR_POSITIVE)
        {
             STM32_GPIO_SetPin(U_GPIO_C, U_PIN_7, 1);   // DIR3=HIGH
        }
        else
        {
             STM32_GPIO_SetPin(U_GPIO_C, U_PIN_7, 0);    // DIR3=LOW
        }
     break;
     case MOTOR_RB:
        if (dir == DIR_POSITIVE)
        {
             STM32_GPIO_SetPin(U_GPIO_C, U_PIN_6, 1);   // DIR4=HIGH
        }
        else
        {
             STM32_GPIO_SetPin(U_GPIO_C, U_PIN_6, 0);   // DIR4=LOW
        }
     break;  

     default: break;
    }
    
    return ;
}

/*************************************************************************************************
*****
***** 函数名：void  Set_Motor_Speed()
*****
***** 入口参数：(1)motorNo -- 电机号
*****           0号-MOTOR_LT(左上), 1号-MOTOR_RT(右上), 2号-MOTOR_LB(左下), 3号-MOTOR_RB(右下)
*****
*****           (2)speed -- 速度(0~100)，跟据speed值计算出PWM占空比并设置到对应PWM输出上
*****
***** 功能描述：设置robort的运行方向信号
*****            GPIOA_PIN0 - Motor1_PWM, GPIOA_PIN1 - Motor2_PWM
*****            GPIOA_PIN2 - Motor2_PWM, GPIOA_PIN3 - Motor3_PWM
***** 
*****							
***** 调用： CalPWMDutyCycle() -- 根据speed值计算占空比	
*****        STM32_TIMER2_PWMControl() -- 调整PWM输出
*****
***** 返回值：无
*****
***** 作者：Sandy
****				
****  版本：v2.0
**************************************************************************************************/
//speed: 0~100
static void Set_Motor_Speed(MOTOR_TAG motorNo, uint8 speed)
{
    static MOTOR_TAG preMotorNo;
    static uint8    preSpeed;
    uint16 ratio = 0;
    
    if (preMotorNo==motorNo && preSpeed==speed)
    {
        return ;
    }
    
    preMotorNo = motorNo;
    preSpeed = speed;
    
    ratio =  CalPWMDutyCycle(speed);
    
    switch (motorNo)
    {
     case MOTOR_LT: //TIME2_PWM1
        STM32_TIMER2_PWMControl(TIM2_PWM1, MOTOR_PERIOD_DEFAULT, ratio);
     break;
     case MOTOR_RT://TIME2_PWM2
        STM32_TIMER2_PWMControl(TIM2_PWM2, MOTOR_PERIOD_DEFAULT, ratio);
     break;
     case MOTOR_LB://TIME2_PWM3
        STM32_TIMER2_PWMControl(TIM2_PWM3, MOTOR_PERIOD_DEFAULT, ratio);
     break;
     case MOTOR_RB://TIME2_PWM4
        STM32_TIMER2_PWMControl(TIM2_PWM4, MOTOR_PERIOD_DEFAULT, ratio);
     break;  

     default: break;
    }
    
    return ;
}


/*************************************************************************************************
*****
***** 函数名：void  CalPWMDutyCycle()
*****
***** 入口参数：speed -- 速度(0~100)，跟据speed值计算出PWM占空比并设置到对应PWM输出上
*****
***** 功能描述：设置robort的运行方向信号
*****            GPIOA_PIN0 - Motor1_PWM, GPIOA_PIN1 - Motor2_PWM
*****            GPIOA_PIN2 - Motor2_PWM, GPIOA_PIN3 - Motor3_PWM
***** 
*****							
***** 调用： CalPWMDutyCycle() -- 根据speed值计算占空比	
*****        STM32_TIMER2_PWMControl() -- 调整PWM输出
*****
***** 返回值：DutyCycle -- 占空比(0~1000表示千分之0~千分之1000), 
*****
***** 作者：Sandy
****				
****  版本：v2.0
**************************************************************************************************/
static uint16  CalPWMDutyCycle(unsigned char speed)
{
	uint16 DutyCycle = 0;

	DutyCycle = (uint16) (speed*1.0 / 100 * 1000);
	
	return DutyCycle;
}

/*************************************************************************************************
*****
***** 函数名：void  Robort_Speed_Control()
*****
***** 入口参数：(1)dir -- robort方向
*****           (ROBORT_STOP, ROBORT_FORWARD, ROBORT_BACK, ROBORT_LEFT, ROBORT_RIGHT)
*****
*****           (2)speed -- 速度设置, 给robort的dir参数设置新的运行速度
*****
***** 功能描述：调整robort的运行方向
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
void  Robort_Speed_Control(ROBORT_SPEED_TAG robort_dir, unsigned char robort_speed)
{
    
    
    switch (robort_dir)
    {
     case ROBORT_SPEED_L:
     {
		  gFlagSpeedRset_L 	= 1;   //标志为1，代表客户端设定了速度，下次方向调整时使用gRobortSpeed_L/gRobortSpeed_R
          gRobortSpeed_L 	= robort_speed;
          break;
     }
     case ROBORT_SPEED_R:
     {
		  gFlagSpeedRset_R	= 1;
          gRobortSpeed_R 	= robort_speed;
          break;
     }
  
     default: break;
    }  
    
}
/*************************************************************************************************
*****
***** 函数名：void  Robort_Motor_Init()
*****
***** 入口参数：无
*****
***** 功能描述：motor接口初始化
*****           GPIOC_PIN9 - Motor1_dir, GPIOC_PIN8 - Motor2_dir
*****           GPIOC_PIN7 - Motor2_dir, GPIOC_PIN6 - Motor3_dir	
*****           GPIOA_PIN0 - Motor1_PWM, GPIOA_PIN1 - Motor2_PWM
*****           GPIOA_PIN2 - Motor2_PWM, GPIOA_PIN3 - Motor3_PWM 
*****
***** 调用： 无
*****
***** 返回值：无
*****
***** 作者：Sandy
****				
****  版本：v2.0
**************************************************************************************************/
void Robort_Motor_Init(void)
{
 	STM32_GPIO_ConfgPin(U_GPIO_C, U_PIN_6, U_MODE_OUT_PP, U_SPEED_50M);
	STM32_GPIO_ConfgPin(U_GPIO_C, U_PIN_7, U_MODE_OUT_PP, U_SPEED_50M);
	STM32_GPIO_ConfgPin(U_GPIO_C, U_PIN_8, U_MODE_OUT_PP, U_SPEED_50M);
	STM32_GPIO_ConfgPin(U_GPIO_C, U_PIN_9, U_MODE_OUT_PP, U_SPEED_50M);

	STM32_GPIO_SetPin(U_GPIO_C, U_PIN_6, 1);
	STM32_GPIO_SetPin(U_GPIO_C, U_PIN_7, 1);
	STM32_GPIO_SetPin(U_GPIO_C, U_PIN_8, 1);
	STM32_GPIO_SetPin(U_GPIO_C, U_PIN_9, 1);

	TIM2_PWM_Init(MOTOR_PERIOD_DEFAULT,71);
	STM32_TIMER2_PWMControl(TIM2_PWM1, MOTOR_PERIOD_DEFAULT, 0); // PWM out low 
	STM32_TIMER2_PWMControl(TIM2_PWM2, MOTOR_PERIOD_DEFAULT, 0); // PWM out low 
	STM32_TIMER2_PWMControl(TIM2_PWM3, MOTOR_PERIOD_DEFAULT, 0); // PWM out low 
	STM32_TIMER2_PWMControl(TIM2_PWM4, MOTOR_PERIOD_DEFAULT, 0); // PWM out low  
}

