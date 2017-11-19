#ifndef _MAIN_H
#define _MAIN_H
#include "common.h"
#include "beep.h"
#include "light.h"
#include "motor.h"
#include "parser.h"
#include "steer.h"
#include "gps.h"
#include "receiver.h"
#include "sender.h"

static TError AllocateThreadStack(TByte ThreadId, void **pStack);
static void BindThreadFunction(void);
static TError InitUserThreadInfo(TByte ThreadId);
extern void RobortBeepThreadServer(uint32 argAddr);


#endif

