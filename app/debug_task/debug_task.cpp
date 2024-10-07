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


extern Motor_C610 m2006;

osThreadId_t Debug_TaskHandle;
extern CAN_Tx_Instance_t m2006_1_tx_instance;

__attribute((noreturn)) void Debug_Task(void *argument)
{
    int16_t target_current = 1000;
    for(;;)
    {
        m2006.Out = target_current;
        // Motor_SendMsgs(m2006.can_tx_for_motor,m2006);
        LOGINFO("motor task is running!");
        osDelay(1);
    }
}