/**
 * @file com_config.h
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
#pragma once

#ifdef __cplusplus
extern "C"{
#endif

/*----------------------------------include-----------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "cmsis_os.h"
#include "bsp_can.h"

/*------------------------------------extern------------------------------------*/

extern QueueHandle_t CAN1_TxPort;
extern QueueHandle_t CAN2_TxPort;

/*-----------------------------------macro------------------------------------*/
/* 注意这里的can queue size取决于你的can总线负载，如果发送跟不上数据的装载的话，需要适当增加这里的队列大小 */
#define CAN1_TX_QUEUE_SIZE      8
#define CAN2_TX_QUEUE_SIZE      8

/*----------------------------------function----------------------------------*/

/**
 * @brief 
 * 
 * @return uint8_t 
 */
uint8_t Common_Service_Init();


/**
 * @brief 
 * 
 * @param can_instance 
 */
void CAN1_Rx_Callback(CAN_Rx_Instance_t *can_instance);


/**
 * @brief 
 * 
 * @param can_instance 
 */
void CAN2_Rx_Callback(CAN_Rx_Instance_t *can_instance);


/**
 * @brief 
 * 
 * @param argument 
 */
void CAN1_Send_Task(void *argument);


#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#include "chassis_task.h"

#endif



