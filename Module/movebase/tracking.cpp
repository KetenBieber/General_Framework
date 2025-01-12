/**
 * @file tracking.cpp
 * @author Keten (2863861004@qq.com)
 * @brief 
 * @version 0.1
 * @date 2024-10-29
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention :
 * @note :
 * @versioninfo :
 */
#include "tracking.h"




uint8_t Yaw_Adjust(PID_t *yaw_pid,float Target_Yaw,float Current_Yaw,float measure_min,float measure_max)
{
    // 将目标角度限制在 能被观测的 角度范围内
    ANGLE_LIMIT(Target_Yaw,measure_min,measure_max);

    float temp_yaw_error;
    if(Current_Yaw*Target_Yaw >= 0)
    {
        temp_yaw_error = Target_Yaw - Current_Yaw;
    }
    else
    {
        if(ABS(Target_Yaw) + ABS(Current_Yaw) <= (measure_max - measure_min) / 2)
        {
            temp_yaw_error = Target_Yaw - Current_Yaw;
        }
        else
        {
            if(Current_Yaw > 0)
            {
                temp_yaw_error = 1*Target_Yaw - (Current_Yaw - (measure_max - measure_min));
            }
            else
            {
                temp_yaw_error = 1*(Target_Yaw + (measure_max - measure_min)) - Current_Yaw;
            }
        }
    }
    PID_Calculate(yaw_pid,Current_Yaw,Target_Yaw);

    return (ABS(temp_yaw_error) < 0.5f) ? 1 : 0;
}
