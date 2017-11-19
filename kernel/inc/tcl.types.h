/*************************************************************************************************
 *                                     Trochili RTOS Kernel                                      *
 *                                  Copyright(C) 2017 LIUXUMING                                  *
 *                                       www.trochili.com                                        *
 *************************************************************************************************/
#ifndef _TCL_TYPES_H
#define _TCL_TYPES_H

/* 各种数据类型定义，内核移植时必须确认 */
typedef unsigned char      TByte;
typedef          char      TChar;
typedef unsigned int       TBase32;
typedef unsigned int       TAddr32;
typedef unsigned int       TBitMask;
typedef unsigned long long TTimeTick;
typedef unsigned int       TArgument;
typedef unsigned int       TError;

/* 布尔类型定义                    */
typedef enum
{
    eFalse = 0U,
    eTrue  = 1U
} TBool;

/* API与各函数返回值的统一定义  */
typedef enum
{
    eError   = 0U,
    eSuccess = 1U,
} TState;


/* 寄存器读写宏定义 */
#define TCLM_GET_REG32(r)   (*((volatile unsigned int*)(r)))
#define TCLM_SET_REG32(r,v) (*((volatile unsigned int*)(r)) = ((unsigned int)(v)))

#endif /* _TCL_TYPES_H */

