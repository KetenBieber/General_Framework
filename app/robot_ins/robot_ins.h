/**
 * @file robot_ins.h
 * @author Keten (2863861004@qq.com)
 * @brief 
 * @version 0.1
 * @date 2024-09-28
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention :
 * @note :
 * @versioninfo :
 */
#pragma once

/* RTOS层及mcu main接口 */
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "robot_def.h"
/* bsp层接口 */
#include "bsp_log.h"
#include "bsp_usart.h"
#include "bsp_dwt.h"
/* module层接口 */
#include "action.h"
#include "soft_iwdg.h"
#include "topics.h"
/* app层接口 */
#include "data_pool.h"

typedef struct 
{
    Publisher *chassis_imu_pub;
    pub_imu_yaw *imu_data;// 将发布的航向角数据
    Publisher *chassis_pos_pub;
    pub_Chassis_Pos *pos_data;// 将发布的坐标数据
}Robot_Ins_t;


uint8_t action_iwdg_callback(void *instance);
void ins_Task(void *argument);




