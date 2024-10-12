
#pragma once

/* RTOS层及mcu main接口 */
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* bsp 层接口头文件 */
#include "bsp_log.h"
#include "bsp_dwt.h"
#include "bsp_bitband.h"

/* app 层接口头文件 */
#include "com_config.h"
#include "chassis_task.h"
#include "robot_ins.h"

/* module层接口头文件 */
#include "soft_iwdg.h"
#include "topics.h"

/* Definitions for TaskHand */
osThreadId_t LogTaskHandle;
extern osThreadId_t ins_TaskHandle;
osThreadId_t MotorTaskHandle;
osThreadId_t IWDGTaskHandle;
extern osThreadId_t CAN1_Send_TaskHandle;
extern osThreadId_t Debug_TaskHandle;


/* Definitions for TaskFunc */
void LogTask(void *argument);
void ins_Task(void *argument);
void MotorTask(void *argument);
void IWDGTask(void *argument);
void CAN1_Send_Task(void *argument);
void Debug_Task(void *argument);
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
    // LogTaskHandle = osThreadNew(LogTask, NULL, &LogTaskHandle_attributes);


    const osThreadAttr_t ins_TaskHandle_attributes = {
    .name = "ins_TaskHandle",
    .stack_size = 128 * 8,
    .priority = (osPriority_t) osPriorityNormal,
    };
    ins_TaskHandle = osThreadNew(ins_Task, NULL, &ins_TaskHandle_attributes);


    const osThreadAttr_t MotorTaskHandle_attributes = {
    .name = "MotorTaskHandle",
    .stack_size = 128 *4,
    .priority = (osPriority_t) osPriorityNormal,
    };
    MotorTaskHandle = osThreadNew(MotorTask, NULL, &MotorTaskHandle_attributes);

    const osThreadAttr_t IWDGTaskHandle_attributes = {
    .name = "IWDGTaskHandle",
    .stack_size = 128*4 ,
    .priority = (osPriority_t) osPriorityNormal,
    };
    // IWDGTaskHandle = osThreadNew(IWDGTask, NULL, &IWDGTaskHandle_attributes);

    const osThreadAttr_t CAN1_SendTaskHandle_attributes = {
    .name = "CAN1_Send_TaskHandle",
    .stack_size = 128*4 ,
    .priority = (osPriority_t) osPriorityNormal,
    };
    CAN1_Send_TaskHandle = osThreadNew(CAN1_Send_Task, NULL, &CAN1_SendTaskHandle_attributes);

    const osThreadAttr_t DebugTaskHandle_attributes = {
    .name = "Debug_TaskHandle",
    .stack_size = 128*4 ,
    .priority = (osPriority_t) osPriorityNormal,
    };
    Debug_TaskHandle = osThreadNew(Debug_Task, NULL, &DebugTaskHandle_attributes);
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
        if(Log_dt > 1)
        {
            LOGERROR("LogTask is being DELAY!!! dt= [%s] ms", sLog_dt);
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
        // LOGINFO("MotorTask is running!");
        Motor_dt = DWT_GetTimeline_ms() - Motor_start;
        Float2Str(sMotor_dt,Motor_dt);
        PEout(12) = 0;
        if(Motor_dt > 1)
        {
            LOGERROR("MotorTask is being DELAY!!! dt= [%s] ms", sMotor_dt);
        }
        osDelay(100);
    }
}

__attribute((noreturn)) void IWDGTask(void *argument)
{
    static float IWDG_start;
    static float IWDG_dt;
    static char sIWDG_dt[20];
    for(;;)
    {
        IWDG_start = DWT_GetTimeline_ms();
        IWDG_Task();
        LOGINFO("the dog is feeding!");
        IWDG_dt = DWT_GetTimeline_ms() - IWDG_start;
        Float2Str(sIWDG_dt,IWDG_dt);
        if(IWDG_dt > 1)
        {
            LOGERROR("IWDGTask is being DELAY!!! dt= [%s] ms", sIWDG_dt);
        }
        osDelay(100);
    }
}


