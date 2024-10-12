/**
 * @file motor_interface.h
 * @author Keten (2863861004@qq.com)
 * @brief 电机控制接口，提供两个template函数,这里主要和app层和rtos层有一定耦合，所以独立出来写，电机发送函数需要和具体使用情况挂钩
 *        可能就只使用了can1 或者 can2，但是统一了can总线发送queue，所有需要在can总线上发送的信息，都由CAN1_TxPort 和 CAN2_TxPort
 *        队列缓存并发送，使用xQueueSend发送信息到can发送线程那边进行处理
 *        并且统一了can包，全部使用bsp层同样的定义
 *        如有其他同样的can发射设备，同样是使用同样的can发送queue进行发送！
 * @version 0.1
 * @date 2024-10-07
 * 
 * @copyright Copyright (c) 2024 
 * 
 * @attention :
 * @note :
 * @versioninfo :
 */
#ifndef MOTOR_INTERFACE_H 
#define MOTOR_INTERFACE_H 

#ifdef __cplusplus
extern "C" {
#endif
#include "FreeRTOS.h"
#include "queue.h"
#include "com_config.h"

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

/*----------------------------------include-----------------------------------*/
#include "motor_base.h"
#include "rm_motor.h"
#include "bsp_log.h"
/*----------------------------------template----------------------------------*/
/**
 * @brief 电机can发送函数，用于多个电机的发送（传多电机对象的数组为参数）
 * 
 * @tparam Motor_Type  电机类
 * @tparam N           电机数量
 * @param hcan         CAN句柄
 */
template <class Motor_Type, int N>
void Motor_SendMsgs(Motor_Type (&motor)[N])
{
    CAN_Tx_Instance_t can_tx_instance;
    for(int i=0; i<N; i++)
    {
        motor[i].CanMsg_Process(can_tx_instance);
        LOGINFO("data is ready!");
    }

    if(can_tx_instance.can_handle == &hcan1)
        xQueueSend(CAN1_TxPort, &can_tx_instance, portMAX_DELAY);
    else if(can_tx_instance.can_handle == &hcan2)
        xQueueSend(CAN2_TxPort, &can_tx_instance, portMAX_DELAY);
}

/**
 * @brief 电机can发送函数，用于单个电机的发送（单电机对象）
 * 
 * @tparam Motor_Type 电机类
 * @param hcan        CAN句柄
 * @param motor       电机
 */
// template <class Motor_Type>
// void Motor_SendMsgs(Motor_Type &motor)
// {
//     Motor_Type motor_arr[1] = {motor};
//     Motor_SendMsgs(motor_arr);
// }

#endif

#endif	/* MOTOR_INTERFACE_H */

