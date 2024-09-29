/**
 * @file action.c
 * @author Keten (2863861004@qq.com)
 * @brief 
 * @version 0.1
 * @date 2024-09-27
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention : 使用module层应注意！
 *              在app层实现bsp层接口和module层接口，module只管使用bsp层提供的接口，但是具体需要先进行bsp层接口的配置，才可以进行module层的初始化！

 * @note :
 * @versioninfo :
 */
#include "action.h"
#include "bsp_usart.h"

union 
{
    uint8_t posture[24];
    float ActVal[6];
}action_posture;


Action_Instance_t* Action_Init(Uart_Instance_t *action_uart)
{
    if(action_uart == NULL)
    {
        LOGERROR("action uart is not prepared!");
        return NULL;
    }
    Action_Instance_t *temp_action_instance = (Action_Instance_t*)malloc(sizeof(Action_Instance_t));
    if(temp_action_instance == NULL)
    {
        /* 如果malloc失败了 */
        LOGERROR("action malloc failed!");
        return NULL;
    }
    /* 挂载外部接口 */
    if(action_uart->rtos_for_uart ==NULL || action_uart->uart_package == NULL)
    {
        LOGERROR("action_uart have somethong is not vaild!");
        free(temp_action_instance);
        temp_action_instance = NULL;
        return NULL;
    }
    temp_action_instance->action_uart_instance = action_uart;

    /* 挂载内部接口 */
    temp_action_instance->action_orin_data = (action_original_info*)malloc(sizeof(action_original_info));
    if(temp_action_instance->action_orin_data == NULL)
    {
        LOGERROR("action_orin_data malloc failed!");
        free(temp_action_instance);
        temp_action_instance = NULL;
        return NULL;
    }
    memset(temp_action_instance->action_orin_data,0,sizeof(action_original_info));

    temp_action_instance->action_diff_data = (robot_info_from_action*)malloc(sizeof(robot_info_from_action));
    if(temp_action_instance->action_diff_data == NULL)
    {
        LOGERROR("action_diff_data malloc failed!");
        free(temp_action_instance);
        temp_action_instance = NULL;
        return NULL;
    }
    memset(temp_action_instance->action_diff_data,0,sizeof(robot_info_from_action));
    /* 挂载函数指针 */
    temp_action_instance->action_get_data = Action_GetData;
    temp_action_instance->action_refresh_data = Action_RefreshData;
    temp_action_instance->action_task = Action_Task;
    LOGINFO("action is prepared!");
    return temp_action_instance;
}


uint8_t Action_Task(void* action_instance)
{
    UART_TxMsg Msg;
    Action_Instance_t *temp_action_instance = action_instance;
    if(temp_action_instance->action_uart_instance->rtos_for_uart->queue_receive(temp_action_instance->action_uart_instance->rtos_for_uart->xQueue,&Msg,0) == pdPASS)
    {
        LOGINFO("action task is running!");
        temp_action_instance->action_get_data(Msg.data_addr,temp_action_instance->action_orin_data,temp_action_instance->action_diff_data);
        return 1;
    }
    LOGERROR("action task is not running!");
    return 0;
}

uint8_t Action_GetData(uint8_t *data,action_original_info *action_data,robot_info_from_action *info_from_action)
{
    if(data == NULL)
    {
        LOGWARNING("action data is NULL!");
        return 0;
    }
    /* 检查包头 */
    if(data[0] == 0x0d && data[1] == 0x0a)
    {
        // 没有操作
    }
    else
    {
        LOGWARNING("action_uart head error!");
        return 0;
    }
    
    /* 检查包尾 */
    if(data[26] == 0x0a && data[27] == 0x0d)
    {
        // 没有操作
    }
    else
    {
        LOGWARNING("action_uart tail error!");
        return 0;
    }

    for(int i = 0; i < 24; i++)
    {
        action_posture.posture[i] = data[i+2];
    }

#ifdef USE_DIFF_GET_DATA
    /* 储存上一次的值 */
    action_data->LAST_POS_X = action_data->POS_X;
    action_data->LAST_POS_Y = action_data->POS_Y;
    action_data->LAST_YAW = action_data->ANGLE_Z;
#endif
    action_data->ANGLE_Z = action_posture.ActVal[0];// 角度 -180~180
    action_data->ANGLE_X = action_posture.ActVal[1];
    action_data->ANGLE_Y = action_posture.ActVal[2];
    action_data->POS_X = action_posture.ActVal[3];
    action_data->POS_Y = action_posture.ActVal[4];
    action_data->W_Z = action_posture.ActVal[5];// 角速度
    // action_data->W_Z = 666;// 角速度


#ifdef USE_DIFF_GET_DATA
    /* 差分运算 */
    action_data->DELTA_POS_X = action_data->POS_X - action_data->LAST_POS_X;
    action_data->DELTA_POS_Y = action_data->POS_Y - action_data->LAST_POS_Y;
    action_data->DELTA_YAW = action_data->ANGLE_Z - action_data->LAST_YAW;

    /* 累加得出最终真实位置 */
    action_data->REAL_X += (-action_data->DELTA_POS_X);
    action_data->REAL_Y += (-action_data->DELTA_POS_Y);
    action_data->REAL_YAW += (-action_data->DELTA_YAW); 

    /* 在这里可以根据action实际安装位置进行坐标向底盘中心的转换,这里提供的是给出action位于底盘中心时的数据赋值 */
    info_from_action->x = action_data->REAL_X;
    info_from_action->y = action_data->REAL_Y;
    info_from_action->yaw = action_data->REAL_YAW;
    info_from_action->yaw_rate = action_data->W_Z*PI/180;

#endif
    LOGINFO("receive data!");
    memset(data,0,28);
    return 1;
}


uint8_t Action_RxCallback_Fun(Uart_Instance_t *action_instance, uint16_t data_len)
{
    UART_TxMsg Msg;
    if(action_instance->rtos_for_uart->xQueue !=NULL)
    {
        Msg.data_addr = action_instance->uart_package->rx_buffer;
        Msg.len = data_len;
        Msg.huart = action_instance->uart_package->uart_handle;
        if(Msg.data_addr != NULL)
            action_instance->rtos_for_uart->queue_send(action_instance->rtos_for_uart->xQueue,&Msg,NULL);
        return 1;
    }
    return 0;
}

uint8_t Action_RefreshData(void)
{
    return 1;
}

