/**
 * @file tracking.h
 * @author Keten (2863861004@qq.com)
 * @brief 
 * @version 0.1
 * @date 2024-10-29
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention :
 * @note : 开发需求，一些运动所需的跟踪器
 *         1.角度跟踪器
 *         2.位置跟踪器 
 * @versioninfo :
 */
#ifndef TRACKING_H 
#define TRACKING_H 

#ifdef __cplusplus
extern "C"{
#endif

/*----------------------------------include-----------------------------------*/
#include "user_tool.h"
#include "pid_controller.h"

/*-----------------------------------macro------------------------------------*/

/*----------------------------------typedef-----------------------------------*/

/*----------------------------------variable----------------------------------*/

/**
 * @brief 角度跟踪器 --- 基于pid控制器
 *        可以以最短距离跟踪目标角度  
 *          
 * @param yaw_pid pid控制器结构体
 * @param Target_Yaw 
 * @param Current_Yaw 
 * @param measure_min 传感器所能观测的角度最小值
 * @param measure_max 传感器所能观测的角度最大值
 * @return uint8_t 
 */
uint8_t Yaw_Adjust(PID_t *yaw_pid,float Target_Yaw,float Current_Yaw,float measure_min,float measure_max);


/*----------------------------------function----------------------------------*/


#ifdef __cplusplus
}
#endif

#endif	/* TRACKING_H */
