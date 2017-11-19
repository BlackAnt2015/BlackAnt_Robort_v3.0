/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2017 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#include <string.h>

#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.object.h"
#include "tcl.cpu.h"
#include "tcl.ipc.h"
#include "tcl.kernel.h"
#include "tcl.timer.h"
#include "tcl.thread.h"

/* 内核进就绪队列定义,处于就绪和运行的线程都放在这个队列里 */
static TThreadQueue ThreadReadyQueue;

/* 内核线程辅助队列定义，处于延时、挂起、休眠的线程都放在这个队列里 */
static TThreadQueue ThreadAuxiliaryQueue;


/*************************************************************************************************
 *  功能：线程运行监理函数，线程的运行都以它为基础                                               *
 *  参数：(1) pThread  线程地址                                                                  *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
static void SuperviseThread(TThread* pThread)
{
    /* 普通线程需要注意用户不小心退出导致非法指令等死机的问题 */
    OS_ASSERT((pThread == OsKernelVariable.CurrentThread), "");
    pThread->Entry(pThread->Argument);

    OsKernelVariable.Diagnosis |= OS_KERNEL_DIAG_THREAD_ERROR;
    pThread->Diagnosis |= OS_THREAD_DIAG_INVALID_EXIT;
    OsDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
}


/*************************************************************************************************
 *  功能：计算就绪线程队列中的最高优先级函数                                                     *
 *  参数：无                                                                                     *
 *  返回：HiRP (Highest Ready Priority)                                                          *
 *  说明：                                                                                       *
 *************************************************************************************************/
static TBase32 CalcThreadHiRP(void)
{
    TBase32 priority;

    /* 如果就绪优先级不存在则说明内核发生致命错误 */
    if (ThreadReadyQueue.PriorityMask == (TBitMask)0)
    {
        OsKernelVariable.Diagnosis |= OS_KERNEL_DIAG_PRIORITY_ERROR;
        OsDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }
    priority = OsCpuCalcHiPRIO(ThreadReadyQueue.PriorityMask);
    return priority;
}


#if (TCLC_THREAD_STACK_CHECK_ENABLE)
/*************************************************************************************************
 *  功能：告警和检查线程栈溢出问题                                                               *
 *  参数：(1) pThread  线程地址                                                                  *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
static void CheckThreadStack(TThread* pThread)
{
    if ((pThread->StackTop < pThread->StackBarrier) ||
            (*(TBase32*)(pThread->StackBarrier) != TCLC_THREAD_STACK_BARRIER_VALUE))
    {
        OsKernelVariable.Diagnosis |= OS_KERNEL_DIAG_THREAD_ERROR;
        pThread->Diagnosis |= OS_THREAD_DIAG_STACK_OVERFLOW;
        OsDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }

    if (pThread->StackTop < pThread->StackAlarm)
    {
        pThread->Diagnosis |= OS_THREAD_DIAG_STACK_ALARM;
    }
}

#endif


/* RULE
 * 1 当前线程离开就绪队列后，再次加入就绪队列时，
 *   如果仍然是当前线程则一定放在相应的队列头部，而且不重新计算时间片。
 *   如果已经不是当前线程则一定放在相应的队列尾部，而且不重新计算时间片。
 * 2 当前线程在就绪队列内部调整优先级时，在新的队列里也一定要在队列头。
 */

/*************************************************************************************************
 *  功能：将线程加入到指定的线程队列中                                                           *
 *  参数：(1) pQueue  线程队列地址地址                                                           *
 *        (2) pThread 线程结构地址                                                               *
 *        (3) pos     线程在线程队列中的位置                                                     *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void OsThreadEnterQueue(TThreadQueue* pQueue, TThread* pThread, TLinkPos pos)
{
    TBase32 priority;
    TLinkNode** pHandle;

    /* 检查线程和线程队列 */
    OS_ASSERT((pThread != (TThread*)0), "");
    OS_ASSERT((pThread->Queue == (TThreadQueue*)0), "");

    /* 根据线程优先级得出线程实际所属分队列 */
    priority = pThread->Priority;
    pHandle = &(pQueue->Handle[priority]);

    /* 将线程加入指定的分队列 */
    OsObjQueueAddFifoNode(pHandle, &(pThread->LinkNode), pos);

    /* 设置线程所属队列 */
    pThread->Queue = pQueue;

    /* 设定该线程优先级为就绪优先级 */
    pQueue->PriorityMask |= (0x1 << priority);
}


/*************************************************************************************************
 *  功能：将线程从指定的线程队列中移出                                                           *
 *  参数：(1) pQueue  线程队列地址地址                                                           *
 *        (2) pThread 线程结构地址                                                               *
 *  返回：无                                                                                     *
 *  说明：FIFO PRIO两种访问资源的方式                                                            *
 *************************************************************************************************/
void OsThreadLeaveQueue(TThreadQueue* pQueue, TThread* pThread)
{
    TBase32 priority;
    TLinkNode** pHandle;

    /* 检查线程是否属于本队列,如果不属于则内核发生致命错误 */
    OS_ASSERT((pThread != (TThread*)0), "");
    OS_ASSERT((pQueue == pThread->Queue), "");

    /* 根据线程优先级得出线程实际所属分队列 */
    priority = pThread->Priority;
    pHandle = &(pQueue->Handle[priority]);

    /* 将线程从指定的分队列中取出 */
    OsObjQueueRemoveNode(pHandle, &(pThread->LinkNode));

    /* 设置线程所属队列 */
    pThread->Queue = (TThreadQueue*)0;

    /* 处理线程离开队列后对队列优先级就绪标记的影响 */
    if (pQueue->Handle[priority] == (TLinkNode*)0)
    {
        /* 设定该线程优先级未就绪 */
        pQueue->PriorityMask &= (~(0x1 << priority));
    }
}


/*************************************************************************************************
 *  功能：线程时间片处理函数，在时间片中断处理ISR中会调用本函数                                  *
 *  参数：无                                                                                     *
 *  返回：无                                                                                     *
 *  说明：本函数完成了当前线程的时间片处理，但并没有选择需要调度的后继线程和进行线程切换         *
 *************************************************************************************************/
/*
 * 当前线程可能处于3种位置
 * 1 就绪队列的头位置(任何优先级)
 * 2 就绪队列的其它位置(任何优先级)
 * 3 辅助队列或者阻塞队列里
 * 只有情况1才需要进行时间片轮转的处理，但此时不涉及线程切换,因为本函数只在ISR中调用。
 */
void OsThreadTickUpdate(void)
{
    TThread* pThread;
    TLinkNode* pHandle;

    /* 将当前线程时间片减去1个节拍数,线程运行总节拍数加1 */
    pThread = OsKernelVariable.CurrentThread;
    pThread->Jiffies++;
    pThread->Ticks--;

    /* 如果本轮时间片运行完毕 */
    if (pThread->Ticks == 0U)
    {
        /* 恢复线程的时钟节拍数 */
        pThread->Ticks = pThread->BaseTicks;

        /* 判断线程是不是处于内核就绪线程队列的某个优先级的队列头 */
        pHandle = ThreadReadyQueue.Handle[pThread->Priority];
        if (OS_CONTAINER_OF(pHandle, TThread, LinkNode) == pThread)
        {
            /* 如果内核此时允许线程调度,则发起时间片调度，之后pThread处于
             * 线程队列尾部。当前线程所在线程队列也可能只有当前线程唯一1个线程
             */
            if (OsKernelVariable.SchedLocks == 0U)
            {
                ThreadReadyQueue.Handle[pThread->Priority] =
                    (ThreadReadyQueue.Handle[pThread->Priority])->Next;

                /* 将线程状态置为就绪,准备线程切换 */
                pThread->Status = OS_THREAD_STATUS_READY;
            }
        }
    }
}


/*************************************************************************************************
 *  功能：线程定时器处理函数                                                                     *
 *  参数：无                                                                                     *
 *  返回：无                                                                                     *
 *  说明:                                                                                        *
 *************************************************************************************************/
void OsThreadTimerTickUpdate(void)
{
    TThread* pThread;
    TThreadTimer* pTimer;
    TBool HiRP = eFalse;

    /* 得到处于队列头的线程定时器，将对应的定时计数减1 */
    if (OsKernelVariable.ThreadTimerList != (TLinkNode*)0)
    {
        pTimer = OS_CONTAINER_OF(OsKernelVariable.ThreadTimerList, TThreadTimer, LinkNode);
        pTimer->Ticks--;

        /* 处理计数为0的线程定时器 */
        while (pTimer->Ticks == 0U)
        {
            /*
             * 操作线程，完成线程队列和状态转换,注意只有中断处理时，
             * 当前线程才会处在内核线程辅助队列里(因为还没来得及线程切换)
             * 当前线程返回就绪队列时，一定要回到相应的队列头
             * 当线程进出就绪队列时，不需要处理线程的时钟节拍数
             */
            pThread = OS_CONTAINER_OF(pTimer, TThread, Timer);
            switch (pThread->Status)
            {
                case (OS_THREAD_STATUS_DELAYED):
                {
                    OsThreadLeaveQueue(OsKernelVariable.ThreadAuxiliaryQueue, pThread);
                    if (pThread == OsKernelVariable.CurrentThread)
                    {
                        OsThreadEnterQueue(OsKernelVariable.ThreadReadyQueue, pThread, OsLinkHead);
                        pThread->Status = OS_THREAD_STATUS_RUNNING;
                    }
                    else
                    {
                        OsThreadEnterQueue(OsKernelVariable.ThreadReadyQueue, pThread, OsLinkTail);
                        pThread->Status = OS_THREAD_STATUS_READY;
                    }
                    /* 将线程定时器从差分队列中移出 */
                    OsObjListRemoveDiffNode(&(OsKernelVariable.ThreadTimerList), &(pTimer->LinkNode));
                    break;
                }
                case (OS_THREAD_STATUS_DELAYED | OS_THREAD_STATUS_SUSPENDED):
                {
                    pThread->Status &= ~OS_THREAD_STATUS_DELAYED;
                    /* 将线程定时器从差分队列中移出 */
                    OsObjListRemoveDiffNode(&(OsKernelVariable.ThreadTimerList), &(pTimer->LinkNode));
                    break;
                }
#if (TCLC_IPC_ENABLE)
                /* 将线程从阻塞队列中解除阻塞 */
                case (OS_THREAD_STATUS_BLOCKED):
                case (OS_THREAD_STATUS_BLOCKED | OS_THREAD_STATUS_SUSPENDED):
                {
                    OsIpcUnblockThread(pThread->IpcContext, eError, OS_IPC_ERR_TIMEO, &HiRP);
                    break;
                }
#endif
                default:
                {
                    OsKernelVariable.Diagnosis |= OS_KERNEL_DIAG_THREAD_ERROR;
                    pThread->Diagnosis |= OS_THREAD_DIAG_INVALID_STATUS;
                    OsDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
                }
            }

            if (OsKernelVariable.ThreadTimerList == (TLinkNode*)0)
            {
                break;
            }

            /* 获得下一个线程定时器 */
            pTimer = OS_CONTAINER_OF(OsKernelVariable.ThreadTimerList, TThreadTimer, LinkNode);
        }
    }
}


/*************************************************************************************************
 *  功能：用于请求线程调度                                                                       *
 *  参数：无                                                                                     *
 *  返回：无                                                                                     *
 *  说明：线程的调度请求可能被ISR最终取消                                                        *
 *************************************************************************************************/
/*
 * 1 当前线程离开队列即代表它放弃本轮运行，当前线程返回就绪队列时，一定要回到相应的队列头
     当线程进出就绪队列时，不需要处理线程的时钟节拍数
 * 2 导致当前线程不是最高就绪优先级的原因有
 *   1 别的优先级更高的线程进入就绪队列
 *   2 当前线程自己离开队列
 *   3 别的线程的优先级被提高
 *   4 当前线程的优先级被拉低
 *   5 当前线程Yiled
 *   6 时间片中断中，当前线程被轮转
 * 3 在cortex处理器上, 有这样一种可能:
 *   当前线程释放了处理器，但在PendSV中断得到响应之前，又有其它高优先级中断发生，
 *   在高级isr中又把当前线程置为运行，
 *   1 并且当前线程仍然是最高就绪优先级，
 *   2 并且当前线程仍然在最高就绪线程队列的队列头。
 *   此时需要考虑取消PENDSV的操作，避免当前线程和自己切换
 */
void OsThreadSchedule(void)
{
    TBase32 priority;

    /* 查找最高就绪优先级，获得后继线程，如果后继线程指针为空则说明内核发生致命错误 */
    priority = CalcThreadHiRP();
    if (priority >= TCLC_PRIORITY_NUM)
    {
        OsDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }

    if (ThreadReadyQueue.Handle[priority] == (TLinkNode*)0)
    {
        OsDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }

    OsKernelVariable.NomineeThread =
        OS_CONTAINER_OF(ThreadReadyQueue.Handle[priority], TThread, LinkNode);
    if (OsKernelVariable.NomineeThread == (TThread*)0)
    {
        OsDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }

    /*
     * 此处代码逻辑复杂，涉及到很多种线程调度情景，特别是时间片，Yiled、
     * 中断、中断抢占引起的当前线程的状态变化。
     */
    if (OsKernelVariable.NomineeThread != OsKernelVariable.CurrentThread)
    {
#if (TCLC_THREAD_STACK_CHECK_ENABLE)
        CheckThreadStack(OsKernelVariable.NomineeThread);
#endif
        /*
         * 被阻塞之类操作，不需要处理当前线程状态，这里只考虑被抢占的情况。
         * 此时有两种可能，一是线程正常执行，然后有更高优先级的线程就绪。
         * 二是当前线程短暂不就绪但是很快又返回运行状态，(同时/然后)有更高优先级的线程就绪。
         * 不论哪种情况，都需要将当前线程设置为就绪状态。
         */
        if (OsKernelVariable.CurrentThread->Status == OS_THREAD_STATUS_RUNNING)
        {
            OsKernelVariable.CurrentThread->Status = OS_THREAD_STATUS_READY;
        }
        OsCpuConfirmThreadSwitch();
    }
    else
    {
        /*
         * 当前线程短暂不就绪但是很快又返回运行状态，并且此时它还是最高优先级的就绪线程，
         * 重新放回就绪队列，此时在相关代码里已经将当前线程重新设置成运行状态。
         * 而在yield、tick isr里，有可能将当前线程设置成就绪态，而此时当前线程所在
         * 队列又只有唯一一个线程就绪，所以这时需要将当前线程重新设置成运行状态。
         */
        if (OsKernelVariable.CurrentThread->Status == OS_THREAD_STATUS_READY)
        {
            OsKernelVariable.CurrentThread->Status = OS_THREAD_STATUS_RUNNING;
        }
        OsCpuCancelThreadSwitch();
    }
}

/*************************************************************************************************
 *  功能：线程结构初始化函数                                                                     *
 *  参数：(1)  pThread  线程结构地址                                                             *
  *       (2)  pName    线程的初始状态                                                           *
 *        (3)  status   线程的初始状态                                                           *
 *        (4)  property 线程属性                                                                 *
 *        (5)  pEntry   线程函数地址                                                             *
 *        (6)  argument 线程函数参数                                                             *
 *        (7)  pStack   线程栈地址                                                               *
 *        (8)  bytes    线程栈大小，以字为单位                                                   *
 *        (9)  priority 线程优先级                                                               *
 *        (10) ticks    线程时间片长度                                                           *
 *  返回：(1)  eError                                                                            *
 *        (2)  eSuccess                                                                          *
 *  说明：注意栈起始地址、栈大小和栈告警地址的字节对齐问题                                       *
 *************************************************************************************************/
void OsThreadCreate(TThread* pThread, TChar* pName, TBitMask status, TBitMask property,
                    TThreadEntry pEntry, TArgument argument,
                    void* pStack, TBase32 bytes, TBase32 priority, TTimeTick ticks)
{
    TThreadQueue* pQueue;

    /* 初始化线程基本对象信息 */
    OsKernelAddObject(&(pThread->Object), pName, OsObjTypeThread);

    /* 设置线程栈相关数据和构造线程初始栈栈帧 */
    OS_ASSERT((bytes >= TCLC_CPU_MINIMAL_STACK), "");

    /* 栈大小向下对齐 */
    bytes &= (~((TBase32)(TCLC_CPU_STACK_ALIGNED - 1U)));
    pThread->StackBase = (TBase32)pStack + bytes;

    /* 清空线程栈空间 */
    if (property & OS_THREAD_PROP_CLEAN_STACK)
    {
        memset(pStack, 0U, bytes);
    }

    /* 构造(伪造)线程初始栈帧,这里将线程结构地址作为参数传给SuperviseThread()函数 */
    OsCpuBuildThreadStack(&(pThread->StackTop), pStack, bytes, (void*)(&SuperviseThread),
                          (TArgument)pThread);

    /* 计算线程栈告警地址 */
#if (TCLC_THREAD_STACK_CHECK_ENABLE)
    pThread->StackAlarm = (TBase32)pStack + bytes - (bytes* TCLC_THREAD_STACK_ALARM_RATIO) / 100;
    pThread->StackBarrier = (TBase32)pStack;
    (*(TAddr32*)pStack) = TCLC_THREAD_STACK_BARRIER_VALUE;
#endif

    /* 设置线程时间片相关参数 */
    pThread->Ticks = ticks;
    pThread->BaseTicks = ticks;
    pThread->Jiffies = 0U;

    /* 设置线程优先级 */
    pThread->Priority = priority;
    pThread->BasePriority = priority;

    /* 设置线程入口函数和线程参数 */
    pThread->Entry = pEntry;
    pThread->Argument = argument;

    /* 设置线程所属队列信息 */
    pThread->Queue = (TThreadQueue*)0;

    /* 设置线程定时器 */
    pThread->Timer.LinkNode.Data   = (TBase32*)(&(pThread->Timer.Ticks));
    pThread->Timer.LinkNode.Prev   = (TLinkNode*)0;
    pThread->Timer.LinkNode.Next   = (TLinkNode*)0;
    pThread->Timer.LinkNode.Handle = (TLinkNode**)0;
    pThread->Timer.Ticks = (TTimeTick)0;

    /*
     * 线程IPC阻塞上下文结构，没有直接定义在线程结构里，而是在需要阻塞的时候，
     * 临时在线程栈里安排的。好处是减少了线程结构占用的内存。
     */
#if (TCLC_IPC_ENABLE)
    pThread->IpcContext = (TIpcContext*)0;
#endif

    /* 线程占有的锁(MUTEX)队列 */
#if ((TCLC_IPC_ENABLE) && (TCLC_IPC_MUTEX_ENABLE))
    pThread->LockList = (TLinkNode*)0;
#endif

    /* 初始线程运行诊断信息 */
    pThread->Diagnosis = OS_THREAD_DIAG_NORMAL;

    /* 设置线程链表节点信息，线程此时不属于任何线程队列 */
    pThread->LinkNode.Data = (TBase32*)(&(pThread->Priority));
    pThread->LinkNode.Prev = (TLinkNode*)0;
    pThread->LinkNode.Next = (TLinkNode*)0;
    pThread->LinkNode.Handle = (TLinkNode**)0;

    /* 将线程加入内核线程队列，设置线程状态 */
    pQueue = (status == OS_THREAD_STATUS_READY) ?
             (&ThreadReadyQueue): (&ThreadAuxiliaryQueue);
    OsThreadEnterQueue(pQueue, pThread, OsLinkTail);
    pThread->Status = status;

    /* 标记线程已经完成初始化 */
    pThread->Property = (property| OS_THREAD_PROP_READY);
}


/*************************************************************************************************
 *  功能：更改线程优先级                                                                         *
 *  参数：(1) pThread  线程结构地址                                                              *
 *        (2) priority 线程优先级                                                                *
 *        (3) flag     是否被SetPriority API调用                                                 *
 *        (4) pHiRP    是否需要线程调度标记                                                      *
 *        (4) pError   保存操作结果                                                              *
 *  返回：(1) eError   更改线程优先级失败                                                        *
 *        (2) eSuccess 更改线程优先级成功                                                        *
 *  说明：如果是临时修改优先级，则不修改线程结构的基本优先级                                     *
 *************************************************************************************************/
TState OsThreadSetPriority(TThread* pThread, TBase32 priority, TBool flag, TBool* pHiRP,
                           TError* pError)
{
    TState state;
    TError error;
    TBase32 HiRP;
    TBitMask property;

    switch (pThread->Status)
    {
        case (OS_THREAD_STATUS_BLOCKED):
        case (OS_THREAD_STATUS_BLOCKED |OS_THREAD_STATUS_SUSPENDED):
        {
            /* 阻塞状态的线程都在辅助队列里，修改其优先级 */
            OsThreadLeaveQueue(&ThreadAuxiliaryQueue, pThread);
            pThread->Priority = priority;
            OsThreadEnterQueue(&ThreadAuxiliaryQueue, pThread, OsLinkTail);

            /* 修改线程在IPC阻塞队列里优先级 */
            property =*(pThread->IpcContext->Queue->Property);
            if (property & OS_IPC_PROP_PREEMP)
            {
                OsObjQueueRemoveNode(&(pThread->IpcContext->Queue->Handle),
                                     &(pThread->IpcContext->LinkNode));
                OsObjQueueAddPriorityNode(&(pThread->IpcContext->Queue->Handle),
                                          &(pThread->IpcContext->LinkNode));
            }
            state = eSuccess;
            error = OS_THREAD_ERR_NONE;
            break;
        }
        case (OS_THREAD_STATUS_READY):
        {
            /*
             * 就绪线程调整优先级时，可以直接调整其在就绪线程队列中的分队列
             * 对于处于就绪线程队列中的当前线程，如果修改它的优先级，
             * 因为不会把它移出线程就绪队列，所以即使内核不允许调度也没问题
             */
            OsThreadLeaveQueue(&ThreadReadyQueue, pThread);
            pThread->Priority = priority;
            OsThreadEnterQueue(&ThreadReadyQueue, pThread, OsLinkTail);

            /*
             * 得到当前就绪队列的最高就绪优先级，因为就绪线程(包括当前线程)
             * 在线程就绪队列内的折腾会导致当前线程可能不是最高优先级。
             */
            if (priority < OsKernelVariable.CurrentThread->Priority)
            {
                *pHiRP = eTrue;
            }
            state = eSuccess;
            error = OS_THREAD_ERR_NONE;
            break;
        }
        case (OS_THREAD_STATUS_RUNNING):
        {
            /*
             * 假设当前线程优先级最高且唯一，假如调低它的优先级之后仍然是最高，
             * 但是在新的优先级里有多个就绪线程，那么最好把当前线程放在新的就绪队列
             * 的头部，这样不会引起隐式的时间片轮转；当前线程先后被多次调整优先级时，
             * 只有每次都把它放在队列头才能保证它最后一次调整优先级后还处在队列头。
             */
            OsThreadLeaveQueue(&ThreadReadyQueue, pThread);
            pThread->Priority = priority;
            OsThreadEnterQueue(&ThreadReadyQueue, pThread, OsLinkHead);

            /*
             * 因为当前线程在线程就绪队列内的折腾会导致当前线程可能不是最高优先级，
             * 所以需要重新计算当前就绪队列的最高就绪优先级。
             */
            HiRP = CalcThreadHiRP();
            if (HiRP < OsKernelVariable.CurrentThread->Priority)
            {
                *pHiRP = eTrue;
            }

            state = eSuccess;
            error = OS_THREAD_ERR_NONE;
            break;
        }
        case (OS_THREAD_STATUS_DORMANT):
        case (OS_THREAD_STATUS_SUSPENDED):
        case (OS_THREAD_STATUS_DELAYED):
        case (OS_THREAD_STATUS_DELAYED |OS_THREAD_STATUS_SUSPENDED):
        {
            /*其它状态的线程都在辅助队列里，修改其优先级 */
            OsThreadLeaveQueue(&ThreadAuxiliaryQueue, pThread);
            pThread->Priority = priority;
            OsThreadEnterQueue(&ThreadAuxiliaryQueue, pThread, OsLinkTail);
            state = eSuccess;
            error = OS_THREAD_ERR_NONE;
            break;
        }
        default:
        {
            error = OS_THREAD_ERR_STATUS;
            state = eError;
            break;
        }
    }

    /* 如果需要则修改线程固定优先级 */
    if (flag == eTrue)
    {
        pThread->BasePriority = priority;
    }

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能：将线程从挂起状态转换到就绪态，使得线程能够参与内核调度                                 *
 *  参数：(1) pThread   线程结构地址                                                             *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void OsThreadResume(TThread* pThread)
{
    /*
     * 操作线程，完成线程队列和状态转换,注意只有中断处理时，
     * 当前线程才会处在内核线程辅助队列里(因为还没来得及线程切换)
     * 当前线程返回就绪队列时，一定要回到相应的队列头
     * 当线程进出就绪队列时，不需要处理线程的时钟节拍数
     */
    if (pThread->Property & OS_THREAD_PROP_DAEMON)
    {
        if (pThread->Status == OS_THREAD_STATUS_SUSPENDED)
        {
            OsThreadLeaveQueue(&ThreadAuxiliaryQueue, pThread);
            if (pThread == OsKernelVariable.CurrentThread)
            {
                OsThreadEnterQueue(&ThreadReadyQueue, pThread, OsLinkHead);
                pThread->Status = OS_THREAD_STATUS_RUNNING;
            }
            else
            {
                OsThreadEnterQueue(&ThreadReadyQueue, pThread, OsLinkTail);
                pThread->Status = OS_THREAD_STATUS_READY;
            }
        }
    }
}


/*************************************************************************************************
 *  功能：将线程自己挂起                                                                         *
 *  参数：无                                                                                     *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void OsThreadSuspend(void)
{
    if (OsKernelVariable.CurrentThread->Property & OS_THREAD_PROP_DAEMON)
    {
        if (OsKernelVariable.SchedLocks == 0U)
        {
            OsThreadLeaveQueue(&ThreadReadyQueue, OsKernelVariable.CurrentThread);
            OsThreadEnterQueue(&ThreadAuxiliaryQueue,
                               OsKernelVariable.CurrentThread, OsLinkTail);
            OsKernelVariable.CurrentThread->Status = OS_THREAD_STATUS_SUSPENDED;
            OsThreadSchedule();
        }
    }
}


/* 内核ROOT线程定义和栈定义 */
static TThread RootThread;
static TBase32 RootThreadStack[TCLC_ROOT_THREAD_STACK_BYTES >> 2];

/*************************************************************************************************
 *  功能：内核ROOT线程函数                                                                       *
 *  参数：(1) argument 线程的参数                                                                *
 *  返回：无                                                                                     *
 *  说明：该函数首先开启多任务机制，然后调度其它线程运行                                         *
 *        注意线程栈容量大小的问题，这个线程函数不要做太多工作                                   *
 *************************************************************************************************/
static void RootThreadEntry(TBase32 argument)
{
    /* 关闭处理器中断 */
    OsCpuDisableInt();

    /* 标记内核进入多线程模式 */
    OsKernelVariable.State = OsThreadState;

    /* 临时关闭线程调度功能 */
    OsKernelVariable.SchedLocks = 1U;

    /*
     * 调用用户入口函数，初始化用户程序。
     * 该函数运行在OsThreadState,但是禁止Schedulable的状态下
     */
    if(OsKernelVariable.UserEntry == (TUserEntry)0)
    {
        OsDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }
    OsKernelVariable.UserEntry();

    /* 开启线程调度功能 */
    OsKernelVariable.SchedLocks = 0U;

    /* 打开系统时钟节拍 */
    OsCpuStartTickClock();

    /* 打开处理器中断 */
    OsCpuEnableInt();

    /* 调用IDLE Hook函数，此时多线程机制已经打开 */
    while (eTrue)
    {
        if (OsKernelVariable.SysIdleEntry != (TSysIdleEntry)0)
        {
            OsKernelVariable.SysIdleEntry();
        }
    }
}


/*************************************************************************************************
 *  功能：初始化内核线程管理模块                                                                 *
 *  参数：无                                                                                     *
 *  返回：无                                                                                     *
 *  说明：内核中的线程队列主要有一下几种：                                                       *
 *        (1) 线程就绪队列,用于存储所有的就绪线和运行线程。内核中只有一个就绪队列。              *
 *        (2) 线程辅助队列, 所有挂起状态、延时状态和休眠状态的线程都存储在这个队列中。           *
 *            同样内核中只有一个休眠队列                                                         *
 *        (3) IPC对象的线程阻塞队列，数量不定。所有阻塞状态的线程都保存在相应的线程阻塞队列里。  *
 *************************************************************************************************/
void OsThreadModuleInit(void)
{
    /* 检查内核是否处于初始状态 */
    if (OsKernelVariable.State != OsOriginState)
    {
        OsDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }

    memset(&ThreadReadyQueue, 0U, sizeof(ThreadReadyQueue));
    memset(&ThreadAuxiliaryQueue, 0U, sizeof(ThreadAuxiliaryQueue));

    OsKernelVariable.ThreadReadyQueue = &ThreadReadyQueue;
    OsKernelVariable.ThreadAuxiliaryQueue = &ThreadAuxiliaryQueue;

    /* 初始化内核ROOT线程 */
    OsThreadCreate(&RootThread,
                   "ROOTD",/* kernel root thread */
                   OS_THREAD_STATUS_READY,
                   OS_THREAD_PROP_PRIORITY_FIXED|\
                   OS_THREAD_PROP_CLEAN_STACK|\
                   OS_THREAD_PROP_ROOT| OS_THREAD_PROP_DAEMON,
                   RootThreadEntry,
                   (TArgument)0,
                   (void*)RootThreadStack,
                   (TBase32)TCLC_ROOT_THREAD_STACK_BYTES,
                   (TBase32)TCLC_ROOT_THREAD_PRIORITY,
                   (TTimeTick)TCLC_ROOT_THREAD_SLICE);

    /* 初始化相关的内核变量 */
    OsKernelVariable.RootThread    = &RootThread;
    OsKernelVariable.NomineeThread = &RootThread;
    OsKernelVariable.CurrentThread = &RootThread;
}


/*************************************************************************************************
 *  功能：线程结构初始化函数                                                                     *
 *  参数：(1)  pThread  线程结构地址                                                             *
 *        (2)  pName    线程的初始状态                                                           *
 *        (3)  pEntry   线程函数地址                                                             *
 *        (4)  argument 线程函数参数                                                             *
 *        (5)  pStack   线程栈地址                                                               *
 *        (6)  bytes    线程栈大小，以字为单位                                                   *
 *        (7)  priority 线程优先级                                                               *
 *        (8)  ticks    线程时间片长度                                                           *
 *        (9)  pError   详细调用结果                                                             *
 *  返回：(1)  eError                                                                            *
 *        (2)  eSuccess                                                                          *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclCreateThread(TThread* pThread, TChar* pName, TThreadEntry pEntry, TArgument argument,
                       void* pStack, TBase32 bytes, TBase32 priority, TTimeTick ticks, TError* pError)
{
    TBase32 imask;

    /* 必要的参数检查 */
    OS_ASSERT((pThread != (TThread*)0), "");
    OS_ASSERT((pName != (TChar*)0), "");
    OS_ASSERT((pEntry != (void*)0), "");
    OS_ASSERT((pStack != (void*)0), "");
    OS_ASSERT((bytes > 0U), "");
    OS_ASSERT((priority <= TCLC_USER_PRIORITY_LOW), "");
    OS_ASSERT((priority >= TCLC_USER_PRIORITY_HIGH), "");
    OS_ASSERT((ticks > 0U), "");

    OsCpuEnterCritical(&imask);

    /* 只允许在线程代码里调用本函数 */
    if (OsKernelVariable.State != OsThreadState)
    {
        OsCpuLeaveCritical(imask);
        *pError = OS_THREAD_ERR_KERNEL_STATE;
        return eError;
    }

    /* 检查线程是否已经被初始化 */
    if (pThread->Property & OS_THREAD_PROP_READY)
    {
        OsCpuLeaveCritical(imask);
        *pError = OS_THREAD_ERR_UNREADY;
        return eError;
    }

    OsThreadCreate(pThread, pName, OS_THREAD_STATUS_DORMANT, OS_THREAD_PROP_PRIORITY_SAFE,
                   pEntry, argument, pStack, bytes, priority, ticks);

    OsCpuLeaveCritical(imask);

    *pError = OS_THREAD_ERR_NONE;
    return eSuccess;
}


/*************************************************************************************************
 *  功能：线程注销                                                                               *
 *  参数：(1) pThread 线程结构地址                                                               *
 *        (2) pError  详细调用结果                                                               *
 *  返回：(1) eError                                                                             *
 *        (2) eSuccess                                                                           *
 *  说明：IDLE线程、中断处理线程和定时器线程不能被注销                                           *
 *************************************************************************************************/
TState TclDeleteThread(TThread* pThread, TError* pError)
{
    TState state;
    TError error;
    TBase32 imask;

    /* 必要的参数检查 */
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    /* 只允许在线程代码里调用本函数 */
    if (OsKernelVariable.State != OsThreadState)
    {
        OsCpuLeaveCritical(imask);
        *pError = OS_THREAD_ERR_KERNEL_STATE;
        return eError;
    }

    /* 如果没有给出被操作的线程地址，则强制使用当前线程 */
    if (pThread == (TThread*)0)
    {
        pThread = OsKernelVariable.CurrentThread;
    }

    /* 检查线程是否已经被初始化 */
    if (!(pThread->Property & OS_THREAD_PROP_READY))
    {
        OsCpuLeaveCritical(imask);
        *pError = OS_THREAD_ERR_UNREADY;
        return eError;
    }

    /* 内核守护线程不接受某些API调用 */
    if (pThread->Property & OS_THREAD_PROP_DAEMON)
    {
        *pError = OS_THREAD_ERR_DENIED;
        OsCpuLeaveCritical(imask);
        return eError;
    }

    if (pThread->Status == OS_THREAD_STATUS_DORMANT)
    {
#if ((TCLC_IPC_ENABLE) && (TCLC_IPC_MUTEX_ENABLE))
        if (pThread->LockList)
        {
            error = OS_THREAD_ERR_OWNLOCKS;
            state = eError;
        }
        else
#endif
        {
            OsKernelRemoveObject(&(pThread->Object));
            OsThreadLeaveQueue(&ThreadAuxiliaryQueue, pThread);
            memset(pThread, 0U, sizeof(TThread));
            error = OS_THREAD_ERR_NONE;
            state = eSuccess;
        }
    }
    else
    {
        error = OS_THREAD_ERR_STATUS;
        state = eError;
    }

    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能：更改线程优先级                                                                         *
 *  参数：(1) pThread  线程结构地址                                                              *
 *        (2) priority 线程优先级                                                                *
 *        (3) pError   详细调用结果                                                              *
 *  返回：(1) eError   更改线程优先级失败                                                        *
 *        (2) eSuccess 更改线程优先级成功                                                        *
 *  说明：(1) 如果是临时修改优先级，则不修改线程结构的基本优先级数据                             *
 *        (2) 互斥量实施优先级继承协议的时候不受AUTHORITY控制                                    *
 *************************************************************************************************/
TState TclSetThreadPriority(TThread* pThread, TBase32 priority, TError* pError)
{
    TState state;
    TError error;
    TBool HiRP = eFalse;
    TBase32 imask;

    OS_ASSERT((priority < TCLC_PRIORITY_NUM), "");
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    /* 只允许在线程代码里调用本函数 */
    if (OsKernelVariable.State != OsThreadState)
    {
        OsCpuLeaveCritical(imask);
        *pError = OS_THREAD_ERR_KERNEL_STATE;
        return eError;
    }

    /* 如果没有给出被操作的线程地址，则强制使用当前线程 */
    if (pThread == (TThread*)0)
    {
        pThread = OsKernelVariable.CurrentThread;
    }

    /* 检查线程是否已经被初始化 */
    if (!(pThread->Property & OS_THREAD_PROP_READY))
    {
        OsCpuLeaveCritical(imask);
        *pError = OS_THREAD_ERR_UNREADY;
        return eError;
    }

    /* 内核守护线程不接受某些API调用 */
    if (pThread->Property & OS_THREAD_PROP_DAEMON)
    {
        OsCpuLeaveCritical(imask);
        *pError = OS_THREAD_ERR_DENIED;
        return eError;
    }

    if (pThread->Priority == priority)
    {
        OsCpuLeaveCritical(imask);
        *pError = OS_THREAD_ERR_NONE;
        return eSuccess;
    }

    if ((!(pThread->Property & OS_THREAD_PROP_PRIORITY_FIXED)) &&
            (pThread->Property & OS_THREAD_PROP_PRIORITY_SAFE))
    {
        state = OsThreadSetPriority(pThread, priority, eTrue, &HiRP, &error);
        if ((OsKernelVariable.SchedLocks == 0U) && (HiRP == eTrue))
        {
            OsThreadSchedule();
        }
    }
    else
    {
        error = OS_THREAD_ERR_PROTECTED;
        state = eError;
    }

    OsCpuLeaveCritical(imask);
    *pError = error;
    return state;

}


/*************************************************************************************************
 *  功能：修改线程时间片长度                                                                     *
 *  参数：(1) pThread 线程结构地址                                                               *
 *        (2) slice   线程时间片长度                                                             *
 *        (3) pError  详细调用结果                                                               *
 *  返回：(1) eSuccess                                                                           *
 *        (2) eError                                                                           *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclSetThreadSlice(TThread* pThread, TTimeTick ticks, TError* pError)
{
    TBase32 imask;

    OS_ASSERT((ticks > 0U), "");
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    /* 只允许在线程代码里调用本函数 */
    if (OsKernelVariable.State != OsThreadState)
    {
        OsCpuLeaveCritical(imask);
        *pError = OS_THREAD_ERR_KERNEL_STATE;
        return eError;
    }

    /* 如果没有给出被操作的线程地址，则强制使用当前线程 */
    if (pThread == (TThread*)0)
    {
        pThread = OsKernelVariable.CurrentThread;
    }

    /* 检查线程是否已经被初始化 */
    if (!(pThread->Property & OS_THREAD_PROP_READY))
    {
        OsCpuLeaveCritical(imask);
        *pError = OS_THREAD_ERR_UNREADY;
        return eError;
    }

    /* 内核守护线程不接受某些API调用 */
    if (pThread->Property & OS_THREAD_PROP_DAEMON)
    {
        *pError = OS_THREAD_ERR_DENIED;
        OsCpuLeaveCritical(imask);
        return eError;
    }

    /* 调整线程时间片长度 */
    if (pThread->BaseTicks > ticks)
    {
        pThread->Ticks = (pThread->Ticks < ticks) ? (pThread->Ticks): ticks;
    }
    else
    {
        pThread->Ticks += (ticks - pThread->BaseTicks);
    }

    pThread->BaseTicks = ticks;
    OsCpuLeaveCritical(imask);

    *pError = OS_THREAD_ERR_NONE;
    return eSuccess;
}


/*************************************************************************************************
 *  功能：线程级线程调度函数，当前线程主动让出处理器(保持就绪状态)                               *
 *  参数：(1) pError    详细调用结果                                                             *
 *  返回：(1) eError                                                                           *
 *        (2) eSuccess                                                                           *
 *  说明：因为不能破坏最高就绪优先级占用处理器的原则，                                           *
 *        所以Yield操作只能在拥有最高就绪优先级的线程之间操作                                    *
 *************************************************************************************************/
TState TclYieldThread(TError* pError)
{
    TState state;
    TError error;
    TBase32 imask;
    TThread* pThread;

    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    /* 只允许在线程代码里调用本函数 */
    if (OsKernelVariable.State != OsThreadState)
    {
        OsCpuLeaveCritical(imask);
        *pError = OS_THREAD_ERR_KERNEL_STATE;
        return eError;
    }

    /* 强制使用当前线程 */
    pThread = OsKernelVariable.CurrentThread;

    /* 检查线程是否已经被初始化 */
    if (!(pThread->Property & OS_THREAD_PROP_READY))
    {
        OsCpuLeaveCritical(imask);
        *pError = OS_THREAD_ERR_UNREADY;
        return eError;
    }

    /* 内核守护线程不接受某些API调用 */
    if (pThread->Property & OS_THREAD_PROP_DAEMON)
    {
        *pError = OS_THREAD_ERR_DENIED;
        OsCpuLeaveCritical(imask);
        return eError;
    }

    /* 只能在内核允许线程调度的条件下才能调用本函数 */
    if (OsKernelVariable.SchedLocks == 0U)
    {
        /*
         * 调整当前线程所在队列的头指针
         * 当前线程所在线程队列也可能只有当前线程唯一1个线程
         */
        ThreadReadyQueue.Handle[pThread->Priority] =
            (ThreadReadyQueue.Handle[pThread->Priority])->Next;
        pThread->Status = OS_THREAD_STATUS_READY;

        OsThreadSchedule();
        error = OS_THREAD_ERR_NONE;
        state = eSuccess;
    }
    else
    {
        error = OS_THREAD_ERR_SCHED_LOCKED;
        state = eError;
    }

    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能：线程终止，使得线程不再参与内核调度                                                     *
 *  参数：(1) pThread 线程结构地址                                                               *
 *        (2) pError  详细调用结果                                                               *
 *  返回：(1) eError                                                                           *
 *        (2) eSuccess                                                                           *
 *  说明：(1) 初始化线程和定时器线程不能被休眠                                                   *
 *************************************************************************************************/
TState TclDeactivateThread(TThread* pThread, TError* pError)
{
    TState state;
    TError error;
    TBase32 imask;
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    /* 只允许在线程代码里调用本函数 */
    if (OsKernelVariable.State != OsThreadState)
    {
        OsCpuLeaveCritical(imask);
        *pError = OS_THREAD_ERR_KERNEL_STATE;
        return eError;
    }

    /* 如果没有给出被操作的线程地址，则强制使用当前线程 */
    if (pThread == (TThread*)0)
    {
        pThread = OsKernelVariable.CurrentThread;
    }

    /* 检查线程是否已经被初始化 */
    if (!(pThread->Property & OS_THREAD_PROP_READY))
    {
        OsCpuLeaveCritical(imask);
        *pError = OS_THREAD_ERR_UNREADY;
        return eError;
    }

    /* 内核守护线程不接受某些API调用 */
    if (pThread->Property & OS_THREAD_PROP_DAEMON)
    {
        *pError = OS_THREAD_ERR_DENIED;
        OsCpuLeaveCritical(imask);
        return eError;
    }

    switch (pThread->Status)
    {
        case OS_THREAD_STATUS_RUNNING:
        {
            /* 如果内核此时禁止线程调度，那么当前线程不能被操作 */
            if (OsKernelVariable.SchedLocks == 0U)
            {
                OsThreadLeaveQueue(&ThreadReadyQueue, pThread);
                OsThreadEnterQueue(&ThreadAuxiliaryQueue, pThread, OsLinkTail);
                pThread->Status = OS_THREAD_STATUS_DORMANT;
                OsThreadSchedule();
                error = OS_THREAD_ERR_NONE;
                state = eSuccess;
            }
            else
            {
                error = OS_THREAD_ERR_SCHED_LOCKED;
                state = eError;
            }
            break;
        }
        case (OS_THREAD_STATUS_READY):
        {
            /* 如果被操作的线程不是当前线程，则不会引起线程调度，所以直接处理线程和队列 */
            OsThreadLeaveQueue(&ThreadReadyQueue, pThread);
            OsThreadEnterQueue(&ThreadAuxiliaryQueue, pThread, OsLinkTail);
            pThread->Status = OS_THREAD_STATUS_DORMANT;

            error = OS_THREAD_ERR_NONE;
            state = eSuccess;
            break;
        }
        default:
        {
            error = OS_THREAD_ERR_STATUS;
            state = eError;
            break;
        }
    }

    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能：激活线程，使得线程能够参与内核调度                                                     *
 *  参数：(1) pThread  线程结构地址                                                              *
 *        (2) pError   详细调用结果                                                              *
 *  返回：(1) eError                                                                           *
 *        (2) eSuccess                                                                           *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclActivateThread(TThread* pThread, TError* pError)
{
    TState state;
    TError error;
    TBase32 imask;

    OS_ASSERT((pThread != (TThread*)0), "");
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    /* 只允许在线程代码里调用本函数 */
    if (OsKernelVariable.State != OsThreadState)
    {
        OsCpuLeaveCritical(imask);
        *pError = OS_THREAD_ERR_KERNEL_STATE;
        return eError;
    }

    /* 检查线程是否已经被初始化 */
    if (!(pThread->Property & OS_THREAD_PROP_READY))
    {
        OsCpuLeaveCritical(imask);
        *pError = OS_THREAD_ERR_UNREADY;
        return eError;
    }

    /* 内核守护线程不接受某些API调用 */
    if (pThread->Property & OS_THREAD_PROP_DAEMON)
    {
        OsCpuLeaveCritical(imask);
        *pError = OS_THREAD_ERR_DENIED;
        return eError;
    }

    /* 线程状态校验,只有状态符合的线程才能被操作 ,
       因为是在线程环境下，所以此时pThread一定不是当前线程 */
    if (pThread->Status == OS_THREAD_STATUS_DORMANT)
    {
        OsThreadLeaveQueue(&ThreadAuxiliaryQueue, pThread);
        OsThreadEnterQueue(&ThreadReadyQueue, pThread, OsLinkTail);
        pThread->Status = OS_THREAD_STATUS_READY;

        if ((OsKernelVariable.SchedLocks == 0U) &&
                (pThread->Priority < OsKernelVariable.CurrentThread->Priority))
        {
            OsThreadSchedule();
        }

        state = eSuccess;
        error = OS_THREAD_ERR_NONE;
    }

    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能：线程挂起函数                                                                           *
 *  参数：(1) pThread 线程结构地址                                                               *
 *        (2) pError  详细调用结果                                                               *
 *  返回：(1) eError                                                                           *
 *        (2) eSuccess                                                                           *
 *  说明：(1) 内核初始化线程不能被挂起                                                           *
 *************************************************************************************************/
TState TclSuspendThread(TThread* pThread, TError* pError)
{
    TState state;
    TError error;
    TBase32 imask;

    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    /* 只允许在线程代码里调用本函数 */
    if (OsKernelVariable.State != OsThreadState)
    {
        OsCpuLeaveCritical(imask);
        *pError = OS_THREAD_ERR_KERNEL_STATE;
        return eError;
    }

    /* 如果没有给出被操作的线程地址，则强制使用当前线程 */
    if (pThread == (TThread*)0)
    {
        pThread = OsKernelVariable.CurrentThread;
    }

    /* 检查线程是否已经被初始化 */
    if (!(pThread->Property & OS_THREAD_PROP_READY))
    {
        OsCpuLeaveCritical(imask);
        *pError = OS_THREAD_ERR_UNREADY;
        return eError;
    }

    /* 内核守护线程不接受某些API调用 */
    if (pThread->Property & OS_THREAD_PROP_DAEMON)
    {
        OsCpuLeaveCritical(imask);
        *pError = OS_THREAD_ERR_DENIED;
        return eError;
    }

    switch (pThread->Status)
    {
        case (OS_THREAD_STATUS_RUNNING):
        {
            /* 如果内核此时禁止线程调度，那么当前线程不能被操作 */
            if (OsKernelVariable.SchedLocks == 0U)
            {
                OsThreadLeaveQueue(&ThreadReadyQueue, pThread);
                OsThreadEnterQueue(&ThreadAuxiliaryQueue, pThread, OsLinkTail);
                pThread->Status = OS_THREAD_STATUS_SUSPENDED;
                error = OS_THREAD_ERR_NONE;
                state = eSuccess;
                OsThreadSchedule();
            }
            else
            {
                error = OS_THREAD_ERR_SCHED_LOCKED;
                state = eError;
            }
            break;
        }
        case (OS_THREAD_STATUS_READY):
        {
            /* 如果被操作的线程不是当前线程，则不会引起线程调度，所以直接处理线程和队列 */
            OsThreadLeaveQueue(&ThreadReadyQueue, pThread);
            OsThreadEnterQueue(&ThreadAuxiliaryQueue, pThread, OsLinkTail);
            pThread->Status = OS_THREAD_STATUS_SUSPENDED;
            error = OS_THREAD_ERR_NONE;
            state = eSuccess;
            break;
        }
        case (OS_THREAD_STATUS_BLOCKED):
        case (OS_THREAD_STATUS_DELAYED):
        {
            pThread->Status |= OS_THREAD_STATUS_SUSPENDED;
            error = OS_THREAD_ERR_NONE;
            state = eSuccess;
            break;
        }
        case (OS_THREAD_STATUS_SUSPENDED):
        case (OS_THREAD_STATUS_SUSPENDED|OS_THREAD_STATUS_BLOCKED):
        case (OS_THREAD_STATUS_SUSPENDED|OS_THREAD_STATUS_DELAYED):
        {
            error = OS_THREAD_ERR_NONE;
            state = eSuccess;
            break;
        }
        default:
        {
            error = OS_THREAD_ERR_STATUS;
            state = eError;
            break;
        }
    }

    OsCpuLeaveCritical(imask);
    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能：线程解挂函数                                                                           *
 *  参数：(1) pThread 线程结构地址                                                               *
 *        (2) pError  详细调用结果                                                               *
 *  返回：(1) eError                                                                           *
 *        (2) eSuccess                                                                           *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclResumeThread(TThread* pThread, TError* pError)
{
    TState state;
    TError error;
    TBase32 imask;

    OS_ASSERT((pThread != (TThread*)0), "");
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    /* 只允许在线程代码里调用本函数 */
    if (OsKernelVariable.State != OsThreadState)
    {
        OsCpuLeaveCritical(imask);
        *pError = OS_THREAD_ERR_KERNEL_STATE;
        return eError;
    }

    /* 检查线程是否已经被初始化 */
    if (!(pThread->Property & OS_THREAD_PROP_READY))
    {
        OsCpuLeaveCritical(imask);
        *pError = OS_THREAD_ERR_UNREADY;
        return eError;
    }

    /* 内核守护线程不接受某些API调用 */
    if (pThread->Property & OS_THREAD_PROP_DAEMON)
    {
        OsCpuLeaveCritical(imask);
        *pError = OS_THREAD_ERR_DENIED;
        return eError;
    }

    /* 线程状态校验,只有状态符合的线程才能被操作 */
    switch (pThread->Status)
    {
        case OS_THREAD_STATUS_SUSPENDED:
        {
            /* 因为是在线程环境下，所以此时pThread一定不是当前线程 */
            OsThreadLeaveQueue(&ThreadAuxiliaryQueue, pThread);
            OsThreadEnterQueue(&ThreadReadyQueue, pThread, OsLinkTail);
            pThread->Status = OS_THREAD_STATUS_READY;

            if (pThread->Priority < OsKernelVariable.CurrentThread->Priority)
            {
                OsThreadSchedule();
            }

            error = OS_THREAD_ERR_NONE;
            state = eSuccess;
            break;
        }
        case (OS_THREAD_STATUS_SUSPENDED|OS_THREAD_STATUS_BLOCKED):
        case (OS_THREAD_STATUS_SUSPENDED|OS_THREAD_STATUS_DELAYED):
        {
            pThread->Status &= (~OS_THREAD_STATUS_SUSPENDED);
            error = OS_THREAD_ERR_NONE;
            state = eSuccess;
            break;
        }
        default:
        {
            error = OS_THREAD_ERR_STATUS;
            state = eError;
            break;
        }
    }

    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能：线程延时模块接口函数                                                                   *
 *  参数：(1) pThread 线程结构地址                                                               *
 *        (2) ticks   需要延时的滴答数目                                                         *
 *        (3) pError  详细调用结果                                                               *
 *  返回：(1) eSuccess                                                                           *
 *        (2) eError                                                                           *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclDelayThread(TTimeTick ticks, TError* pError)
{
    TState state;
    TError error;
    TBase32 imask;
    TThread* pThread;

    OS_ASSERT((ticks > 0U), "");
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    /* 只允许在线程代码里调用本函数 */
    if (OsKernelVariable.State != OsThreadState)
    {
        OsCpuLeaveCritical(imask);
        *pError = OS_THREAD_ERR_KERNEL_STATE;
        return eError;
    }

    /* 强制使用当前线程 */
    pThread = OsKernelVariable.CurrentThread;

    /* 检查线程是否已经被初始化 */
    if (!(pThread->Property & OS_THREAD_PROP_READY))
    {
        OsCpuLeaveCritical(imask);
        *pError = OS_THREAD_ERR_UNREADY;
        return eError;
    }

    /* 内核守护线程不接受某些API调用 */
    if (pThread->Property & OS_THREAD_PROP_DAEMON)
    {
        OsCpuLeaveCritical(imask);
        *pError = OS_THREAD_ERR_DENIED;
        return eError;
    }

    switch (pThread->Status)
    {
        case (OS_THREAD_STATUS_RUNNING):
        {
            /* 如果内核此时禁止线程调度，那么当前线程不能被操作 */
            if (OsKernelVariable.SchedLocks == 0U)
            {
                OsThreadLeaveQueue(&ThreadReadyQueue, pThread);
                OsThreadEnterQueue(&ThreadAuxiliaryQueue, pThread, OsLinkTail);
                pThread->Status = OS_THREAD_STATUS_DELAYED;

                /* 重置并启动线程定时器 */
                pThread->Timer.Ticks = ticks;
                OsObjListAddDiffNode(&(OsKernelVariable.ThreadTimerList), &(pThread->Timer.LinkNode));

                OsThreadSchedule();
                error = OS_THREAD_ERR_NONE;
                state= eSuccess;
            }
            else
            {
                error = OS_THREAD_ERR_SCHED_LOCKED;
                state = eError;
            }
            break;
        }
        case (OS_THREAD_STATUS_READY):
        {
            /* 如果被操作的线程不是当前线程，则不会引起线程调度，所以直接处理线程和队列 */
            OsThreadLeaveQueue(&ThreadReadyQueue, pThread);
            OsThreadEnterQueue(&ThreadAuxiliaryQueue, pThread, OsLinkTail);
            pThread->Status = OS_THREAD_STATUS_DELAYED;

            /* 重置并启动线程定时器 */
            pThread->Timer.Ticks = ticks;
            OsObjListAddDiffNode(&(OsKernelVariable.ThreadTimerList), &(pThread->Timer.LinkNode));

            error = OS_THREAD_ERR_NONE;
            state = eSuccess;
            break;
        }
        case (OS_THREAD_STATUS_DELAYED):
        case (OS_THREAD_STATUS_DELAYED |OS_THREAD_STATUS_SUSPENDED):
        {
            /* 将线程定时器从差分队列中移出,重置并启动线程定时器 */
            OsObjListRemoveDiffNode(&(OsKernelVariable.ThreadTimerList), &(pThread->Timer.LinkNode));
            pThread->Timer.Ticks = ticks;
            OsObjListAddDiffNode(&(OsKernelVariable.ThreadTimerList), &(pThread->Timer.LinkNode));
            error = OS_THREAD_ERR_NONE;
            state = eSuccess;
            break;
        }
        default:
        {
            error = OS_THREAD_ERR_STATUS;
            state = eError;
            break;
        }
    }

    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能：线程延时取消函数                                                                       *
 *  参数：(1) pThread 线程结构地址                                                               *
 *        (2) pError  详细调用结果                                                               *
 *  返回：(1) eSuccess                                                                           *
 *        (2) eError                                                                           *
 *  说明：(1) 这个函数对以时限等待方式阻塞在IPC线程阻塞队列上的线程无效                          *
 *************************************************************************************************/
TState TclUndelayThread(TThread* pThread, TError* pError)
{
    TState state;
    TError error;
    TBase32 imask;

    OS_ASSERT((pThread != (TThread*)0), "");
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    /* 只允许在线程代码里调用本函数 */
    if (OsKernelVariable.State != OsThreadState)
    {
        OsCpuLeaveCritical(imask);
        *pError = OS_THREAD_ERR_KERNEL_STATE;
        return eError;
    }

    /* 检查线程是否已经被初始化 */
    if (!(pThread->Property & OS_THREAD_PROP_READY))
    {
        OsCpuLeaveCritical(imask);
        *pError = OS_THREAD_ERR_UNREADY;
        return eError;
    }

    /* 内核守护线程不接受某些API调用 */
    if (pThread->Property & OS_THREAD_PROP_DAEMON)
    {
        OsCpuLeaveCritical(imask);
        *pError = OS_THREAD_ERR_DENIED;
        return eError;
    }

    switch (pThread->Status)
    {
        case OS_THREAD_STATUS_DELAYED:
        {
            /* 因为是在线程环境下，所以此时pThread一定不是当前线程 */
            OsThreadLeaveQueue(&ThreadAuxiliaryQueue, pThread);
            OsThreadEnterQueue(&ThreadReadyQueue, pThread, OsLinkTail);
            pThread->Status = OS_THREAD_STATUS_READY;

            /* 停止线程定时器 */
            OsObjListRemoveDiffNode(&(OsKernelVariable.ThreadTimerList), &(pThread->Timer.LinkNode));

            if (pThread->Priority < OsKernelVariable.CurrentThread->Priority)
            {
                OsThreadSchedule();
            }
            error = OS_THREAD_ERR_NONE;
            state = eSuccess;
            break;
        }
        case (OS_THREAD_STATUS_SUSPENDED|OS_THREAD_STATUS_DELAYED):
        {
            pThread->Status &= (~OS_THREAD_STATUS_DELAYED);
            /* 停止线程定时器 */
            OsObjListRemoveDiffNode(&(OsKernelVariable.ThreadTimerList), &(pThread->Timer.LinkNode));
            error = OS_THREAD_ERR_NONE;
            state = eSuccess;
            break;
        }
        default:
        {
            error = OS_THREAD_ERR_STATUS;
            state = eError;
            break;
        }
    }

    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}


#if (TCLC_IPC_ENABLE)
/*************************************************************************************************
 *  功能：解除线程阻塞函数                                                                       *
 *  参数：(1) pThread 线程结构地址                                                               *
 *        (2) pError  详细调用结果                                                               *
 *  返回：(1) eError                                                                           *
 *        (2) eSuccess                                                                           *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclUnblockThread(TThread* pThread, TError* pError)
{
    TState state;
    TError error;
    TBool HiRP = eFalse;
    TBase32 imask;

    OS_ASSERT((pThread != (TThread*)0), "");
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    /* 只允许在线程代码里调用本函数 */
    if (OsKernelVariable.State != OsThreadState)
    {
        OsCpuLeaveCritical(imask);
        *pError = OS_THREAD_ERR_KERNEL_STATE;
        return eError;
    }

    /* 检查线程是否已经被初始化 */
    if (!(pThread->Property & OS_THREAD_PROP_READY))
    {
        OsCpuLeaveCritical(imask);
        *pError = OS_THREAD_ERR_UNREADY;
        return eError;
    }

    /* 内核守护线程不接受某些API调用 */
    if (pThread->Property & OS_THREAD_PROP_DAEMON)
    {
        OsCpuLeaveCritical(imask);
        *pError = OS_THREAD_ERR_DENIED;
        return eError;
    }

    if (pThread->Status & OS_THREAD_STATUS_BLOCKED)
    {
        /*
         * 将阻塞队列上的指定阻塞线程释放
         * 在线程环境下，如果当前线程的优先级已经不再是线程就绪队列的最高优先级，
         * 并且内核此时并没有关闭线程调度，那么就需要进行一次线程抢占
         */
        OsIpcUnblockThread(pThread->IpcContext, eError, OS_IPC_ERR_ABORT, &HiRP);
        if ((OsKernelVariable.SchedLocks == 0U) && (HiRP == eTrue))
        {
            OsThreadSchedule();
        }
        error = OS_THREAD_ERR_NONE;
        state = eSuccess;
    }
    else
    {
        error = OS_THREAD_ERR_STATUS;
        state = eError;
    }

    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}
#endif

