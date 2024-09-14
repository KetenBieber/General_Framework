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
osThreadId_t LogTestTaskHandHandle;
osThreadId_t DebugTaskHandleHandle;

/* Definitions for TaskFunc */
void LogTestTask(void *argument);
void DebugTask(void *argument);

/**
 * @brief os任务创建初始化函数
 * 
 */
void osTaskInit(void)
{
    const osThreadAttr_t LogTestTaskHand_attributes = {
    .name = "LogTestTaskHand",
    .stack_size = 128 * 4,
    .priority = (osPriority_t) osPriorityLow,
    };
    LogTestTaskHandHandle = osThreadNew(LogTestTask, NULL, &LogTestTaskHand_attributes);


    const osThreadAttr_t DebugTaskHandle_attributes = {
    .name = "DebugTaskHandle",
    .stack_size = 128 * 4,
    .priority = (osPriority_t) osPriorityLow,
    };
    DebugTaskHandleHandle = osThreadNew(DebugTask, NULL, &DebugTaskHandle_attributes);

}

/**
 * @brief LogTest 任务函数
 *        测试打印日志的效果
 * 
 */
__attribute((noreturn)) void LogTestTask(void *argument)
{
    static int test_num = 0;
    LOGINFO("LogTask is ready!\n");
    for(;;)
    {
        if(test_num)
        {
            LOGWARNING("something maybe error! test_num is %d\n",test_num);
            test_num = 0;
        }
        if(!test_num)
        {
            LOGERROR("something must be error! test_num is %d\n",test_num);
            test_num = 1;
        }
        osDelay(5);   
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
        
        osDelay(1);
    }
}


