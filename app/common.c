#include "common.h"

TThread 		ThreadTable[BA_THREAD_END_ID] 		= {0};
TThreadEntry 	pThreadEntryTable[BA_THREAD_END_ID] = {NULL};
TBase32 		*ThreadStackTable[BA_THREAD_END_ID] = {NULL};


BA_USER_THREAD_INFO BAUserThreadInfoTable[] = 
{//  ID		ENTRY	   NAME			STACK		STACK_LENGTH		PRIORITY   SLICE    	ARG
    {NULL, 	NULL, 	"ThreadBeep", 	NULL, 	THREAD_BEEP_STACK_BYTES, 	5, 		20, 	(TArgument)NULL},	
    {NULL, 	NULL, 	"ThreadParser",	NULL, 	THREAD_PARSER_STACK_BYTES, 	5, 		20, 	(TArgument)NULL},	
    {NULL, 	NULL, 	"ThreadLight",	NULL, 	THREAD_LIGHT_STACK_BYTES, 	5, 		10, 	(TArgument)NULL},	
    {NULL, 	NULL, 	"ThreadMotor",	NULL, 	THREAD_MOTOR_STACK_BYTES, 	5, 		50, 	(TArgument)NULL},	
    {NULL, 	NULL, 	"ThreadSteer",	NULL, 	THREAD_STEER_STACK_BYTES, 	5, 		30, 	(TArgument)NULL},	
    {NULL, 	NULL, 	"ThreadGps",	NULL, 	THREAD_GPS_STACK_BYTES, 	5, 		50, 	(TArgument)NULL},
	{NULL, 	NULL, 	"ThreadRec",	NULL, 	THREAD_RECEIVER_STACK_BYTES,5, 		50, 	(TArgument)NULL},
	{NULL, 	NULL, 	"ThreadSend",	NULL, 	THREAD_SENDER_STACK_BYTES, 	5, 		50, 	(TArgument)NULL},
};

void* InternMsgInPool[MQ_POOL_LEN];
TMsgQueue InternMsgInQueue; 

void* InternMsgOutPool[MQ_POOL_LEN];
TMsgQueue InternMsgOutQueue; 

void* ExternMsgInPool[MQ_POOL_LEN]; 
TMsgQueue ExternInMsgQueue; 

void* ExternMsgOutPool[MQ_POOL_LEN]; 
TMsgQueue ExternOutMsgQueue; 



