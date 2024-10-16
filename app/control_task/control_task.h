/**
 * @file control_task.h
 * @author Keten (2863861004@qq.com)
 * @brief 控制器任务,调用各类控制器：预期加入 pid 控制器、LQR 控制器、状态观测器
 * @version 0.1
 * @date 2024-10-04
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
#include "task.h"
#include "cmsis_os.h"

#include "bsp_gpio.h"
#include "bsp_usart.h"


#include "air_joy.h"
/*-----------------------------------macro------------------------------------*/

/*----------------------------------typedef-----------------------------------*/

/*----------------------------------variable----------------------------------*/


/*----------------------------------function----------------------------------*/
void Control_Task(void *argument);


#ifdef __cplusplus
}
#endif

