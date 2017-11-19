#ifndef _APP_STEER_H
#define _APP_STEER_H

#include "common.h"
#include "../bsp/BSP_Pwm.h"

#define ANGLE0_DUTY_CYCLE 35   //Duty cycle which steer stays angle-0
#define ANGLE180_DUTY_CYCLE 125 
 
#define ANGLE_MAX_ANGLE 180
#define ANGLE_MIN_ANGLE 0 


#define ANGLE_DEFAULT 90

#define CLOUD_STAGE_STEER_PERIOD 20000  //20 ms
 
typedef enum
{
    ROBORT_STEER1 = 0,
	ROBORT_STEER2,
	ROBORT_STEER3,
	ROBORT_STEER4,
	ROBORT_STEER5,
	ROBORT_STEER6,
	ROBORT_STEER7,
	ROBORT_STEER8
}ROBORT_STEER_TAG;


void Robort_Steer_Init(void);

static void  SteerOneControl(short turnAngle);
static void  SteerTwoControl(short turnAngle);
static void  SteerThreeControl(short turnAngle);
static void  SteerFourControl(short turnAngle);
static void  SteerFiveControl(short turnAngle);
static void  SteerSixControl(short turnAngle);
static void  SteerSevenControl(short turnAngle);
static void  SteerEightControl(short turnAngle);
static char Robort_Steer_Control(ROBORT_STEER_TAG steerNo,  short turnAngle);

void RobortSteerThreadServer(uint32 argAddr);
void Robort_Steer_Init(void);

#endif





