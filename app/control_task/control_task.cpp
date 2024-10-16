/**
 * @file control_task.cpp
 * @author Keten (2863861004@qq.com)
 * @brief 
 * @version 0.1
 * @date 2024-10-04
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention :
 * @note :
 * @versioninfo :
 */
#include "control_task.h"


osThreadId_t Control_TaskHandle;


__attribute((noreturn)) void Control_Task(void *argument)
{
    /* 机器人控制接口，这里选用航模遥控 */
    
    /* 航模遥控 */
    GPIO_Instance_t *gpio_instance = GPIO_Pin_Register(GPIOA, GPIO_PIN_1);
    if(gpio_instance == NULL)
    {
        LOGERROR("gpio instance create failed!");
        vTaskDelete(NULL);
    }
    Air_Joy_Init(gpio_instance,TRAPEZOIDAL);
    for(;;)
    {
        Air_Joy_Process();
        osDelay(2);
    }
}
