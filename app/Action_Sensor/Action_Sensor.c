/**
 * @file Action_Sensor.c
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
#include "Action_Sensor.h"

osThreadId_t Action_SensorTaskHandle;

/* action所用uart实例声明，以便被外部调用 */
Uart_Instance_t *action_uart_instance = NULL;
uint8_t rx_buffer[30];// 配置用来装储存数据的buffer
Action_Instance_t *action_instance = NULL;


/**
 * @brief 测试调试的函数任务
 * 
 */
__attribute((noreturn)) void Action_SensorTask(void *argument)
{
    static float Action_Sensor_start;
    static float Action_Sensor_dt;
    static char sAction_Sensor_dt[20];
    /* 串口实例注册 */
    // uint8_t rx_buffer[30];// 配置用来装储存数据的buffer
    uart_package_t action_package = {
        .uart_handle = &huart4,
        .rx_buffer = rx_buffer,
        .rx_buffer_size = ACTION_DATA_NUM,
        .uart_callback = Action_RxCallback_Fun,
    };// 配置uart包
    action_uart_instance = Uart_Register(&action_package, 10, sizeof(UART_TxMsg));
    if(action_uart_instance == NULL)
    {
        /* 如果action设备创建失败，就删除本task，防止占cpu */
        LOGWARNING("uart register failed!");
        vTaskDelete(NULL);
    }
    /* 看门狗注册流程 */
    iwdg_config_t action_iwdg_config = {
        .reload_count = 1000,
        .init_count = 10000,
        .callback = action_iwdg_callback,
    };
    IWDG_Instance_t *action_iwdg_instance = NULL;
    action_iwdg_instance = IWDG_Register(&action_iwdg_config);
    if(action_iwdg_instance == NULL)
    {
        /* 如果action设备创建失败，就删除本task，防止占cpu */
        LOGWARNING("iwdg register failed!");
        vTaskDelete(NULL);
    }
    /* action设备注册流程 */
    action_instance = Action_Init(action_uart_instance,action_iwdg_instance);    
    if(action_instance == NULL)
    {
        /* 如果action设备创建失败，就删除本task，防止占cpu */
        LOGWARNING("action device init failed!");
        vTaskDelete(NULL);
    }
    /* 创建实例完毕，开始进入接收task */
    action_instance->action_iwdg_instance->fall_asleep(action_instance->action_iwdg_instance);

    for(;;)
    {
        Action_Sensor_start = DWT_GetTimeline_ms();
        LOGINFO("Action_SensorTask is running!");

        /* 调用action设备中读数据的函数,记得要检查实例有无最终挂载成功,才可以调用,其实如果action作为一个设备的话
            完全不需要再做一次判断,因为能进到任务循环内部一般都是实例创建完毕,否则该线程会被直接删除而不可能直接进入这个
            位置,这里是为了保持良好习惯,因为以后可能不会说action独占一个线程,可能会包含其它传感器数据的接收 */
        if(action_instance != NULL)
        {
            action_instance->action_task(action_instance);
        }
        Action_Sensor_dt = DWT_GetTimeline_ms() - Action_Sensor_start;
        Float2Str(sAction_Sensor_dt,Action_Sensor_dt);
        if(Action_Sensor_dt > 1)
        {
            LOGERROR("Action_SensorTask is being DELAY!!! dt= [%s] ms", sAction_Sensor_dt);
        }
        osDelay(5);
    }
}
