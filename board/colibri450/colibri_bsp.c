//#include "tcl.gd32f450ve.h"
#include "colibri_bsp.h"

/* 配置使能评估板上的设备 */
void EvbSetupEntry(void)
{
  EvbUart1Config();
  EvbLedConfig();//while(1)
  EvbKeyConfig();
}


void EvbTraceEntry(const char* str)
{
    EvbUart1WriteStr(str);
}


