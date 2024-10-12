/**
 * @file debug.cpp
 * @author Keten (2863861004@qq.com)
 * @brief 
 * @version 0.1
 * @date 2024-10-07
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention :
 * @note :
 * @versioninfo :
 */
#include "debug_task.h"
#include "topics.h"
#include "bsp_bitband.h"


// extern Motor_C610 m2006;

osThreadId_t Debug_TaskHandle;




__attribute((noreturn)) void Debug_Task(void *argument)
{
   
    for(;;)
    {
   
        // chassis_motor[0].Out = 500;
        // chassis_motor[1].Out = 500;
        // chassis_motor[2].Out = 500;  
        // chassis_motor[3].Out = 500;
        // Motor_SendMsgs(chassis_motor);
        // PFout(8) = 0;

        LOGINFO("debug task is running!");
        osDelay(10);
    }
}
