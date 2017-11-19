#ifndef _APP_COMMON_H
#define _APP_COMMON_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "trochili.h"

#include "../bsp/BSP_Gpio.h"

#define MQ_POOL_LEN 16

#define FRAME_HEAD_SIZE		1 //1??
#define LOCAL_ADDR_SIZE     1
#define MASSAGE_H_SIZE      1
#define MASSAGE_L_SIZE      1
#define CMD_TYPE_SIZE       1
#define CRC_H_SIZE			1
#define CRC_L_SIZE			1

#define FRAME_INFO_SIZE     (FRAME_HEAD_SIZE+LOCAL_ADDR_SIZE+MASSAGE_H_SIZE+MASSAGE_L_SIZE+\
							 CMD_TYPE_SIZE+CRC_H_SIZE+CRC_L_SIZE)\
							 
#define ROBORT_LOCAL_ADDRESS 0X01
#define APP_TERMINAL_ADDRESS 0X02

typedef enum
{
	APP_ROBORT_CMD_STOP   		= 0x00, //direction
	APP_ROBORT_CMD_FORWARD		= 0x01,
	APP_ROBORT_CMD_BACK		    = 0x02,
	APP_ROBORT_CMD_LEFT	    	= 0x03,
	APP_ROBORT_CMD_RIGHT		= 0x04,
	APP_ROBORT_CMD_STEER1		= 0x05, //steer engine
	APP_ROBORT_CMD_STEER2		= 0x06, //steer engine
	APP_ROBORT_CMD_STEER3		= 0x07, //steer engine
	APP_ROBORT_CMD_STEER4		= 0x08, //steer engine
	APP_ROBORT_CMD_STEER5		= 0x09, //steer engine
	APP_ROBORT_CMD_STEER6		= 0x0a, //steer engine
	APP_ROBORT_CMD_STEER7		= 0x0b, //steer engine
	APP_ROBORT_CMD_STEER8		= 0x0c, //steer engine
	APP_ROBORT_CMD_SPEED_L		= 0x0d, //speed
	APP_ROBORT_CMD_SPEED_R		= 0x0e, //speed
	APP_ROBORT_CMD_LIGHT_ON		= 0x0f, //light 
	APP_ROBORT_CMD_LIGHT_OFF	= 0x10,
    APP_ROBORT_CMD_BEEP_ON		= 0x11, //beep
   	APP_ROBORT_CMD_BEEP_OFF		= 0x12,
	APP_ROBORT_CMD_GPS_ON		= 0x13,
	APP_ROBORT_CMD_GPS_OFF		= 0x14,
	APP_ROBORT_END
}APP_ROBORT_CMD_TYPE;
	
typedef enum
{
    ROBORT_APP_CMD_ENERGY = 0,
    ROBORT_APP_CMD_HUMIDITY,
    ROBORT_APP_CMD_TEMPERATURE,
    ROBORT_APP_CMD_POSITION,
    ROBORT_APP_CMD_DISTANCE,
    ROBORT_APP_CMD_GPS,
    ROBORT_APP_END
}ROBORT_APP_CMD_TYPE;


typedef unsigned char      uint8;
typedef unsigned char      uchar;
typedef          char      int8;
typedef unsigned short     uint16;
typedef unsigned int       uint32;
typedef unsigned long long uint64;
typedef unsigned long long ulong;

/******************************************************************************
*        The definition of threads
******************************************************************************/
#define PRIORITY_SET(val) {return val;}
#define SLICE_SET(val) {return val;}

#define THREAD_MOTOR_STACK_BYTES    	(256)
#define THREAD_STEER_STACK_BYTES    	(256)
#define THREAD_LIGHT_STACK_BYTES    	(256)
#define THREAD_BEEP_STACK_BYTES     	(256)
#define THREAD_GPS_STACK_BYTES      	(512)
#define THREAD_PARSER_STACK_BYTES   	(256)
#define THREAD_RECEIVER_STACK_BYTES   	(256)
#define THREAD_SENDER_STACK_BYTES   	(256)

typedef enum
{
      BA_THREAD_BEEP_ID         = 0,
      BA_THREAD_PARSER_ID       = 1,   
      BA_THREAD_MOTOR_ID        = 2,
      BA_THREAD_STEER_ID        = 3,
      BA_THREAD_LIGHT_ID        = 4,
      BA_THREAD_GPS_ID          = 5,
	  BA_THREAD_RECEIVER_ID    	= 6,
	  BA_THREAD_SENDER_ID    	= 7,
      BA_THREAD_END_ID,
}BA_THREAD_ID;



typedef struct user_thread_info
{
    TThread	        *Thread;
    TThreadEntry    pThreadEntry;
    TChar	        *pThreadName;
    void            *pThreadStack;
    TBase32         StackLength;   
    TBase32         Priority;
    TTimeTick       Slice;
    TArgument       Argument; 
}BA_USER_THREAD_INFO;

extern TThread 		ThreadTable[BA_THREAD_END_ID];
extern TThreadEntry pThreadEntryTable[BA_THREAD_END_ID];
extern TBase32 		*ThreadStackTable[BA_THREAD_END_ID];
extern BA_USER_THREAD_INFO BAUserThreadInfoTable[];

/******************************************************************************
*        The definition of protocol 
******************************************************************************/
struct protocol
{
	uint8 Frame_Head;       //0xff
	uint8 Addr;             //robort local address
	uint8 Msg_L;            //Data[] length LSB
	uint8 Msg_H;            //Data[] length MSB
	uint8 Cmd;              //Command type
	uint8 *pMsg;    		//msg buffer size
	uint8 Crc_L;            //crc check LSB
	uint8 Crc_H;            //crc check MSB
};


typedef struct protocol App_Robort_Dat_Tag;
typedef struct protocol Robort_App_Dat_Tag;



/******************************************************************************
*        The definition of internal interface 
******************************************************************************/
typedef enum
{
    PARSER_BEEP_DAT_TAG  = 0,
    PARSER_LIGHT_DAT_TAG = 1,
    PARSER_MOTOR_DAT_TAG = 2,
    PARSER_STEER_DAT_TAG = 3,
    PARSER_GPS_DAT_TAG   = 4,
}E_Internal_Entry_Type;

typedef enum
{
    BEEP_OFF = 0,
    BEEP_ON  = 1, 
}E_Internal_Beep_Status;

typedef enum 
{
    LIGHT_OFF = 0,
    LIGHT_ON  = 1,
}E_Internal_LIGHT_Status;


typedef enum
{
    MOTOR_DIR_TYPE   = 0,
    MOTOR_SPEED_TYPE,
}E_Internal_Motor_TYPE;


typedef enum
{
    GPS_ON   = 0,
    GPS_OFF,
}E_Internal_GPS_CMD;

typedef struct
{
    uint8 beep_no;      //beep number
    uint8 beep_status;  //beep status
}Parser_Beep_Dat_Tag;

typedef struct
{
    uint8 light_no;
    uint8 light_status;
}Parser_Light_Dat_Tag;

typedef struct
{
    uint8 type;     //set moving or speed
    uint8 direction;//
    uint8 speed_dir;//left or right
    uint8 speed;
}Parser_Motor_Dat_Tag;

typedef struct
{
    uint8 steer_no;
    uint8 angle;
}Parser_Steer_Dat_Tag;


typedef struct
{
    uint8 gps_cmd;
}Parser_Gps_Dat_Tag;

typedef struct
{
    uint16 msg_len;
    void   *pmsg;    
}Gps_Sender_Dat_Tag;

typedef struct 
{
    uint8 Entry_Type;
    
    union
    {
        //in data
        Parser_Beep_Dat_Tag     p2b_tag;
        Parser_Light_Dat_Tag    p2l_tag;
        Parser_Motor_Dat_Tag    p2m_tag;
        Parser_Steer_Dat_Tag    p2s_tag;
        Parser_Gps_Dat_Tag      p2g_tag;

        //out data
        Gps_Sender_Dat_Tag      g2s_tag;
    }data;
}S_Internal_Interface_Tag;


extern void* InternMsgInPool[MQ_POOL_LEN];
extern TMsgQueue InternMsgInQueue; 

extern void* InternMsgOutPool[MQ_POOL_LEN];
extern TMsgQueue InternMsgOutQueue; 

extern void* ExternMsgInPool[MQ_POOL_LEN];
extern TMsgQueue ExternInMsgQueue; 

extern void* ExternMsgOutPool[MQ_POOL_LEN]; 
extern TMsgQueue ExternOutMsgQueue; 

#endif
