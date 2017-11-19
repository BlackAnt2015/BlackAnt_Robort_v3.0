/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2017 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCLC_IPC_H
#define _TCLC_IPC_H

#include "tcl.config.h"
#include "tcl.types.h"
#include "tcl.object.h"

#if (TCLC_IPC_ENABLE)

/* IPC线程阻塞队列结构定义 */
struct IpcBlockedQueueDef
{
    TBitMask*  Property;                      /* 线程阻塞队列属性                       */
    TLinkNode* Handle;                        /* 队列中基本线程分队列                   */
};
typedef struct IpcBlockedQueueDef TIpcQueue;


/* IPC操作结果，内核代码使用 */
#define OS_IPC_ERR_NONE           (TError)(0x0)       /* 操作成功                               */
#define OS_IPC_ERR_FAULT          (TError)(0x1<<0)    /* 函数/参数用法错误                      */
#define OS_IPC_ERR_UNREADY        (TError)(0x1<<1)    /* IPC对象没有被初始化                    */
#define OS_IPC_ERR_NORMAL         (TError)(0x1<<2)    /* 信号量:信号量的数值不能满足操作
                                                       * 邮箱和消息队列:状态不能被满足操作
                                                       * 事件标记:待发送的事件已经存在 or
                                                       * 接收事件时期待的事件不能被满足
                                                       * 互斥量： 互斥量已被其他线程占用 or
                                                       * 互斥量不属于当前线程
                                                       */
#define OS_IPC_ERR_TIMEO          (TError)(0x1<<3)    /* 因阻塞时限到达线程被唤醒               */
#define OS_IPC_ERR_DELETE         (TError)(0x1<<4)    /* IPC对象被销毁，线程被唤醒              */
#define OS_IPC_ERR_RESET          (TError)(0x1<<5)    /* IPC对象被重置，线程被唤醒              */
#define OS_IPC_ERR_FLUSH          (TError)(0x1<<6)    /* IPC阻塞队列上的线程被迫中止            */
#define OS_IPC_ERR_ABORT          (TError)(0x1<<7)    /* IPC阻塞队列上的线程被迫中止            */
#define OS_IPC_ERR_DENIDED        (TError)(0x1<<8)    /* 线程不能以阻塞方式访问IPC对象          */

/* IPC对象属性，内核代码使用 */
#define OS_IPC_PROP_DEFAULT       (TBitMask)(0x0)
#define OS_IPC_PROP_READY         (TBitMask)(0x1<<0) /* IPC对象已经被初始化                    */
#define OS_IPC_PROP_PREEMP        (TBitMask)(0x1<<1) /* 线程阻塞队列采用优先级调度方案     */

#define OS_RESET_SEMAPHORE_PROP   (OS_IPC_PROP_READY | OS_IPC_PROP_PREEMP)
#define OS_RESET_MUTEX_PROP       (OS_IPC_PROP_READY | OS_IPC_PROP_PREEMP)
#define OS_RESET_MBOX_PROP        (OS_IPC_PROP_READY | OS_IPC_PROP_PREEMP)
#define OS_RESET_MQUE_PROP        (OS_IPC_PROP_READY | OS_IPC_PROP_PREEMP)
#define OS_RESET_FLAG_PROP        (OS_IPC_PROP_READY | OS_IPC_PROP_PREEMP)

#define OS_USER_SEMAPHORE_PROP    (OS_IPC_PROP_PREEMP)
#define OS_USER_MUTEX_PROP        (OS_IPC_PROP_PREEMP)
#define OS_USER_MBOX_PROP         (OS_IPC_PROP_PREEMP)
#define OS_USER_MQUE_PROP         (OS_IPC_PROP_PREEMP)
#define OS_USER_FLAG_PROP         (OS_IPC_PROP_PREEMP)

/* 线程IPC选项，内核代码使用 */
#define OS_IPC_OPT_DEFAULT        (TBitMask)(0x0)
#define OS_IPC_OPT_WAIT           (TBitMask)(0x1<<1)   /* 永久方式等待IPC                        */
#define OS_IPC_OPT_TIMEO          (TBitMask)(0x1<<2)   /* 时限方式等待标记                       */
#define OS_IPC_OPT_UARGENT        (TBitMask)(0x1<<3)   /* 消息队列、邮件使用                     */
#define OS_IPC_OPT_AND            (TBitMask)(0x1<<4)   /* 标记事件标记操作是AND类型              */
#define OS_IPC_OPT_OR             (TBitMask)(0x1<<5)   /* 标记事件标记操作是OR类型               */
#define OS_IPC_OPT_CONSUME        (TBitMask)(0x1<<6)   /* 事件标记使用                           */

#define OS_IPC_OPT_SEMAPHORE      (TBitMask)(0x1<<16)  /* 标记线程阻塞在信号量的线程阻塞队列中   */
#define OS_IPC_OPT_MUTEX          (TBitMask)(0x1<<17)  /* 标记线程阻塞在互斥量的线程阻塞队列中   */
#define OS_IPC_OPT_MAILBOX        (TBitMask)(0x1<<18)  /* 标记线程阻塞在邮箱的线程阻塞队列中     */
#define OS_IPC_OPT_MSGQUEUE       (TBitMask)(0x1<<19)  /* 标记线程阻塞在消息队列的线程阻塞队列中 */
#define OS_IPC_OPT_FLAGS          (TBitMask)(0x1<<20)  /* 标记线程阻塞在事件标记的线程阻塞队列中 */

#define OS_IPC_OPT_READ_DATA      (TBitMask)(0x1<<24)  /* 接收邮件或者消息                       */
#define OS_IPC_OPT_WRITE_DATA     (TBitMask)(0x1<<25)  /* 发送邮件或者消息                       */

#define OS_USER_SEMAPHORE_OPTION  (OS_IPC_OPT_WAIT | OS_IPC_OPT_TIMEO)
#define OS_USER_MUTEX_OPTION      (OS_IPC_OPT_WAIT | OS_IPC_OPT_TIMEO)
#define OS_USER_MBOX_OPTION       (OS_IPC_OPT_WAIT | OS_IPC_OPT_TIMEO | OS_IPC_OPT_UARGENT)
#define OS_USER_MSGQ_OPTION       (OS_IPC_OPT_WAIT | OS_IPC_OPT_TIMEO | OS_IPC_OPT_UARGENT)
#define OS_USER_FLAG_OPTION       (OS_IPC_OPT_WAIT | OS_IPC_OPT_TIMEO | OS_IPC_OPT_AND |\
                                   OS_IPC_OPT_OR | OS_IPC_OPT_CONSUME)

#define OS_ISR_MSGQ_OPTION        (OS_IPC_OPT_UARGENT)

/* NOTE: not compliant MISRA2004 18.4: Unions shall not be used. */
union IpcDataDef
{
    TBase32 Value;                                    /* 保存被传输数据变量的地址址值           */
    void*   Addr1;                                    /* 指向事件标记的一级指针                 */
    void**  Addr2;                                    /* 指向消息或者邮件的二级指针             */
};
typedef union IpcDataDef TIpcData;

/* 线程用于记录IPC对象的详细信息的记录结构 */
struct IpcContextDef
{
    void*        Object;                              /* 指向IPC对象地址的指针                  */
    TIpcQueue*   Queue;                               /* 线程所属IPC线程队列指针                */
    TIpcData     Data;                                /* 和IPC对象操作相关的数据指针            */
    TBase32      Length;                              /* 和IPC对象操作相关的数据长度            */
    TBitMask     Option;                              /* 访问IPC对象的操作参数                  */
    TState*      State;                               /* IPC对象操作的返回值                    */
    TError*      Error;                               /* IPC对象操作的错误代码                  */
    void*        Owner;                               /* IPC对象所属线程                        */
    TLinkNode    LinkNode;                            /* 线程所在IPC队列的链表节点              */
};
typedef struct IpcContextDef TIpcContext;


extern void OsIpcBlockThread(TIpcContext* pContext, void* pIpc, TIpcQueue* pQueue, TBase32 data, TBase32 len,
                        TBitMask option, TTimeTick ticks, TState* pState, TError* pError, TBool* pHiRP);
extern void OsIpcUnblockThread(TIpcContext* pContext, TState state, TError error, TBool* pHiRP);
extern void OsIpcUnblockAll(TIpcQueue* pQueue, TState state, TError error, void** pData2, TBool* pHiRP);
extern void OsIpcCleanContext(TIpcContext* pContext);
extern void OsIpcSetPriority(TIpcContext* pContext, TBase32 priority);

#endif

#endif /* _TCLC_IPC_H */

