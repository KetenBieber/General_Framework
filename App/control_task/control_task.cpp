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
Uart_Instance_t *xbox_uart_instance = NULL;
extern uart_package_t xbox_uart_package;

__attribute((noreturn)) void Control_Task(void *argument)
{
    /* 机器人控制接口，这里选用航模遥控 */
#ifdef AIR_JOY


    /* 航模遥控 */
    GPIO_Instance_t *gpio_instance = GPIO_Pin_Register(GPIOA, GPIO_PIN_1);
    if(gpio_instance == NULL)
    {
        LOGERROR("air_joy gpio instance create failed!");
        vTaskDelete(NULL);
    }
    Air_Joy_Init(gpio_instance,NORMAL);
    
    for(;;)
    {
        Air_Joy_Process();
        osDelay(2);
    }
#elif XBOX_CONTROL
    xbox_uart_instance = Uart_Register(&xbox_uart_package);
    if(xbox_uart_instance == NULL)
    {
        LOGERROR("xbox uart_instance is not prepared!");
        vTaskDelete(NULL);
    }
    if(Xbox_Init(xbox_uart_instance)==0)
    {
        LOGERROR("xbox init failed!");
    }
    
    vTaskDelete(NULL);//xbox初始化完这个任务就能删了
    
#else
    LOGERROR("NO CONTROL METHOD SELECTED!");
    vTaskDelete(NULL);
#endif
}
