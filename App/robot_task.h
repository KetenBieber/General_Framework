
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

/* app 层接口头文件 一般是extern了任务函数才会在这里include */
#include "com_config.h"
#include "chassis_task.h"
#include "robot_ins.h"
#include "control_task.h"
#include "debug_task.h"

/* module层接口头文件 */
#include "soft_iwdg.h"
#include "topics.h"


/* Definitions for TaskHand */
extern osThreadId_t ins_TaskHandle;
osThreadId_t IWDGTaskHandle;
extern osThreadId_t CAN1_Send_TaskHandle;
extern osThreadId_t CAN2_Send_TaskHandle;
extern osThreadId_t Debug_TaskHandle;
extern osThreadId_t Chassis_TaskHandle;
extern osThreadId_t Control_TaskHandle;
extern osThreadId_t ROSCOM_TaskHandle;

/* Definitions for TaskFunc */
void ins_Task(void *argument);
void IWDGTask(void *argument);
void CAN1_Send_Task(void *argument);
void CAN2_Send_Task(void *argument);
void Debug_Task(void *argument);
void Chassis_Task(void *argument);
void Control_Task(void *argument);
void ROSCOM_Task(void *argument);

/**
 * @brief os任务创建初始化函数
 * 
 */
void osTaskInit(void)
{

    const osThreadAttr_t ins_TaskHandle_attributes = {
    .name = "ins_TaskHandle",
    .stack_size = 128 * 4,
    .priority = (osPriority_t) osPriorityNormal,
    };
    ins_TaskHandle = osThreadNew(ins_Task, NULL, &ins_TaskHandle_attributes);


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

    const osThreadAttr_t CAN2_SendTaskHandle_attributes = {
    .name = "CAN2_Send_TaskHandle",
    .stack_size = 128*4 ,
    .priority = (osPriority_t) osPriorityNormal,
    };
    CAN2_Send_TaskHandle = osThreadNew(CAN2_Send_Task, NULL, &CAN2_SendTaskHandle_attributes);

#ifdef DEBUG_TASK_RUN 
    const osThreadAttr_t DebugTaskHandle_attributes = {
    .name = "Debug_TaskHandle",
    .stack_size = 128*4 ,
    .priority = (osPriority_t) osPriorityNormal,
    };
    Debug_TaskHandle = osThreadNew(Debug_Task, NULL, &DebugTaskHandle_attributes);
#endif 


#ifdef CHASSIS_TO_DEBUG
    const osThreadAttr_t ChassisTaskHandle_attributes = {
    .name = "Chassis_TaskHandle",
    .stack_size = 128*4 ,
    .priority = (osPriority_t) osPriorityNormal,
    };
    Chassis_TaskHandle = osThreadNew(Chassis_Task, NULL, &ChassisTaskHandle_attributes);
#endif

    const osThreadAttr_t ControlTaskHandle_attributes = {
    .name = "Control_TaskHandle",
    .stack_size = 128*4 ,
    .priority = (osPriority_t) osPriorityNormal,
    };
    Control_TaskHandle = osThreadNew(Control_Task, NULL, &ControlTaskHandle_attributes);

    const osThreadAttr_t ROSCOMTaskHandle_attributes = {
    .name = "ROSCOM_TaskHandle",
    .stack_size = 128*4 ,
    .priority = (osPriority_t) osPriorityNormal,
    };
    Control_TaskHandle = osThreadNew(ROSCOM_Task, NULL, &ROSCOMTaskHandle_attributes);

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


