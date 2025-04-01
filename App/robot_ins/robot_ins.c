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
 * @note :  2025-1-31 是时候复活了，ins任务，需要完成状态观测器的工作，底盘任务将会从这里去获取机器人的坐标
 *                    然后，底盘实例不需要订阅机器人坐标，只需要订阅底盘速度即可
 *                    哪里需要订阅坐标呢？---路径跟踪器
 *                    何为观测器？ ---现在只是差分坐标得到速度数据，后面会加入数据融合的部分！
 *                  
 *                    将速度发布到 chassis_spe_pub    
 * @versioninfo :
 */
#include "robot_ins.h"

osThreadId_t ins_TaskHandle;

/* action所用uart实例声明，以便被外部调用 */
uint8_t rx_buffer[28];// 配置用来装储存数据的buffer

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
 * @brief 当前提供的方法为简单的根据action传感器差分得到速度
 * 
 * @param now_pos_x 
 * @param now_pos_y 
 * @param now_yaw_angle 
 * @param last_pos_x 
 * @param last_pos_y 
 * @param last_yaw_angle 
 * @param dt 
 * @param robo_vx 
 * @param robo_vy 
 * @param robo_omega 
 * @param world_vx 
 * @param world_vy 
 * @param world_omega 
 */
void ins_spe_get_easyily(float *now_pos_x,float *now_pos_y,float *now_yaw_angle,
                 float *last_pos_x,float *last_pos_y,
                 float dt,
                 float *robo_vx,float *robo_vy,
                 float *world_vx,float *world_vy)
{
    *world_vx = (*now_pos_x - *last_pos_x)/dt;
    *world_vy = (*now_pos_y - *last_pos_y)/dt;

    float COSANGLE = arm_cos_f32(*now_yaw_angle * DEGREE_2_RAD);
    float SINANGLE = arm_sin_f32(*now_yaw_angle * DEGREE_2_RAD);

    *robo_vx = *world_vx * COSANGLE - *world_vy * SINANGLE;
    *robo_vx = *world_vx * SINANGLE + *world_vy * COSANGLE;
    // *robo_omega由action直接读

    *last_pos_x = *now_pos_x;*last_pos_y = *now_pos_y;
}

/**
 * @brief 机器人姿态获取函数
 * 
 */
__attribute((noreturn)) void ins_Task(void *argument)
{
    portTickType currentTime;
    currentTime = xTaskGetTickCount();

    /* 串口实例注册 */
    uart_package_t action_package = {
        .uart_handle = &huart4,
        .use_static_length_data = 1,
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
        .reload_count = 5000,// 设置重载值为1000，t=1000*看门狗线程周期
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
    action_instance = Action_Init(action_uart_instance,action_iwdg_instance,20);// 设定action队列长度为10    
    if(action_instance == NULL)
    {
        /* 如果action设备创建失败，就删除本task，防止占cpu */
        LOGWARNING("action device init failed!");
        vTaskDelete(NULL);
    }
    /* 创建实例完毕，开始进入接收task */
    
    action_instance->action_iwdg_instance->fall_asleep(action_instance->action_iwdg_instance);
    

    float last_pos_x = 0,last_pos_y = 0,last_yaw_angle = 0;
    // /* 发布订阅准备 */
    publish_data p_chassis_imu_data;// 发布姿态角数据
    publish_data p_chassis_pos_data;// 发布坐标数据
    publish_data p_chassis_spe_data;// 发布机器人速度数据

    ins_interface.imu_data = (pub_imu_yaw*)pvPortMalloc(sizeof(pub_imu_yaw));
    assert_param(ins_interface.imu_data != NULL);
    ins_interface.pos_data = (pub_Chassis_Pos*)pvPortMalloc(sizeof(pub_Chassis_Pos));
    assert_param(ins_interface.pos_data != NULL);
    ins_interface.spe_data = (pub_chassis_spe*)pvPortMalloc(sizeof(pub_chassis_spe));
    assert_param(ins_interface.spe_data != NULL);
    
    ins_interface.chassis_imu_pub = register_pub("chassis_imu_pub");
    ins_interface.chassis_pos_pub = register_pub("chassis_pos_pub");
    ins_interface.chassis_spe_pub = register_pub("chassis_spe_pub");
    for(;;)
    {
        LOGINFO("Action_SensorTask is running!");

        /* 调用action设备中读数据的函数,记得要检查实例有无最终挂载成功,才可以调用,其实如果action作为一个设备的话
            完全不需要再做一次判断,因为能进到任务循环内部一般都是实例创建完毕,否则该线程会被直接删除而不可能直接进入这个
            位置,这里是为了保持良好习惯,因为以后可能不会说action独占一个线程,可能会包含其它传感器数据的接收 */
        if(action_instance != NULL)
        {
            ins_interface.dt = DWT_GetDeltaT(&ins_interface.DWT_CNT);
            action_instance->action_task(action_instance);

            ins_interface.imu_data->yaw = action_instance->action_diff_data->yaw;
            p_chassis_imu_data.data = (uint8_t *)(ins_interface.imu_data);
            p_chassis_imu_data.len = sizeof(pub_imu_yaw);
            ins_interface.chassis_imu_pub->publish(ins_interface.chassis_imu_pub,p_chassis_imu_data);

            ins_interface.pos_data->x = action_instance->action_diff_data->x;
            ins_interface.pos_data->y = action_instance->action_diff_data->y;
            ins_interface.pos_data->yaw = action_instance->action_diff_data->yaw;
            p_chassis_pos_data.data = (uint8_t *)(ins_interface.pos_data);  
            p_chassis_pos_data.len = sizeof(pub_Chassis_Pos);
            ins_interface.chassis_pos_pub->publish(ins_interface.chassis_pos_pub,p_chassis_pos_data);
            
            ins_spe_get_easyily(&ins_interface.pos_data->x,&ins_interface.pos_data->y,&ins_interface.pos_data->yaw,
                                &last_pos_x,&last_pos_y,
                                ins_interface.dt,
                                &ins_interface.spe_data->robo_vx,&ins_interface.spe_data->robo_vy,
                                &ins_interface.spe_data->world_vx,&ins_interface.spe_data->world_vy);
            ins_interface.spe_data->robo_omega = action_instance->action_diff_data->yaw_rate;
            ins_interface.spe_data->world_omega = action_instance->action_diff_data->yaw_rate;

            p_chassis_spe_data.data = (uint8_t *)(ins_interface.spe_data);
            p_chassis_spe_data.len = sizeof(pub_chassis_spe);
            ins_interface.chassis_spe_pub->publish(ins_interface.chassis_spe_pub,p_chassis_spe_data);
        }
        vTaskDelayUntil(&currentTime,1);
    }
}


