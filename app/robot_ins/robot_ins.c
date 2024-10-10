/**
 * @file robot_ins.c
 * @author Keten (2863861004@qq.com)
 * @brief 机器人获取姿态函数，需自己去实现挂载获取姿态的传感器
 * @version 0.1
 * @date 2024-09-28
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention :
 * @note :
 * @versioninfo :
 */
#include "robot_ins.h"

osThreadId_t ins_TaskHandle;

/* action所用uart实例声明，以便被外部调用 */
uint8_t rx_buffer[30];// 配置用来装储存数据的buffer

Uart_Instance_t *action_uart_instance = NULL;
Action_Instance_t *action_instance = NULL;

Robot_Ins_t ins_interface;


/**
 * @brief action串口设备监护线程回调函数，如果action模块离线之后会自动调用
 * 
 * @param instance 
 * @return uint8_t 
 */
uint8_t action_iwdg_callback(void *instance)
{
    if(instance == NULL)
    {
        return 0;
    }
    Action_Instance_t *temp_action_instance = (Action_Instance_t*)instance;  
    LOGINFO("action has not response!");
    if (temp_action_instance == NULL)
    {
        LOGERROR("temp_action_instance is NULL!");
        return 0;
    }

    if (temp_action_instance->action_deinit == NULL)
    {
        LOGERROR("action_deinit function pointer is NULL!");
        return 0;
    }    
    temp_action_instance->action_deinit(temp_action_instance);
    if(ins_TaskHandle != NULL)
    {
        if(xTaskAbortDelay(ins_TaskHandle) == pdPASS)
        {
            LOGINFO("action task is abort!");
            vTaskDelete(ins_TaskHandle);
            ins_TaskHandle = NULL;
        }
        else
        {
            LOGERROR("action task is not abort!");
        }
    }
    return 1;
}



/**
 * @brief 机器人姿态获取函数
 * 
 */
__attribute((noreturn)) void ins_Task(void *argument)
{
    /* 时间监测 */
    static float ins_start;
    static float ins_dt;

    static char s_ins_dt[20];
    /* 串口实例注册 */
    uart_package_t action_package = {
        .uart_handle = &huart4,
        .rx_buffer = rx_buffer,
        .rx_buffer_size = ACTION_DATA_NUM,
        .uart_callback = Action_RxCallback_Fun,
    };// 配置uart包
    action_uart_instance = Uart_Register(&action_package);
    if(action_uart_instance == NULL)
    {
        /* 如果action设备创建失败，就删除本task，防止占cpu */
        LOGWARNING("uart register failed!");
        vTaskDelete(NULL);
    }

    /* 看门狗注册流程 */
    iwdg_config_t action_iwdg_config = {
        .reload_count = 1000,// 设置重载值为1000，t=1000*看门狗线程周期
        .init_count = 9000,// 设置action设备初始化所需的时间 t=10000*1ms
        .callback = action_iwdg_callback,// 设置action看门狗意外触发函数，需要用户提供
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
    action_instance = Action_Init(action_uart_instance,action_iwdg_instance,10);// 设定action队列长度为10    
    if(action_instance == NULL)
    {
        /* 如果action设备创建失败，就删除本task，防止占cpu */
        LOGWARNING("action device init failed!");
        vTaskDelete(NULL);
    }
    /* 创建实例完毕，开始进入接收task */
    
    action_instance->action_iwdg_instance->fall_asleep(action_instance->action_iwdg_instance);
    
    /* 发布订阅准备 */
    ins_interface.chassis_imu_pub = register_pub("chassis_imu_pub");
    ins_interface.chassis_pos_pub = register_pub("chassis_pos_pub");
    publish_data chassis_imu_data;
    publish_data chassis_pos_data;
    for(;;)
    {
        ins_start = DWT_GetTimeline_ms();
        LOGINFO("Action_SensorTask is running!");

        /* 调用action设备中读数据的函数,记得要检查实例有无最终挂载成功,才可以调用,其实如果action作为一个设备的话
            完全不需要再做一次判断,因为能进到任务循环内部一般都是实例创建完毕,否则该线程会被直接删除而不可能直接进入这个
            位置,这里是为了保持良好习惯,因为以后可能不会说action独占一个线程,可能会包含其它传感器数据的接收 */
        if(action_instance != NULL)
        {
            action_instance->action_task(action_instance);
            // ins_interface.imu_data->yaw = action_instance->action_diff_data->yaw;
            // ins_interface.pos_data->x = action_instance->action_diff_data->x;
            // ins_interface.pos_data->y = action_instance->action_diff_data->y;
            // ins_interface.pos_data->yaw = action_instance->action_diff_data->yaw;
            // chassis_imu_data.data = (uint8_t *)&(ins_interface.imu_data);
            // chassis_imu_data.len = sizeof(pub_imu_yaw);
            // ins_interface.chassis_imu_pub->publish(ins_interface.chassis_imu_pub,chassis_imu_data);
            // chassis_pos_data.data = (uint8_t *)&(ins_interface.pos_data);  
            // chassis_pos_data.len = sizeof(pub_Chassis_Pos);
            // ins_interface.chassis_pos_pub->publish(ins_interface.chassis_pos_pub,chassis_pos_data);
        }
        ins_dt = DWT_GetTimeline_ms() - ins_start;
        Float2Str(s_ins_dt,ins_dt);
        if(ins_dt > 1)
        {
            LOGERROR("Action_SensorTask is being DELAY!!! dt= [%s] ms", s_ins_dt);
        }
        osDelay(5);
    }
}


