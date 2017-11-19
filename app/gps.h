#ifndef _APP_GPS_H
#define _APP_GPS_H

#include "common.h"
#include "../bsp/BSP_Gps.h"
#include "../bsp/BSP_Uart.h"

struct GPS_Info{
	union{
		uint32_t fLat;
		u8   uLat[4];
	}lat;
	union {
		uint32_t fLon;
		u8    uLon[4];   
	}lon;
	union{
		uint32_t fDir;
		u8    uDir[4];
	}direction;
	u8 sAtinfo_iNuse;
	u8 sAtinfo_iNview;
	union{
		uint32_t fSpeed;
		u8    uSpeed[4]; 
	}speed;	
};

extern struct GPS_Info GPS_Info_Data;


void Robort_GPS_Init(void);
void RobortGpsThreadServer(uint32 argAddr);
static void GPS_DATA_Init( void);
static uint16 Robort_Fetch_GPS_Data(void **pMsg);



#endif
