#include "../../app/beep.h"
#include "../../app/light.h"
#include "../../app/motor.h"
#include "../../app/steer.h"
#include "../../app/parser.h"
#include "../../app/gps.h"
#include "../../app/sender.h"
#include "../../app/receiver.h"

void BASystemSetupEntry(void)
{
    Robort_Beep_Init();
    Robort_Light_Init();
    Robort_Motor_Init();
    Robort_Steer_Init();
    Robort_Parser_Init();
    Robort_GPS_Init();
    Robort_Sender_Init();
    Robort_Receiver_Init(); 
}


void EvbTraceEntry(const char* str)
{


}
