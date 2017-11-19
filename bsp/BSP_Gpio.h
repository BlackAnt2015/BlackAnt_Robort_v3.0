#ifndef _BSP_GPIO_H
#define _BSP_GPIO_H

#include "BSP_Common.h"

//User GPIO No.
typedef enum
{
	U_GPIO_A = 0,
	U_GPIO_B,
	U_GPIO_C,
	U_GPIO_D,
	U_GPIO_E,
	U_GPIO_F,
	U_GPIO_G,
}GPIO_IDX_TAG;

//User PIN No.
typedef enum
{
	U_PIN_0 = 0,
	U_PIN_1,
	U_PIN_2,
	U_PIN_3,
	U_PIN_4,
	U_PIN_5,
	U_PIN_6,
	U_PIN_7,
	U_PIN_8,
	U_PIN_9,
	U_PIN_10,
	U_PIN_11,
	U_PIN_12,
	U_PIN_13,
	U_PIN_14,
	U_PIN_15,
}GPIO_PIN_TAG;

//User MODE
typedef enum
{
	U_MODE_AIN=0,
	U_MODE_FLOATING,
	U_MODE_IPD,
	U_MODE_IPU,
	U_MODE_OUT_OD,
	U_MODE_OUT_PP,
	U_MODE_AF_OD,
	U_MODE_AF_PP,
}GPIO_MODE_TAG;

//User SPEED
typedef enum
{
	U_SPEED_10M = 0,
	U_SPEED_2M,
	U_SPEED_50M,
}GPIO_SPEED_TAG;

void STM32_GPIO_SetPin( uint8_t gpio, uint8_t pin, uint8_t val);
void STM32_GPIO_ConfgPin(uint8_t gpio, uint8_t pin, uint8_t mode, uint8_t speed);


#endif
