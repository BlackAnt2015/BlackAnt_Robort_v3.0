#ifndef _APP_LIGHT_H
#define _APP_LIGHT_H

#include "common.h"


typedef enum
{
	LIGHT_0 = 0,
	LIGHT_1,
}E_Light_No;

void RobortLightThreadServer(uint32 argAddr);
void Robort_Light_Init(void);

#endif
