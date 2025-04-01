/**
 * @file debug_task.h
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
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------include-----------------------------------*/
#include "FreeRTOS.h"
#include "bsp_log.h"
#include "cmsis_os.h"
#include "queue.h"

/*-----------------------------------macro------------------------------------*/

/*----------------------------------typedef-----------------------------------*/

/*----------------------------------variable----------------------------------*/

/*-------------------------------------os-------------------------------------*/

/*----------------------------------function----------------------------------*/
void Debug_Task(void *argument);
/*------------------------------------test------------------------------------*/

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#include "Unitree_Go1.h"
#include "VESC_motor.h"
#include "chassis_task.h"
#include "com_config.h"
#include "motor_interface.h"
#include "rm_motor.h"
#include "DM_motor.h"


#endif
