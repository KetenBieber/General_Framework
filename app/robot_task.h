
#pragma once

/* RTOS层及mcu main接口 */
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* app 层接口头文件 */
#include "logtest.h"
#include "debug.h"

/* bsp 层接口头文件 */
#include "bsp_log.h"

/* Definitions for TaskHand */
osThreadId_t LogTaskHandle;
osThreadId_t DebugTaskHandle;
osThreadId_t MotorTaskHandle;


/* Definitions for TaskFunc */
void LogTask(void *argument);
void DebugTask(void *argument);
void MotorTask(void *argument);
/**
 * @brief os任务创建初始化函数
 * 
 */
void osTaskInit(void)
{
    const osThreadAttr_t LogTaskHandle_attributes = {
    .name = "LogTaskHandle",
    .stack_size = 128 * 4,
    .priority = (osPriority_t) osPriorityNormal,
    };
    LogTaskHandle = osThreadNew(LogTask, NULL, &LogTaskHandle_attributes);


    const osThreadAttr_t DebugTaskHandle_attributes = {
    .name = "DebugTaskHandle",
    .stack_size = 128 * 4,
    .priority = (osPriority_t) osPriorityNormal,
    };
    DebugTaskHandle = osThreadNew(DebugTask, NULL, &DebugTaskHandle_attributes);


    const osThreadAttr_t MotorTaskHandle_attributes = {
    .name = "MotorTaskHandle",
    .stack_size = 128 * 4,
    .priority = (osPriority_t) osPriorityNormal,
    };
    MotorTaskHandle = osThreadNew(MotorTask, NULL, &MotorTaskHandle_attributes);

}

/**
 * @brief LogTest 任务函数
 *        测试打印日志的效果
 * 
 */
__attribute((noreturn)) void LogTask(void *argument)
{

    for(;;)
    {
        LOGINFO("LogTask is running!");
        osDelay(1);   
    }
}

/**
 * @brief 测试调试的函数任务
 * 
 */
__attribute((noreturn)) void DebugTask(void *argument)
{

    for(;;)
    {
        LOGINFO("DebugTask is running!");
        osDelay(1);
    }
}

__attribute((noreturn)) void MotorTask(void *argument)
{

    for(;;)
    {
        LOGINFO("MotorTask is running!");
        osDelay(1);
    }
}


