#ifndef _APP_BEEP_H
#define _APP_BEEP_H

#include "common.h"

typedef enum
{
	ROBORT_BEEP_ON = 0,
	ROBORT_BEEP_OFF
}ROBORT_BEEP_TAG;

typedef enum
{
	BEEP_0 = 0,
	BEEP_1,
}ROBORT_BEEP_NO;

void RobortBeepThreadServer(uint32 argAddr);
void Robort_Beep_Init(void);

#endif
