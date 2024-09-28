/**
 * @file debug.c
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
#include "debug.h"

/* 创建任务句柄 */
osThreadId_t DebugTaskHandle;

/* action所用uart实例声明，以便被外部调用 */
Uart_Instance_t *action_uart_instance = NULL;
uint8_t rx_buffer[30];// 配置用来装储存数据的buffer
Action_Instance_t *action_instance = NULL;

/**
 * @brief 测试调试的函数任务
 * 
 */
__attribute((noreturn)) void DebugTask(void *argument)
{
    static float Debug_start;
    static float Debug_dt;
    static char sDebug_dt[20];
    /* 串口实例注册 */
    // uint8_t rx_buffer[30];// 配置用来装储存数据的buffer
    uart_package_t action_package = {
        .uart_handle = &huart4,
        .rx_buffer = rx_buffer,
        .rx_buffer_size = ACTION_DATA_NUM,
        .uart_callback = Action_RxCallback_Fun,
    };// 配置uart包
    action_uart_instance = (Uart_Instance_t*)malloc(sizeof(Uart_Instance_t));
    if(action_uart_instance == NULL)
    {
        /* 如果串口设备注册失败，就删除本task，防止占cpu */
        LOGWARNING("uart malloc failed!");
        DELETE_THREAD(DebugTaskHandle);
        osDelay(1);
    }
    action_uart_instance = Uart_Register(&action_package, 10, sizeof(UART_TxMsg));
    if(action_uart_instance == NULL)
    {
        /* 如果action设备创建失败，就删除本task，防止占cpu */
        LOGWARNING("uart register failed!");
        DELETE_THREAD(DebugTaskHandle);
        osDelay(1);
    }
    /* action设备注册流程 */
    action_instance = (Action_Instance_t*)malloc(sizeof(Action_Instance_t));
    if(action_instance == NULL)
    {
        LOGWARNING("action device malloc failed!");
        DELETE_THREAD(DebugTaskHandle);
        osDelay(1);
    }
    action_instance->action_init = Action_Init;
    if(action_instance->action_init(action_instance,action_uart_instance) == 0)
    {
        /* 如果action设备创建失败，就删除本task，防止占cpu */
        LOGWARNING("action device init failed!");
        DELETE_THREAD(DebugTaskHandle);
        osDelay(1);
    }
    if(action_instance->action_orin_data == NULL)
    {
        /* 如果action 原始数据结构体 创建失败，就删除本task，防止占cpu */
        LOGWARNING("action action_orin_data init failed!");
        DELETE_THREAD(DebugTaskHandle);
        osDelay(1);
    }
    if(action_instance->action_diff_data == NULL)
    {
        /* 如果action 差分数据结构体 创建失败，就删除本task，防止占cpu */
        LOGWARNING("action action_diff_data init failed!");
        DELETE_THREAD(DebugTaskHandle);
        osDelay(1);
    }
    /* 工作完毕，开始进入接收task */
    for(;;)
    {
        Debug_start = DWT_GetTimeline_ms();
        // LOGINFO("DebugTask is running!");
        /* 调用action设备中读数据的函数 */
        action_instance->action_task(action_instance);
        Debug_dt = DWT_GetTimeline_ms() - Debug_start;
        Float2Str(sDebug_dt,Debug_dt);
        // LOGWARNING("DebugTask last time is %s",sDebug_dt);
        if(Debug_dt > 1)
        {
            // LOGERROR("DebugTask is being DELAY!!! dt= [%s] ms", sDebug_dt);
        }
        osDelay(5);
    }
}
