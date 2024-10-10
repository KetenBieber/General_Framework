/**
 * @file bsp_can.c
 * @author Keten (2863861004@qq.com)
 * @brief 
 * @version 0.1
 * @date 2024-09-26
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention :
 * @note :
 * @versioninfo :
 */
#include "bsp_can.h"


static void (*pCAN1_RxCpltCallback)(CAN_Rx_Instance_t *);
static void (*pCAN2_RxCpltCallback)(CAN_Rx_Instance_t *);


uint8_t CAN_Init(CAN_HandleTypeDef* hcan, void (*pFunc)(CAN_Rx_Instance_t*))
{
    assert_param(hcan != NULL);
	
    if (HAL_CAN_Start(hcan) != HAL_OK)
	{
		/* Start Error */
		Error_Handler();
	}

    if (HAL_CAN_ActivateNotification(hcan, CAN_IT_TX_MAILBOX_EMPTY) != HAL_OK)
	{
		/* Start Error */
		Error_Handler();
	}
	
	if (HAL_CAN_ActivateNotification(hcan, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
	{
		/* Start Error */
		Error_Handler();
	}

    if (HAL_CAN_ActivateNotification(hcan, CAN_IT_RX_FIFO1_MSG_PENDING) != HAL_OK)
	{
		/* Start Error */
		Error_Handler();
	}
    

	if(hcan->Instance == CAN1)
	{
		pCAN1_RxCpltCallback = pFunc;
		return SUCCESS;
	}
	else if(hcan->Instance == CAN2)
	{
		pCAN2_RxCpltCallback = pFunc;
		return SUCCESS;
	}
	else
		return ERROR;
}


uint8_t CAN_Add_Filter(CAN_HandleTypeDef *hcan,CAN_Rx_Instance_t *can_rx_instance)
{
    CAN_FilterTypeDef CAN_FilterInit_Instance;

    if(hcan == NULL)
    {
        LOGERROR("add filter init failed!");
        return 0;
    }

    /* 根据用户设置的帧设置不同的过滤方式 */
    if((can_rx_instance->object_para & 0x02))
    {
        /* 对扩展帧进行对应过滤器的设置 */
        CAN_FilterInit_Instance.FilterIdHigh = can_rx_instance->rx_id<<3<<16;                                // 用户设置的接收id 高16bit
        CAN_FilterInit_Instance.FilterIdLow = (can_rx_instance->rx_id<<3) | ((can_rx_instance->object_para & 0x03)<<1);         // 用户设置的接收id 低16bit
        CAN_FilterInit_Instance.FilterMaskIdHigh = can_rx_instance->mask_id<<3<<16;                          // 掩码对高16bit的检查规则
        CAN_FilterInit_Instance.FilterMaskIdLow = (can_rx_instance->mask_id<<3) | ((can_rx_instance->object_para & 0x03)<<1);   // 掩码对低16bit的检查规则
    }
    else
    {
        /* 对标准帧进行对应过滤器的设置 */
        CAN_FilterInit_Instance.FilterIdHigh = can_rx_instance->rx_id<<5;                                    // 用户设置的接收id 高16bit
        CAN_FilterInit_Instance.FilterIdLow = 0;                                            // 用户设置的接收id 低16bit
        CAN_FilterInit_Instance.FilterMaskIdHigh = can_rx_instance->rx_id<<5;                                // 掩码对高16bit的检查规则
        CAN_FilterInit_Instance.FilterMaskIdLow =0;                                         // 掩码对低16bit的检查规则
    }

    CAN_FilterInit_Instance.FilterBank = can_rx_instance->object_para>>3;                                    // 用户设置的Filter 组序号
    CAN_FilterInit_Instance.FilterFIFOAssignment = (can_rx_instance->object_para>>2)&0x01;                   // 用户设置的id 所用FIFO (FIFO1和FIFO0可选)
    CAN_FilterInit_Instance.FilterActivation = ENABLE;                                      // 使能Filter
    CAN_FilterInit_Instance.FilterMode = CAN_FILTERMODE_IDMASK;                             // 设置Filter为掩码模式，通过设置id掩码来进行过滤
    CAN_FilterInit_Instance.FilterScale = CAN_FILTERSCALE_32BIT;                            // 设置Filter为32位模式，可兼容扩展帧和标准帧（高16位）
    CAN_FilterInit_Instance.SlaveStartFilterBank = 14;                                      // 过滤器开始组别，对于单can控制器无意义，所以默认为14

    if(HAL_CAN_ConfigFilter(hcan,&CAN_FilterInit_Instance) != HAL_OK)
    {
        LOGERROR("can filter add failed!");
        return 0;
    }
    return 1;
}


void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    static CAN_Rx_Instance_t temp_can_rx_instance;

    if(hcan == &hcan1)
    {
       if(HAL_CAN_GetRxMessage(hcan,CAN_FILTER_FIFO0,&temp_can_rx_instance.RxHeader,temp_can_rx_instance.can_rx_buff)==HAL_ERROR){};
       pCAN1_RxCpltCallback(&temp_can_rx_instance);
    }
    if(hcan == &hcan2)
    {
       if(HAL_CAN_GetRxMessage(hcan,CAN_FILTER_FIFO0,&temp_can_rx_instance.RxHeader,temp_can_rx_instance.can_rx_buff)==HAL_ERROR){};
	   pCAN2_RxCpltCallback(&temp_can_rx_instance);
    }
}


void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    static CAN_Rx_Instance_t temp_can_rx_instance;

    if(hcan == &hcan1)
    {
       if(HAL_CAN_GetRxMessage(hcan,CAN_FILTER_FIFO1,&temp_can_rx_instance.RxHeader,temp_can_rx_instance.can_rx_buff)==HAL_ERROR){};
       pCAN1_RxCpltCallback(&temp_can_rx_instance);
    }
    if(hcan == &hcan2)
    {
       if(HAL_CAN_GetRxMessage(hcan,CAN_FILTER_FIFO1,&temp_can_rx_instance.RxHeader,temp_can_rx_instance.can_rx_buff)==HAL_ERROR){};
	   pCAN2_RxCpltCallback(&temp_can_rx_instance);
    }
}


void CAN_Transmit_ExtId(CAN_Tx_Instance_t *can_tx_instance)
{
    CAN_TxHeaderTypeDef TxHeader;                                                      // 创建发送句柄
    TxHeader.ExtId = can_tx_instance->tx_id;                                           // 将发送目标的id记录（内部不对扩展帧or标准帧作判断了，需要用户明确调用！）
    TxHeader.IDE = CAN_ID_EXT;                                                         // 设置为发送扩展帧模式
    TxHeader.RTR = CAN_RTR_DATA;                                                       // 发送数据帧
    TxHeader.DLC = can_tx_instance->tx_len;                                            // can协议规定，一包can数据只能含有8位数据帧
    TxHeader.TransmitGlobalTime = DISABLE;                                             // 不发送标记时间

    /* param：hcan, 发送句柄,发送buffer,发送邮箱号 */
    if(HAL_CAN_AddTxMessage(can_tx_instance->can_handle,&TxHeader,can_tx_instance->can_tx_buff,&can_tx_instance->tx_mailbox) != HAL_OK)
    {
        LOGERROR("can EXTID transmit wrong!");
    }
}


void CAN_Transmit_StdID(CAN_Tx_Instance_t *can_tx_instance)
{
    CAN_TxHeaderTypeDef TxHeader;
    uint32_t tx_mailbox = 0;                                                           // 创建发送句柄
    TxHeader.StdId = can_tx_instance->tx_id;                                           // 将发送目标的id记录（内部不对扩展帧or标准帧作判断了，需要用户明确调用！）
    TxHeader.IDE = CAN_ID_STD;                                                         // 设置为发送标准帧模式
    TxHeader.RTR = CAN_RTR_DATA;                                                       // 发送数据帧
    TxHeader.DLC = can_tx_instance->tx_len;                                            // can协议规定，一包can数据只能含有8位数据帧
    TxHeader.TransmitGlobalTime = DISABLE;                                             // 不发送标记时间

    /* param：hcan, 发送句柄,发送buffer,发送邮箱号 */
    if(HAL_CAN_AddTxMessage(can_tx_instance->can_handle,&TxHeader,can_tx_instance->can_tx_buff,&tx_mailbox) != HAL_OK)
    {
        LOGERROR("can STDID transmit wrong!");
    }
}

