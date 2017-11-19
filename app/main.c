#include "example.h"
#include "main.h"

#if (EVB_EXAMPLE == CH14_BLACK_ANT_ROBORT)



/* 用户应用入口函数 */
static void AppSetupEntry(void)
{
    TError error;
    TState state;
	uint8  nThreadId = 0;

    BindThreadFunction();

    for (nThreadId=0; nThreadId<BA_THREAD_END_ID; nThreadId++)
    {
        /* 初始化线程信息 */
        state = InitUserThreadInfo(nThreadId);
        
        /* 初始化线程 */
        state = TclCreateThread(BAUserThreadInfoTable[nThreadId].Thread,
                                BAUserThreadInfoTable[nThreadId].pThreadName,
                                BAUserThreadInfoTable[nThreadId].pThreadEntry,
                                BAUserThreadInfoTable[nThreadId].Argument,
                                BAUserThreadInfoTable[nThreadId].pThreadStack,
                                BAUserThreadInfoTable[nThreadId].StackLength,
                                BAUserThreadInfoTable[nThreadId].Priority,
                                BAUserThreadInfoTable[nThreadId].Slice,
                                &error);
                        
         TCLM_ASSERT((state == eSuccess), "");
         TCLM_ASSERT((error == TCLE_THREAD_NONE), "");
    } 
	
	for (nThreadId=0; nThreadId<BA_THREAD_END_ID; nThreadId++)
    {
		/* 激活线程 */
		state =TclActivateThread(BAUserThreadInfoTable[nThreadId].Thread, &error);
		TCLM_ASSERT((state == eSuccess), "");
		TCLM_ASSERT((error == TCLE_THREAD_NONE), "");
	}

    TclTrace("example start!\r\n");
}

extern void BASystemSetupEntry(void);
extern void EvbSetupEntry(void);
extern void EvbTraceEntry(const char* str);

/* 处理器BOOT之后会调用main函数，必须提供 */
int main(void)
{
    /* 注册各个内核函数,启动内核 */
    TclStartKernel(&AppSetupEntry,
                   &OsCpuSetupEntry,
                   &BASystemSetupEntry,
                   &EvbTraceEntry);
    return 1;
}

TError AllocateThreadStack(TByte ThreadId, void **ppStack)
{
   TError ret = eFalse; 
   
   *ppStack = (void *)malloc(BAUserThreadInfoTable[ThreadId].StackLength);
   if (*ppStack == NULL)
   {
        ret = eFalse;
        return ret;
   }

   return eTrue;
}

static void BindThreadFunction(void)
{
    BAUserThreadInfoTable[BA_THREAD_BEEP_ID].pThreadEntry   	= &RobortBeepThreadServer;  
    BAUserThreadInfoTable[BA_THREAD_PARSER_ID].pThreadEntry 	= &RobortParserThreadServer;  
    BAUserThreadInfoTable[BA_THREAD_LIGHT_ID].pThreadEntry  	= &RobortLightThreadServer;  
    BAUserThreadInfoTable[BA_THREAD_MOTOR_ID].pThreadEntry  	= &RobortMotorThreadServer;  
    BAUserThreadInfoTable[BA_THREAD_STEER_ID].pThreadEntry  	= &RobortSteerThreadServer;  
    BAUserThreadInfoTable[BA_THREAD_GPS_ID].pThreadEntry    	= &RobortGpsThreadServer;  
	BAUserThreadInfoTable[BA_THREAD_RECEIVER_ID].pThreadEntry 	= &RobortReceiverThreadServer; 
	BAUserThreadInfoTable[BA_THREAD_SENDER_ID].pThreadEntry    	= &RobortSenderThreadServer; 
}

//pThreadName, StackLength, Priority, Slice are inited when definition
static TError InitUserThreadInfo(TByte ThreadId)
{
    TError err = eFalse;

    BAUserThreadInfoTable[ThreadId].Thread      = &ThreadTable[ThreadId];
    BAUserThreadInfoTable[ThreadId].Argument    = (TArgument)&BAUserThreadInfoTable[ThreadId];

    err = AllocateThreadStack(ThreadId, &BAUserThreadInfoTable[ThreadId].pThreadStack);
    if (err == eFalse)
    {
        return eFalse; 
    }
    
    return eTrue;
}

#endif





