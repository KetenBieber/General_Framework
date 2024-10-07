/**
 * @file com_config.cpp
 * @author Keten (2863861004@qq.com)
 * @brief 
 * @version 0.1
 * @date 2024-10-05
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention :
 * @note :
 * @versioninfo :
 */
#include "com_config.h"

extern Motor_C610 m2006;

osThreadId_t CAN1_Send_TaskHandle;

QueueHandle_t CAN1_TxPort;
QueueHandle_t CAN2_TxPort;

uint8_t Common_Service_Init()
{
    CAN1_TxPort = xQueueCreate(16,sizeof(CAN_Tx_Instance_t));
    CAN2_TxPort = xQueueCreate(16,sizeof(CAN_Tx_Instance_t));
    return 1;
}


void CAN1_Rx_Callback(CAN_Rx_Instance_t *can_instance)
{
    if(can_instance->RxHeader.IDE == CAN_ID_STD)
    {
        switch(can_instance->RxHeader.StdId)
        {
            case 0x201:
            {
                m2006.update(can_instance->can_rx_buff);
                break;
            }
        }
    }
    if(can_instance->RxHeader.IDE == CAN_ID_EXT)
    {
        switch(can_instance->RxHeader.ExtId)
        {
            case 0x201:
            {
                m2006.update(can_instance->can_rx_buff);
                break;
            }
        }
    }
}


void CAN2_Rx_Callback(CAN_Rx_Instance_t *can_instance)
{
    if(can_instance->RxHeader.IDE == CAN_ID_STD)
    {
        switch(can_instance->RxHeader.StdId)
        {
            case 0x201:
            {
                m2006.update(can_instance->can_rx_buff);
                break;
            }
        }
    }
}


__attribute((noreturn)) void CAN1_Send_Task(void *argument)
{
    CAN_Tx_Instance_t temp_can_txmsg;
    uint8_t free_can_mailbox;
    for(;;)
    {
        if(xQueueReceive(CAN1_TxPort,&temp_can_txmsg,0) == pdTRUE)
        {
            do{
                free_can_mailbox = HAL_CAN_GetTxMailboxesFreeLevel(&hcan1);
            }while(free_can_mailbox == 0);
            if(temp_can_txmsg.isExTid == 1)// 发送扩展帧
                CAN_Transmit_ExtId(&temp_can_txmsg);
            else    // 发送标准帧
                // CAN_Transmit_StdID(&hcan1,0x200,temp_can_txmsg.can_tx_buff,temp_can_txmsg.tx_len);
                CAN_Transmit_StdID(&temp_can_txmsg);
        }
        osDelay(1);
    }
}


