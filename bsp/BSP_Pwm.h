#ifndef _BSP_PWM_H
#define _BSP_PWM_H

#include "BSP_Common.h"

typedef enum
{
  TIM2_PWM1 = 1,
  TIM2_PWM2,
  TIM2_PWM3,
  TIM2_PWM4,
  TIM3_PWM1,
  TIM3_PWM2,
  TIM3_PWM3,
  TIM3_PWM4,
  TIM4_PWM1,
  TIM4_PWM2,
  TIM4_PWM3,
  TIM4_PWM4,
  TIM5_PWM1,
  TIM5_PWM2,
  TIM5_PWM3,
  TIM5_PWM4
}TIM_PWM_TAG;


void TIM2_PWM_Init(u16 arr,u16 psc);
void STM32_TIMER2_PWMControl(u8 pwmNo, u32 period, u16 dutyCycle);
void TIM4_PWM_Init(u16 arr,u16 psc);
void STM32_TIMER4_PWMControl(u8 pwmNo, u32 period, u16 dutyCycle);
void TIM3_PWM_Init(u16 arr,u16 psc);
void STM32_TIMER3_PWMControl(u8 pwmNo, u32 period, u16 dutyCycle);

#endif

























/*********************************************END OF FILE**********************/
