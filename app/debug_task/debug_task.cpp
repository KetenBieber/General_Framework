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


// extern Motor_C610 m2006;

osThreadId_t Debug_TaskHandle;

#ifdef TOPICS_DEBUG
Subscriber* chassis_imu_sub;
Subscriber* chassis_pos_sub;
#endif

__attribute((noreturn)) void Debug_Task(void *argument)
{
    chassis_imu_sub = register_sub("chassis_imu_pub",sizeof(pub_imu_yaw));
    chassis_pos_sub = register_sub("chassis_pos_pub",sizeof(pub_Chassis_Pos));
    publish_data chassis_imu_data;
    publish_data chassis_pos_data;
    for(;;)
    {
        chassis_imu_data = chassis_imu_sub->getdata(chassis_imu_sub);
        if(chassis_imu_data.len != -1)
            chassis_imu_data = chassis_imu_sub->getdata(chassis_imu_sub);
        if(chassis_pos_data.len != -1)
            chassis_pos_data = chassis_pos_sub->getdata(chassis_pos_sub);
        LOGINFO("debug task is running!");
        osDelay(1);
    }
}
