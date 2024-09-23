
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
#include "bsp_dwt.h"

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
 * @brief LogTask 任务函数
 *        测试打印日志的效果
 * @attention 需要添加测试模块进行时间监测！
 */
__attribute((noreturn)) void LogTask(void *argument)
{
    static float Log_start;
    static float Log_dt;
    static char sLog_dt[20];
    for(;;)
    {
        Log_start = DWT_GetTimeline_ms();
        LOGINFO("LogTask is running!");
        Log_dt = DWT_GetTimeline_ms() - Log_start;
        Float2Str(sLog_dt,Log_dt);
        LOGWARNING("LogTask last time is %s",sLog_dt);
        if(Log_dt > 1)
        {
            LOGERROR("LogTask is being DELAY!!! dt= [%s] ms", sLog_dt);
        }
        osDelay(2);   
    }
}

/**
 * @brief 测试调试的函数任务
 * 
 */
__attribute((noreturn)) void DebugTask(void *argument)
{
    static float Debug_start;
    static float Debug_dt;
    static char sDebug_dt[20];
    for(;;)
    {
        Debug_start = DWT_GetTimeline_ms();
        LOGINFO("DebugTask is running!");
        Debug_dt = DWT_GetTimeline_ms() - Debug_start;
        Float2Str(sDebug_dt,Debug_dt);
        LOGWARNING("DebugTask last time is %s",sDebug_dt);
        if(Debug_dt > 1)
        {
            LOGERROR("DebugTask is being DELAY!!! dt= [%s] ms", sDebug_dt);
        }
        osDelay(1);
    }
}

__attribute((noreturn)) void MotorTask(void *argument)
{
    static float Motor_start;
    static float Motor_dt;
    static char sMotor_dt[20];
    for(;;)
    {
        Motor_start = DWT_GetTimeline_ms();
        LOGINFO("MotorTask is running!");
        Motor_dt = DWT_GetTimeline_ms() - Motor_start;
        Float2Str(sMotor_dt,Motor_dt);
        LOGWARNING("MotorTask last time is %s",sMotor_dt);
        assert_param(sMotor_dt == NULL);
        if(Motor_dt > 1)
        {
            LOGERROR("MotorTask is being DELAY!!! dt= [%s] ms", sMotor_dt);
        }
        osDelay(1);
    }
}


