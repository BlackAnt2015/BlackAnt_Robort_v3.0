/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2017 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#include "string.h"

#include "tcl.types.h"
#include "tcl.config.h"
#include "tcl.cpu.h"
#include "tcl.kernel.h"
#include "tcl.pool.h"

#if (TCLC_POOL_ENABLE)
/*************************************************************************************************
 *  功能: 初始化对象池                                                                           *
 *  参数: (1) pPool      对象池结构地址                                                          *
 *        (2) pAddr      对象池数据区地址                                                        *
 *        (3) pages      对象池中对象数目                                                        *
 *        (4) pgsize     对象大小                                                                *
 *        (5) pError     详细调用结果                                                            *
 *  返回: (1) eSuccess   操作成功                                                                *
 *        (2) eError     操作失败                                                                *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclCreatePool(TObjPool* pPool, void* pAddr, TBase32 number, TBase32 size, TError* pError)
{
    TState state = eError;
    TError error = OS_POOL_ERR_FAULT;
    TBase32 imask;
    TBase32 index;
    TChar* pTemp;

    OS_ASSERT((pPool != (TObjPool*)0), "");
    OS_ASSERT((pAddr != (void*)0), "");
    OS_ASSERT((number != 0U), "");
    OS_ASSERT((size > 0U), "");
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    if (!(pPool->Property & OS_POOL_PROP_READY))
    {
        /* 清空被管理的对象空间 */
        memset(pAddr, 0U, number * size);

        /* 建立可用对象链表 */
        pTemp = (TChar*)pAddr;
        for (index = 0; index < number; index++)
        {
            OsObjListAddNode(&(pPool->ObjList), (TLinkNode*)pTemp, OsLinkHead);
            pTemp += size;
        }

        pPool->ObjAddr  = pAddr;
        pPool->ObjAvail = number;
        pPool->ObjNbr   = number;
        pPool->ObjSize  = size;
        pPool->Property = OS_POOL_PROP_READY;

        error = OS_POOL_ERR_NONE;
        state = eSuccess;
    }

    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能: 销毁对象池                                                                             *
 *  参数: (1) pPool      对象池结构地址                                                          *
 *        (2) pError     详细调用结果                                                            *
 *  返回: (1) eSuccess   操作成功                                                                *
 *        (2) eError     操作失败                                                                *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclDeletePool(TObjPool* pPool, TError* pError)
{
    TBase32 imask;
    TState state = eError;
    TError error = OS_POOL_ERR_UNREADY;

    OS_ASSERT((pPool != (TObjPool*)0), "");
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);
    if (pPool->Property & OS_POOL_PROP_READY)
    {
        if (pPool->ObjAvail == pPool->ObjNbr)
        {
            memset(pPool->ObjAddr, 0U, pPool->ObjNbr * pPool->ObjSize);
            memset(pPool, 0U, sizeof(TObjPool));
            error = OS_POOL_ERR_NONE;
            state = eSuccess;
        }
        else
        {
            error = OS_POOL_ERR_FAULT;
            state = eError;
        }
    }
    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能: 从对象池中申请分配对象                                                                 *
 *  参数: (1) pPool      对象池结构地址                                                          *
 *        (2) pAddr2     保存申请到的对象的指针变量                                              *
 *        (3) pError     详细调用结果                                                            *
 *  返回: (1) eSuccess   操作成功                                                                *
 *        (2) eError     操作失败                                                                *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclMallocObject(TObjPool* pPool, void** pAddr2, TError* pError)
{
    TState state = eError;
    TError error = OS_POOL_ERR_UNREADY;
    TBase32 imask;
    TChar* pTemp;

    OS_ASSERT((pPool != (TObjPool*)0), "");
    OS_ASSERT((pAddr2 != (void**)0), "");
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    if (pPool->Property & OS_POOL_PROP_READY)
    {
        /* 如果对象池存在可以对象 */
        if (pPool->ObjAvail > 0U)
        {
            /* 将首对象分配出去,清空该对象内容 */
            pTemp = (TChar*)(pPool->ObjList);
            OsObjListRemoveNode(&(pPool->ObjList), (TLinkNode*)pTemp);
            memset((void*)pTemp, 0U, pPool->ObjSize);
            *pAddr2 = (void*)pTemp;
            pPool->ObjAvail--;

            error = OS_POOL_ERR_NONE;
            state = eSuccess;
        }
        else
        {
            error = OS_POOL_ERR_EMPTY;
        }
    }

    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}


/*************************************************************************************************
 *  功能: 向对象池中释放对象                                                                     *
 *  参数: (1) pPool      对象池结构地址                                                          *
 *        (2) pAddr      待释放对象的地址                                                        *
 *        (3) pError     详细调用结果                                                            *
 *  返回: (1) eSuccess   操作成功                                                                *
 *        (2) eError     操作失败                                                                *
 *  说明：                                                                                       *
 *************************************************************************************************/
TState TclFreeObject(TObjPool* pPool, void* pAddr, TError* pError)
{
    TState state = eError;
    TError error = OS_POOL_ERR_UNREADY;
    TBase32 imask;
    TBase32 index;
    TChar* pTemp;

    OS_ASSERT((pPool != (TObjPool*)0), "");
    OS_ASSERT((pAddr != (void*)0), "");
    OS_ASSERT((pError != (TError*)0), "");

    OsCpuEnterCritical(&imask);

    if (pPool->Property & OS_POOL_PROP_READY)
    {
        /* 如果对象池确实有对象被分配出去尚未回收 */
        if (pPool->ObjAvail < pPool->ObjNbr)
        {
            /* 检查释放的对象地址是否真的处在合适的块起始地址上。
               此处代码要求被管理的对象空间必须是连续的 */
            index = ((TChar*)pAddr - pPool->ObjAddr) / (pPool->ObjSize);
            //  index = (index < pPool->ObjNbr) ? index : pPool->ObjNbr;
            pTemp = pPool->ObjAddr + index * pPool->ObjSize;

            /* 如果该地址满足条件，确实是处在某个对象的首地址 */
            if (pTemp == (TChar*)pAddr)
            {
                /* 清空该对象,收回该地址的对象 */
                memset(pAddr, 0U, pPool->ObjSize);                
                OsObjListAddNode(&(pPool->ObjList), (TLinkNode*)pAddr, OsLinkTail);
                pPool->ObjAvail++;

                error = OS_POOL_ERR_NONE;
                state = eSuccess;
            }
            else
            {
                error = OS_POOL_ERR_ADDR;
            }
        }
        else
        {
            error = OS_POOL_ERR_FULL;
        }
    }

    OsCpuLeaveCritical(imask);

    *pError = error;
    return state;
}

#endif

