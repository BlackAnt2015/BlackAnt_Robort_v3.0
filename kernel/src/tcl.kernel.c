/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2017 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#include "string.h"
#include "tcl.cpu.h"
#include "tcl.thread.h"
#include "tcl.debug.h"
#include "tcl.timer.h"
#include "tcl.kernel.h"

/* 内核关键参数集合 */
TKernelVariable OsKernelVariable;


/*************************************************************************************************
 *  功能：将内核对象加入系统中                                                                   *
 *  参数：(1) pObject 内核对象地址                                                               *
 *        (2) pName   内核对象名称                                                               *
 *        (3) type    内核对象类型                                                               *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void OsKernelAddObject(TObject* pObject, TChar* pName, TObjectType type)
{
    TBase32 len;

    len = strlen(pName);
    len = (len > TCL_OBJ_NAME_LEN)?TCL_OBJ_NAME_LEN:len;
    strncpy(pObject->Name, pName, len);
    pObject->Type = type;
    pObject->ID   = OsKernelVariable.ObjectID;
    OsKernelVariable.ObjectID++;
    OsObjListAddNode(&(OsKernelVariable.ObjectList), &(pObject->LinkNode), OsLinkTail);
}


/*************************************************************************************************
 *  功能：将内核对象从系统中移除                                                                 *
 *  参数：(1) pObject 内核对象地址                                                               *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void OsKernelRemoveObject(TObject* pObject)
{
    OsObjListRemoveNode(&(OsKernelVariable.ObjectList), &(pObject->LinkNode));
    memset(pObject, 0U, sizeof(TObject));
}


/*************************************************************************************************
 *  功能：内核进入中断处理程序                                                                   *
 *  参数：无                                                                                     *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void OsKernelEnterIntrState(void)
{
    TBase32 imask;
    OsCpuEnterCritical(&imask);

    OsKernelVariable.IntrNestTimes++;
    OsKernelVariable.State = OsExtremeState;

    OsCpuLeaveCritical(imask);
}


/*************************************************************************************************
 *  功能：内核退出中断处理程序                                                                   *
 *  参数：无                                                                                     *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void OsKernelLeaveIntrState(void)
{
    TBase32 imask;

    OsCpuEnterCritical(&imask);

    OS_ASSERT((OsKernelVariable.IntrNestTimes > 0U), "");
    OsKernelVariable.IntrNestTimes--;
    if (OsKernelVariable.IntrNestTimes == 0U)
    {
        /*
         * 如果还有其它中断标记在挂起或激活，说明当前中断是最高优先级中断，虽然没有发生嵌套，
         * 但是返回后将进入低级别的中断，所以这种情况不必要进行任务切换，理应留给最后一个并且是
         * 最低级别的那个中断在退出中断时来完成。
         * 此处的线程调度体现的是"抢占"
         */
        if (OsKernelVariable.SchedLocks == 0U)
        {
            OsThreadSchedule();
        }
        OsKernelVariable.State = OsThreadState;
    }

    OsCpuLeaveCritical(imask);
}


/*************************************************************************************************
 *  功能：时钟节拍中断处理函数                                                                   *
 *  参数：无                                                                                     *
 *  返回：无                                                                                     *
 *  说明：定时器处理函数首先运行，随后进行线程调度处理                                           *
 *************************************************************************************************/
void OsKernelTickISR(void)
{
    TBase32 imask;

    OsCpuEnterCritical(&imask);

    /* 内核总运行时间节拍数增加1次 */
    OsKernelVariable.Jiffies++;
    if (OsKernelVariable.Jiffies == 0U)
    {
        OsKernelVariable.JiffyCycles++;
    }

    /* 处理线程时钟节拍 */
    OsThreadTickUpdate();

    /* 处理线程定时器时钟节拍 */
    OsThreadTimerTickUpdate();

    /* 处理用户定时器时钟节拍 */
#if (TCLC_TIMER_ENABLE)
    OsTimerTickUpdate();
#endif

    OsCpuLeaveCritical(imask);
}


/*************************************************************************************************
 *  功能：板级字符串打印函数                                                                     *
 *  参数：(1) pStr 待打印的字符串                                                                *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void TclTrace(const char* pNote)
{
    TBase32 imask;
    OS_ASSERT((pNote != (char*)0), "");
    
    OsCpuEnterCritical(&imask);
    if (OsKernelVariable.TraceEntry != (TTraceEntry)0)
    {
        OsKernelVariable.TraceEntry(pNote);
    }
    OsCpuLeaveCritical(imask);
}


/*************************************************************************************************
 *  功能：关闭任务调度功能                                                                       *
 *  参数：无                                                                                     *
 *  返回：无                                                                                     *
 *  说明：本函数只能被线程调用                                                                   *
 *************************************************************************************************/
TState TclLockScheduler(void)
{
    TState state = eError;
    TBase32 imask;

    OsCpuEnterCritical(&imask);
    if (OsKernelVariable.State == OsThreadState)
    {
        OsKernelVariable.SchedLocks++;
        state = eSuccess;
    }
    OsCpuLeaveCritical(imask);
    return state;
}


/*************************************************************************************************
 *  功能：开启任务调度功能                                                                       *
 *  参数：无                                                                                     *
 *  返回：无                                                                                     *
 *  说明：本函数只能被线程调用                                                                   *
 *************************************************************************************************/
TState TclUnlockScheduler(void)
{
    TState state = eError;
    TBase32 imask;

    OsCpuEnterCritical(&imask);
    if (OsKernelVariable.State == OsThreadState)
    {
        if (OsKernelVariable.SchedLocks > 0U)
        {
            OsKernelVariable.SchedLocks--;
            /*
             * 在关闭调度器的阶段，当前线程有可能使得其他更高优先级的线程就绪，ISR也可能将
             * 一些高优先级的线程解除阻塞。所以在打开调度器的时候，需要做一次线程调度检查，
             * 和系统从中断返回时类似
             */
            if (OsKernelVariable.SchedLocks == 0U)
            {
                OsThreadSchedule();
            }
            state = eSuccess;
        }
    }
    OsCpuLeaveCritical(imask);
    return state;
}


/*************************************************************************************************
 *  功能：设置系统Idle函数供IDLE线程调用                                                         *
 *  参数：(1) pEntry 系统Idle函数                                                                *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void TclSetSysIdleEntry(TSysIdleEntry pEntry)
{
    TBase32 imask;
    OS_ASSERT((pEntry != (TSysIdleEntry)0), "");

    OsCpuEnterCritical(&imask);
    OsKernelVariable.SysIdleEntry = pEntry;
    OsCpuLeaveCritical(imask);
}


/*************************************************************************************************
 *  功能：设置系统Fault函数                                                                      *
 *  参数：(1) pEntry 系统Fault函数                                                               *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void TclSetSysFaultEntry(TSysFaultEntry pEntry)
{
    TBase32 imask;
    OS_ASSERT((pEntry != (TSysFaultEntry)0), "");

    OsCpuEnterCritical(&imask);
    OsKernelVariable.SysFaultEntry = pEntry;
    OsCpuLeaveCritical(imask);
}


/*************************************************************************************************
 *  功能：设置系统Warning函数                                                                    *
 *  参数：(1) pEntry 系统Warning函数                                                             *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void TclSetSysWarningEntry(TSysWarningEntry pEntry)
{
    TBase32 imask;
    OS_ASSERT((pEntry != (TSysWarningEntry)0), "");

    OsCpuEnterCritical(&imask);
    OsKernelVariable.SysWarningEntry = pEntry;
    OsCpuLeaveCritical(imask);
}


/*************************************************************************************************
 *  功能：获得系统当前线程指针                                                                   *
 *  参数：(1) pThread2 返回当前线程指针                                                          *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void TclGetCurrentThread(TThread** pThread2)
{
    TBase32 imask;
    OS_ASSERT((pThread2 != (TThread**)0), "");

    OsCpuEnterCritical(&imask);
    *pThread2 = OsKernelVariable.CurrentThread;
    OsCpuLeaveCritical(imask);
}


/*************************************************************************************************
 *  功能：获得系统已运行时钟节拍数                                                               *
 *  参数：(1) pJiffies 返回系统已运行时钟节拍数                                                  *
 *        (2) pCycles  返回系统已运行时钟轮回数                                                  *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void TclGetTimeStamp(TBase32* pCycles, TTimeTick* pJiffies)
{
    TBase32 imask;
    OS_ASSERT((pJiffies != (TTimeTick*)0), "");

    OsCpuEnterCritical(&imask);
    *pJiffies = OsKernelVariable.Jiffies;
    *pCycles  = OsKernelVariable.JiffyCycles;
    OsCpuLeaveCritical(imask);
}


/*************************************************************************************************
 *  功能：内核启动函数                                                                           *
 *  参数：(1) pUserEntry  应用初始化函数                                                         *
 *        (2) pCpuEntry   处理器初始化函数                                                       *
 *        (3) pBoardEntry 板级设备初始化函数                                                     *
 *        (4) pTraceEntry 调试输出函数                                                           *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void TclStartKernel(TUserEntry pUserEntry,
                    TCpuSetupEntry pCpuEntry,
                    TBoardSetupEntry pBoardEntry,
                    TTraceEntry pTraceEntry)
{
    OS_ASSERT((pUserEntry  != (TUserEntry)0), "");
    OS_ASSERT((pCpuEntry   != (TCpuSetupEntry)0), "");
    OS_ASSERT((pBoardEntry != (TBoardSetupEntry)0), "");
    OS_ASSERT((pTraceEntry != (TTraceEntry)0), "");


    /* 关闭处理器中断 */
    OsCpuDisableInt();

    /* 初始化基本内核参数 */
    memset(&OsKernelVariable, 0U, sizeof(TKernelVariable));
    OsKernelVariable.UserEntry       = pUserEntry;
    OsKernelVariable.CpuSetupEntry   = pCpuEntry;
    OsKernelVariable.BoardSetupEntry = pBoardEntry;
    OsKernelVariable.TraceEntry      = pTraceEntry;
    OsKernelVariable.SchedLocks  = 0U;
    OsKernelVariable.State           = OsOriginState;

    /* 初始化线程管理模块和内核ROOT线程 */
    OsThreadModuleInit();

    /* 初始化用户定时器模块和定时器守护线程 */
#if (TCLC_TIMER_ENABLE)
    OsTimerModuleInit();
#endif

    /* 初始化中断管理模块和中断守护线程 */
#if (TCLC_IRQ_ENABLE)
    OsIrqModuleInit();
#endif

    /* 调用处理器和板级初始化函数 */
    OsKernelVariable.CpuSetupEntry();
    OsKernelVariable.BoardSetupEntry();

    /* 启动内核ROOT线程 */
    OsCpuLoadRootThread();

    /* 打开处理器中断 */
    OsCpuEnableInt();

    /*
     * 本段代码应该永远不会被执行，若运行到此，说明RTOS移植时出现问题。
     * 这里的代码起到兜底作用，避免处理器进入非正常状态
     */
    OsDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
}
