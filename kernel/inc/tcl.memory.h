/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2017 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCL_MEMORY_H
#define _TCL_MEMORY_H

#include "tcl.types.h"
#include "tcl.config.h"

#if ((TCLC_MEMORY_ENABLE) && (TCLC_MEMORY_BUDDY_ENABLE))

#define OS_MEM_ERR_NONE               (0x0U)
#define OS_MEM_ERR_FAULT              (0x1<<1)                      /* 一般性错误                 */
#define OS_MEM_ERR_UNREADY            (0x1<<2)                      /* 内存管理结构未初始化       */
#define OS_MEM_ERR_NO_MEM             (0x1<<3)                      /* 可分配内存不足             */
#define OS_MEM_ERR_BAD_ADDR           (0x1<<4)                      /* 释放的内存地址非法         */
#define OS_MEM_ERR_DBL_FREE           (0x1<<5)                      /* 释放的内存没有被分配       */
#define OS_MEM_ERR_POOL_FULL          (0x1<<6)                      /* 释放的内存没有被分配       */

#define OS_MEM_PROP_READY (0x1 << 0)

#define OS_MEM_BUDDY_PAGE_TAGS  ((TCLC_MEMORY_BUDDY_PAGES + 31U) >> 5U)
#define OS_MEM_BUDDY_NODE_TAGS (TCLC_MEMORY_BUDDY_PAGES * 2U - 1U)

typedef struct MemBuddyDef
{
    TBitMask Property;                       /* 内存页池属性                      */
    TChar*    PageAddr;                       /* 被管理的内存的起始地址            */
    TBase32   PageSize;                       /* 内存页大小                        */
    TBase32   PageNbr;                        /* 内存页数目                        */
    TBase32   PageAvail;                      /* 可用内存页数目                    */
    TBitMask  PageTags[OS_MEM_BUDDY_PAGE_TAGS];  /* 内存页是否可用标记                */
    TBase32   NodeNbr;
    TByte     NodeTags[OS_MEM_BUDDY_NODE_TAGS];
} TMemBuddy;


extern TState TclCreateMemoryBuddy(TMemBuddy* pBuddy, TChar* pAddr, TBase32 pages, TBase32 pagesize, TError* pError);
extern TState TclDeleteMemoryBuddy(TMemBuddy* pBuddy, TError* pError);
extern TState TclMallocBuddyMemory(TMemBuddy* pBuddy, TBase32 length, void** pAddr2, TError* pError);
extern TState TclFreeBuddyMemory(TMemBuddy* pBuddy,  void* pAddr, TError* pError);

#endif

#endif /* _TCL_MEMORY_H  */

