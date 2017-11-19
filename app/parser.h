#ifndef _APP_PARSER_H
#define _APP_PARSER_H

#include "common.h"
#include "motor.h"
#include "steer.h"
#include "beep.h"
#include "light.h"





void Robort_Parser_Init(void);
void RobortParserThreadServer(uint32 argAddr);

static TState BroadcastInternalMessage(S_Internal_Interface_Tag **pInternMsg);

static unsigned short Assemble_Message_Length(App_Robort_Dat_Tag *pdata);
static uint8 Robort_Adress_Veritfy(App_Robort_Dat_Tag *pdata);
static uint8 Protocol_Parser(App_Robort_Dat_Tag *pExternMsg, S_Internal_Interface_Tag **pInternMsg);
static uint8 Process_Robort_Beep_Cmd(App_Robort_Dat_Tag *pExternMsg, S_Internal_Interface_Tag **pInternMsg);
static uint8 Process_Robort_Light_Cmd(App_Robort_Dat_Tag *pExternMsg, S_Internal_Interface_Tag **pInternMsg);
#endif

