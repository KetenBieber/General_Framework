/**
 * @file chassis_task.cpp
 * @author Keten (2863861004@qq.com)
 * @brief 底盘任务 在这里完成底盘所有机构的初始化
 *        注意大疆电机作为一款同时具有发送和接收的can设备，较其他can设备来说更加特别的在于，发送电机控制帧时
 *        可以在一帧can中装载4个电机的控制量，也就是说控制4个电机无需发送4帧can，只需一帧就可以完成
 *        VESC电调的can发送帧一帧是只能包括一个电机的控制量，所以需要发送4帧才能控制4个电机
 * @version 0.1
 * @date 2024-10-04
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention :
 * @note :
 * @versioninfo :
 */
#include "chassis_task.h"
#include "bsp_can.h"

/* 即使是大疆电机，也最好做好定义,统一can设备使用方式 */
CCMRAM CAN_Tx_Instance_t m2006_1_tx_instance = {
    .can_handle = &hcan1,
    .isExTid = 0,
    .tx_mailbox = 0,
    .tx_id = 0x200,// 1~4 的电机为 0x200
    .tx_len = 8,// 一个电机发送控制帧只需占2个数据位
    .can_tx_buff = {0},
};
CCMRAM CAN_Rx_Instance_t m2006_1_rx_instance = {
    .can_handle = &hcan1,
    .RxHeader = {0},
    .rx_len = 8,
    .rx_id = 0x201,
    .object_para = CanFilter_0|CanFifo_0|Can_STDID|Can_DataType,
    .mask_id = 0,
    .can_rx_buff = {0},
};

CCMRAM Motor_C610 m2006(1,m2006_1_rx_instance,m2006_1_tx_instance);


uint8_t Chassis_Init()
{
    CAN_Add_Filter(&hcan1,&m2006_1_rx_instance);
    CAN_Add_Filter(&hcan2,&m2006_1_rx_instance);
    LOGINFO("can filter init success!");
    return 1;
}
