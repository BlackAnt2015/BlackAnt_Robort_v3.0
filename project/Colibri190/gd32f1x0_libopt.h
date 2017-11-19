/*!
    \file  gd32f1x0_libopt.h
    \brief library optional for gd32f1x0
*/

/*
    Copyright (C) 2016 GigaDevice

    2014-12-26, V1.0.0, firmware for GD32F1x0(x=3,5)
    2016-01-15, V2.0.0, firmware for GD32F1x0(x=3,5,7,9)
    2016-04-30, V3.0.0, firmware update for GD32F1x0(x=3,5,7,9)
*/

#ifndef GD32F1X0_LIBOPT_H
#define GD32F1X0_LIBOPT_H

/* Includes */
/* Comment the line below to disable peripheral header file inclusion */
#include "RTE_Components.h"

#ifdef RTE_DEVICE_STDPERIPHERALS_ADC
#include "gd32f1x0_adc.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_CAN
#include "gd32f1x0_can.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_CEC
#include "gd32f1x0_cec.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_CRC
#include "gd32f1x0_crc.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_CMP
#include "gd32f1x0_cmp.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_DAC
#include "gd32f1x0_dac.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_DBG
#include "gd32f1x0_dbg.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_DMA
#include "gd32f1x0_dma.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_EXTI
#include "gd32f1x0_exti.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_FMC
#include "gd32f1x0_fmc.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_GPIO
#include "gd32f1x0_gpio.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_SYSCFG
#include "gd32f1x0_syscfg.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_I2C
#include "gd32f1x0_i2c.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_FWDGT
#include "gd32f1x0_fwdgt.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_PMU
#include "gd32f1x0_pmu.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_RCU
#include "gd32f1x0_rcu.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_RTC
#include "gd32f1x0_rtc.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_SPI
#include "gd32f1x0_spi.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_TIMER
#include "gd32f1x0_timer.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_USART
#include "gd32f1x0_usart.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_WWDGT
#include "gd32f1x0_wwdgt.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_MISC
#include "gd32f1x0_misc.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_TSI
#include "gd32f1x0_tsi.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_SLCD
#include "gd32f1x0_slcd.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_OPA
#include "gd32f1x0_opa.h"
#endif
#ifdef RTE_DEVICE_STDPERIPHERALS_IVREF
#include "gd32f1x0_ivref.h"
#endif

#endif /* __GD32F1X0_LIBOPT_H */

/******************* (C) COPYRIGHT 2016 GIGADEVICE *****END OF FILE****/
