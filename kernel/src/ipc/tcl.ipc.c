/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2017 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#include <string.h>

#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.object.h"
#include "tcl.kernel.h"
#include "tcl.timer.h"
#include "tcl.thread.h"
#include "tcl.ipc.h"

#if (TCLC_IPC_ENABLE)

/*************************************************************************************************
 *  功能：将线程放入资源阻塞队列                                                                 *
 *  参数：(1) pContext阻塞对象地址                                                               *
 *        (2) pIpc    正在操作的IPC对象的地址                                                    *
 *        (3) pQueue  线程队列结构地址                                                           *
 *        (4) data    指向数据目标对象指针的指针                                                 *
 *        (5) len     数据的长度                                                                 *
 *        (6) option  操作IPC对象时的各种参数                                                    *
 *        (7) ticks   时限等待方式下的阻塞时间长度                                               *
 *        (8) pState  IPC对象访问结果                                                            *
 *        (9) pError  详细调用结果                                                               *
 *        (10)pHiRP   是否因当前线程阻塞而导致需要进行线程调度的标记                             *
 *  返回：无                                                                                     *
 *  说明：data指向的指针，就是需要通过IPC机制来传递的数据在线程空间的指针                        *
 *************************************************************************************************/
void OsIpcBlockThread(TIpcContext* pContext, void* pIpc, TIpcQueue* pQueue,
                      TBase32 data, TBase32 len, TBitMask option, TTimeTick ticks,
                      TState* pState, TError* pError, TBool* pHiRP)
{
    TThread* pThread;

    /* 获得线程地址 */
    pThread = OsKernelVariable.CurrentThread;

    /* 只有处于运行状态的线程才可以被阻塞 */
    if (pThread->Status != OS_THREAD_STATUS_RUNNING)
    {
        OsKernelVariable.Diagnosis |= OS_KERNEL_DIAG_THREAD_ERROR;
        pThread->Diagnosis |= OS_THREAD_DIAG_INVALID_STATE;
        OsDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }

    /* 如果当前线程不能被阻塞则函数直接返回 */
    if (pThread ->Property & OS_THREAD_PROP_DAEMON)
    {
        *pState = eError;
        *pError = OS_IPC_ERR_DENIDED;
        *pHiRP  = eFalse;
        return;
    }

    pThread->IpcContext  = pContext;
    pContext->Owner      = (void*)pThread;
    pContext->Object     = pIpc;
    pContext->Queue      = pQueue;
    pContext->Data.Value = data;
    pContext->Length     = len;
    pContext->Option     = option;
    pContext->State      = pState;
    pContext->Error      = pError;

    pContext->LinkNode.Next   = (TLinkNode*)0;
    pContext->LinkNode.Prev   = (TLinkNode*)0;
    pContext->LinkNode.Handle = (TLinkNode**)0;
    pContext->LinkNode.Data   = (TBase32*)(&(pThread->Priority));

    /* 将线程放入内核线程辅助队列 */
    OsThreadLeaveQueue(OsKernelVariable.ThreadReadyQueue, pThread);
    OsThreadEnterQueue(OsKernelVariable.ThreadAuxiliaryQueue, pThread, OsLinkTail);
    pThread->Status = OS_THREAD_STATUS_BLOCKED;

    /* 将线程同时放入阻塞队列。
       如果队列支持抢占模式则按线程优先级来排队，否则按照消息类型来排队；
       除此之外，消息在放入消息队列时，也会统一体现一次紧急消息的加急属性。
       */
    if ((*(pQueue->Property)) & OS_IPC_PROP_PREEMP)
    {
        OsObjQueueAddPriorityNode(&(pQueue->Handle), &(pContext->LinkNode));
    }
    else
    {
        if ((pContext->Option) & OS_IPC_OPT_UARGENT)
        {
            OsObjQueueAddFifoNode(&(pQueue->Handle), &(pContext->LinkNode), OsLinkHead);
        }
        else
        {
            OsObjQueueAddFifoNode(&(pQueue->Handle), &(pContext->LinkNode), OsLinkTail);
        }
    }

    /* 设置线程所属队列 */
    pContext->Queue = pQueue;

    /* 如果需要就启动线程用于访问资源的时限定时器 */
    if ((option & OS_IPC_OPT_TIMEO) && (ticks > 0U))
    {
        pThread->Timer.Ticks = ticks;
        OsObjListAddDiffNode(&(OsKernelVariable.ThreadTimerList),
                             &(pThread->Timer.LinkNode));
    }

    *pState = eSuccess;
    *pError = OS_IPC_ERR_NONE;
    *pHiRP  = eTrue;
}


/*************************************************************************************************
 *  功能：唤醒IPC阻塞队列中指定的线程                                                            *
 *  参数：(1) pContext阻塞对象地址                                                               *
 *        (2) state   线程资源访问返回结果                                                       *
 *        (3) error   详细调用结果                                                               *
 *        (4) pHiRP   是否因唤醒更高优先级而导致需要进行线程调度的标记                           *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void OsIpcUnblockThread(TIpcContext* pContext, TState state, TError error, TBool* pHiRP)
{
    TThread* pThread;
    pThread = (TThread*)(pContext->Owner);

    /*
     * 操作线程，完成线程队列和状态转换,注意只有中断处理时，
     * 当前线程才会处在内核线程辅助队列里(因为还没来得及线程切换)
     * 当前线程返回就绪队列时，一定要回到相应的队列头
     * 当线程进出就绪队列时，不需要处理线程的时钟节拍数
     */
    if (pThread->Status == OS_THREAD_STATUS_BLOCKED)
    {
        /* 将线程从内核线程辅助队列中移出 */
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

        /* 将线程从阻塞队列中移出 */
        OsObjQueueRemoveNode(&(pContext->Queue->Handle), &(pContext->LinkNode));
        pContext->Queue = (TIpcQueue*)0;

        /* 设置线程访问资源的结果和错误代码 */
        *(pContext->State) = state;
        *(pContext->Error) = error;

        /* 如果线程是以时限方式访问资源则关闭该线程的时限定时器 */
        if (pContext->Option & OS_IPC_OPT_TIMEO)
        {
            OsObjListRemoveDiffNode(&(OsKernelVariable.ThreadTimerList),
                                    &(pThread->Timer.LinkNode));
        }

        /* 设置线程调度请求标记,此标记只在线程环境下有效。
         * 在ISR里，当前线程可能在任何队列里，跟当前线程相比较优先级也是无意义的。
         * 在线程环境下，如果当前线程的优先级已经不再是线程就绪队列的最高优先级，
         * 并且内核此时并没有关闭线程调度，那么就需要进行一次线程抢占
         */
        if (pThread->Priority < OsKernelVariable.CurrentThread->Priority)
        {
            *pHiRP = eTrue;
        }
    }
    else if (pThread->Status == OS_THREAD_STATUS_BLOCKED | OS_THREAD_STATUS_SUSPENDED)
    {
        /* 将线程从阻塞队列中移出 */
        OsObjQueueRemoveNode(&(pContext->Queue->Handle), &(pContext->LinkNode));
        pContext->Queue = (TIpcQueue*)0;

        /* 设置线程访问资源的结果和错误代码 */
        *(pContext->State) = state;
        *(pContext->Error) = error;

        /* 如果线程是以时限方式访问资源则关闭该线程的时限定时器 */
        if (pContext->Option & OS_IPC_OPT_TIMEO)
        {
            OsObjListRemoveDiffNode(&(OsKernelVariable.ThreadTimerList),
                                    &(pThread->Timer.LinkNode));
        }
        pThread->Status = OS_THREAD_STATUS_SUSPENDED;
    }
    else
    {
        OsKernelVariable.Diagnosis |= OS_KERNEL_DIAG_THREAD_ERROR;
        pThread->Diagnosis |= OS_THREAD_DIAG_INVALID_STATE;
        OsDebugPanic("", __FILE__, __FUNCTION__, __LINE__);
    }
}


/*************************************************************************************************
 *  功能：选择唤醒阻塞队列中的全部线程                                                           *
 *  参数：(1) pQueue  线程队列结构地址                                                           *
 *        (2) state   线程资源访问返回结果                                                       *
 *        (3) error   详细调用结果                                                               *
 *        (4) pData   线程访问IPC得到的数据                                                      *
 *        (5) pHiRP  线程是否需要调度的标记                                                      *
 *  返回：                                                                                       *
 *  说明：只有邮箱和消息队列广播时才会传递pData2参数                                             *
 *************************************************************************************************/
void OsIpcUnblockAll(TIpcQueue* pQueue, TState state, TError error, void** pData2, TBool* pHiRP)
{
    TIpcContext* pContext;

    /* 将队列中的线程逐一解除阻塞 */
    while (pQueue->Handle != (TLinkNode*)0)
    {
        pContext = OS_CONTAINER_OF(pQueue->Handle, TIpcContext, LinkNode);
        OsIpcUnblockThread(pContext, state, error, pHiRP);

        /* 如果被解除阻塞的线程正在等待读取数据 */
        if ((pContext->Option & OS_IPC_OPT_READ_DATA) &&
                (pContext->Data.Addr2 != (void**)0) &&
                (pData2 != (void**)0) )
        {
            *(pContext->Data.Addr2) = *pData2;
        }
    }
}


/*************************************************************************************************
 *  功能：清除阻塞线程的IPC对象的信息                                                            *
 *  参数：(1) pContext 阻塞对象地址                                                              *
 *  返回：无                                                                                     *
 *  说明：                                                                                       *
 *************************************************************************************************/
void OsIpcCleanContext(TIpcContext* pContext)
{
    TThread* pThread;

    pThread = (TThread*)(pContext->Owner);
    pThread->IpcContext = (TIpcContext*)0;

    memset(pContext, 0U, sizeof(TIpcContext));
}

#endif

