/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2017 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#include "string.h"
#include "tcl.cpu.h"
#include "tcl.thread.h"
#include "tcl.timer.h"
#include "tcl.kernel.h"


/*************************************************************************************************
 *  功能：内核出现严重问题而停止继续运行                                                         *
 *  参数：(1) pNote  提示字符串                                                                  *
 *        (2) pFile  出错的文件名                                                                *
 *        (3) pFunc  出错的函数名                                                                *
 *        (4) line   出错的代码行                                                                *
 *  返回：无                                                                                     *
 *  说明：这是内核诊断到自身发生错误时采取的行动                                                 *
 *************************************************************************************************/
void OsDebugPanic(const char* pNote, const char* pFile, const char* pFunc, int line)
{
    OsCpuDisableInt();

    OsKernelVariable.Debug.File = pFile;
    OsKernelVariable.Debug.Func = pFunc;
    OsKernelVariable.Debug.Line = line;
    OsKernelVariable.Debug.Note = pNote;

    if (OsKernelVariable.TraceEntry != (TTraceEntry)0)
    {
        OsKernelVariable.TraceEntry(pNote);
    }
    
    if (OsKernelVariable.SysFaultEntry != (TSysFaultEntry)0)
    {
        OsKernelVariable.SysFaultEntry(&OsKernelVariable);
    }

    while (eTrue);
}


/*************************************************************************************************
 *  功能：内核出现非严重问题而进行警告                                                           *
 *  参数：(1) pNote  提示字符串                                                                  *
 *  返回：无                                                                                     *
 *  说明：这是内核诊断到自身发生错误时采取的行动                                                 *
 *************************************************************************************************/
void OsDebugWarning(const char* pNote)
{
    TBase32 imask;

    OsCpuEnterCritical(&imask);
    if (OsKernelVariable.TraceEntry != (TTraceEntry)0)
    {
        OsKernelVariable.TraceEntry(pNote);
    }
    OsCpuLeaveCritical(imask);

    if (OsKernelVariable.SysWarningEntry != (TSysWarningEntry)0)
    {
        OsKernelVariable.SysWarningEntry(&OsKernelVariable);
    }
}

