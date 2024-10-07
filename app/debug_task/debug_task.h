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
extern "C"{
#endif

/*----------------------------------include-----------------------------------*/
#include "FreeRTOS.h"
#include "queue.h"
#include "cmsis_os.h"
#include "bsp_log.h"

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
#include "rm_motor.h"
#include "motor_interface.h"
#include "chassis_task.h"

#endif



