#include "example.h"
#include "trochili.h"

#if (EVB_EXAMPLE == CH7_MESSAGE_EXAMPLE8)

/* 用户线程参数 */
#define THREAD_LED_STACK_BYTES  (512)
#define THREAD_LED_PRIORITY     (7)
#define THREAD_LED_SLICE        (20)

#define THREAD_CTRL_STACK_BYTES (512)
#define THREAD_CTRL_PRIORITY    (6)
#define THREAD_CTRL_SLICE       (20)

/* 用户线程定义 */
static TThread ThreadLed1;
static TThread ThreadLed2;
static TThread ThreadLed3;
static TThread ThreadCTRL;

/* 用户线程栈定义 */
static TBase32 ThreadLed1Stack[THREAD_LED_STACK_BYTES/4];
static TBase32 ThreadLed2Stack[THREAD_LED_STACK_BYTES/4];
static TBase32 ThreadLed3Stack[THREAD_LED_STACK_BYTES/4];
static TBase32 ThreadCTRLStack[THREAD_CTRL_STACK_BYTES/4];

/* 用户消息类型定义 */
typedef struct
{
    TBase32 Index;
    TBase32 Value;
} TLedMsg;

/* 用户消息队列定义 */
#define MQ_POOL_LEN (32)
static TMsgQueue LedMQ;
static void* LedMsgPool[MQ_POOL_LEN];

/* Led1线程的主函数 */
static void ThreadLed1Entry(TArgument data)
{
    TState state;
    TError error;
    TLedMsg* pMsg;

    while (eTrue)
    {
        /* Led1线程以阻塞方式接收消息，如果发现消息队列被重置则点亮Led1 */
        state = TclReceiveMessage(&LedMQ, (void**)(&pMsg), TCLO_IPC_WAIT,
                                  0, &error);
        if ((state != eSuccess) && (error & TCLE_IPC_DELETE))
        {
            EvbLedControl(LED1,  LED_ON);
        }

        /* Led1线程以阻塞方式接收消息，如果发现消息队列被重置则熄灭Led1 */
        state = TclReceiveMessage(&LedMQ, (void**)(&pMsg), TCLO_IPC_WAIT,
                                  0, &error);
        if ((state != eSuccess) && (error & TCLE_IPC_DELETE))
        {
            EvbLedControl(LED1,  LED_OFF);
        }
    }
}

/* Led2的线程的主函数 */
static void ThreadLed2Entry(TArgument data)
{
    TState state;
    TError error;
    TLedMsg* pMsg;

    while (eTrue)
    {
        /* Led2线程以阻塞方式接收消息，如果发现消息队列被重置则点亮Led2 */
        state = TclReceiveMessage(&LedMQ, (void**)(&pMsg), TCLO_IPC_WAIT,
                                  0, &error);
        if ((state != eSuccess) && (error & TCLE_IPC_DELETE))
        {
            EvbLedControl(LED2, LED_ON);
        }

        /* Led2线程以阻塞方式接收消息，如果发现消息队列被重置则熄灭Led2 */
        state = TclReceiveMessage(&LedMQ, (void**)(&pMsg), TCLO_IPC_WAIT,
                                  0, &error);
        if ((state != eSuccess) && (error & TCLE_IPC_DELETE))
        {
            EvbLedControl(LED2, LED_OFF);
        }
    }
}

/* Led3线程的主函数 */
static void ThreadLed3Entry(TArgument data)
{
    TState state;
    TError error;
    TLedMsg* pMsg;


    while (eTrue)
    {
        /* Led3线程以阻塞方式接收消息，如果发现消息队列被重置则点亮Led3 */
        state = TclReceiveMessage(&LedMQ, (void**)(&pMsg), TCLO_IPC_WAIT,
                                  0, &error);
        if ((state != eSuccess) && (error & TCLE_IPC_DELETE))
        {
            EvbLedControl(LED3,  LED_ON);
        }

        /* Led3线程以阻塞方式接收消息，如果发现消息队列被重置则熄灭Led3 */
        state = TclReceiveMessage(&LedMQ, (void**)(&pMsg), TCLO_IPC_WAIT,
                                  0, &error);
        if ((state != eSuccess) && (error & TCLE_IPC_DELETE))
        {
            EvbLedControl(LED3, LED_OFF);
        }
    }
}

/* CTRL线程的主函数 */
static void ThreadCtrlEntry(TArgument data)
{
    TState state;
    TError error;

    while (eTrue)
    {
        /* CTRL线程延时1秒 */
        state = TclDelayThread(TCLM_MLS2TICKS(1000), &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

        /* 重置消息队列 */
        state = TclDeleteMsgQueue(&LedMQ, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");

        /* 重置后的消息队列需要再次初始化才能使用 */
        state = TclCreateMsgQueue(&LedMQ, "queue renewal", (void**)(&LedMsgPool),
                                MQ_POOL_LEN, TCLP_IPC_DEFAULT, &error);
        TCLM_ASSERT((state == eSuccess), "");
        TCLM_ASSERT((error == TCLE_IPC_NONE), "");
    }
}


/* 用户应用入口函数 */
static void AppSetupEntry(void)
{
    TState state;
    TError error;

    /* 初始化消息队列 */
    state = TclCreateMsgQueue(&LedMQ, "queue", (void**)(&LedMsgPool),
                            MQ_POOL_LEN, TCLP_IPC_DEFAULT, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_IPC_NONE), "");

    /* 初始化Led线程1 */
    state = TclCreateThread(&ThreadLed1, "thread led1",
                          &ThreadLed1Entry, (TArgument)0,
                          ThreadLed1Stack, THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY, THREAD_LED_SLICE, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 初始化Led线程2 */
    state = TclCreateThread(&ThreadLed2, "thread led2",
                          &ThreadLed2Entry, (TArgument)0,
                          ThreadLed2Stack, THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY, THREAD_LED_SLICE,
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 初始化Led线程3 */
    state = TclCreateThread(&ThreadLed3, "thread led3",
                          &ThreadLed3Entry, (TArgument)0,
                          ThreadLed3Stack, THREAD_LED_STACK_BYTES,
                          THREAD_LED_PRIORITY, THREAD_LED_SLICE,
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 初始化CTRL线程 */
    state = TclCreateThread(&ThreadCTRL, "thread ctrl",
                          &ThreadCtrlEntry, (TArgument)0,
                          ThreadCTRLStack, THREAD_CTRL_STACK_BYTES,
                          THREAD_CTRL_PRIORITY, THREAD_CTRL_SLICE,
                          &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 激活Led线程1 */
    state = TclActivateThread(&ThreadLed1, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 激活Led线程2 */
    state = TclActivateThread(&ThreadLed2, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 激活Led线程3 */
    state = TclActivateThread(&ThreadLed3, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");

    /* 激活CTRL线程 */
    state = TclActivateThread(&ThreadCTRL, &error);
    TCLM_ASSERT((state == eSuccess), "");
    TCLM_ASSERT((error == TCLE_THREAD_NONE), "");
}


/* 处理器BOOT之后会调用main函数，必须提供 */
int main(void)
{
    /* 注册各个内核函数,启动内核 */
    TclStartKernel(&AppSetupEntry,
                   &CpuSetupEntry,
                   &EvbSetupEntry,
                   &EvbTraceEntry);

    return 1;
}

#endif

