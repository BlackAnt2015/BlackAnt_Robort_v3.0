/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2017 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCLC_ARM_CM4_H
#define _TCLC_ARM_CM4_H


#include "tcl.types.h"
extern void OsCpuSetupEntry(void);
extern void OsCpuStartTickClock(void);
extern void OsCpuBuildThreadStack(TAddr32* pTop, void* pStack, TBase32 bytes,
                                  void* pEntry, TArgument argument);
extern void OsCpuConfirmThreadSwitch(void);
extern void OsCpuCancelThreadSwitch(void);
extern void OsCpuDisableInt(void);
extern void OsCpuEnableInt(void);
extern void OsCpuEnterCritical(TBase32* pValue);
extern void OsCpuLeaveCritical(TBase32 value);
extern void OsCpuLoadRootThread(void);
extern TBase32 OsCpuCalcHiPRIO(TBase32 data);

#endif /* _TCLC_ARM_CM4_H */

