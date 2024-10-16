/**
 * @file debug.cpp
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
#include "debug_task.h"
#include "topics.h"
#include "bsp_bitband.h"
#include "bsp_usart.h"
#include "vofa.h"
#include "air_joy.h"
// extern Motor_C610 m2006;

osThreadId_t Debug_TaskHandle;

uint8_t debug_buffer[9];

Subscriber *sub_debug;
pub_Control_Data debug_pid;

VOFA_Instance_t *vofa_instance = NULL;
Uart_Instance_t *vofa_uart_instance = NULL;
extern uart_package_t VOFA_uart_package;

__attribute((noreturn)) void Debug_Task(void *argument)
{

    /* vofa设备创建 */
    vofa_uart_instance = Uart_Register(&VOFA_uart_package);
    if(vofa_uart_instance == NULL)
    {
        LOGERROR("vofa uart register failed!");
        vTaskDelete(NULL);
    }
    vofa_instance = VOFA_init(vofa_uart_instance, 10);
    if(vofa_instance == NULL)
    {
        LOGERROR("vofa init failed!");
        vTaskDelete(NULL);
    }

    for(;;)
    {
        vofa_instance->vofa_task(vofa_instance);
        LOGINFO("debug task is running!");
        osDelay(100);
    }
}
