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

#include "data_pool.h"
/*-----------------------------------macro------------------------------------*/
#define MAX_ACCELERATION 0.5f
#define MAX_VELOCITY 2.7f

/*----------------------------------typedef-----------------------------------*/
typedef enum
{
    NORMAL = 0,// 遥杆数据直驱
    TRAPEZOIDAL,// 遥杆数据梯形规划处理
}Process_method_e;

typedef struct 
{
    float current_velocity;
    float target_velocity;
}TrapezoidalState;
/*----------------------------------function----------------------------------*/
/**
 * @brief 航模遥控数据处理函数
 * 
 * @return uint8_t 
 * @attention 这里尤为注意：航模遥控输出的X、Y 是传统的 前Y 右X 坐标系
 *                  2000
 *             1000 1500 2000
 *                  1000
 *            而机器人实际使用坐标系为 前X 左Y 的通用机器人坐标系，所以在这里面直接对航模遥控的输出做特定处理，就没有特别说再解耦出坐标转换器
 *            然后经过坐标转换之后再给到发布速度
 *            （当然这里只是我懒得搞了，以后你们可以试着解耦doge
 */
void Air_Joy_Process();


void Control_Task(void *argument);


#ifdef __cplusplus
}
#endif

