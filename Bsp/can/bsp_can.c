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

void CAN_Filter_Init(CAN_HandleTypeDef * hcan, uint8_t object_para,uint32_t Id,uint32_t MaskId) 
{
    CAN_FilterTypeDef  CAN_FilterInitStructure;
	/* Check the parameters */
	assert_param(hcan != NULL);

    /* 根据对应的帧设置不同过滤方式 */
	/* Communication frame */
	if( (object_para&0x02))     /*拓展帧or标准帧*/
	{
        /* 对扩展帧操作 */
        CAN_FilterInitStructure.FilterIdHigh         = Id<<3<<16;                       /* 掩码后ID的高16bit */
        CAN_FilterInitStructure.FilterIdLow          = Id<<3| ((object_para&0x03)<<1);  /* 掩码后ID的低16bit */
        CAN_FilterInitStructure.FilterMaskIdHigh     = MaskId<<3<<16;                   /* ID掩码值高16bit */
        CAN_FilterInitStructure.FilterMaskIdLow      = MaskId<<3| ((object_para&0x03)<<1);;   /* ID掩码值低16bit */
	}
	else/* Other frame */
	{
        /* 对标准帧操作 */
        CAN_FilterInitStructure.FilterIdHigh         = Id<<5;                           /* 掩码后ID的高16bit */
        CAN_FilterInitStructure.FilterIdLow          = ((object_para&0x03)<<1);         /* 掩码后ID的低16bit */
        CAN_FilterInitStructure.FilterMaskIdHigh     = MaskId<<5;                       /* ID掩码值高16bit */
        CAN_FilterInitStructure.FilterMaskIdLow      = ((object_para&0x03)<<1);;        /* ID掩码值低16bit */
	}

    CAN_FilterInitStructure.FilterBank           = object_para>>3;                  /* 滤波器组序号*/
    CAN_FilterInitStructure.FilterFIFOAssignment = (object_para>>2)&0x01;           /* 滤波器绑定FIFO 0 */
    CAN_FilterInitStructure.FilterActivation     = ENABLE;                          /* 使能滤波器 */
    CAN_FilterInitStructure.FilterMode         = CAN_FILTERMODE_IDMASK;             /* 滤波器模式，设置ID掩码模式 */
    CAN_FilterInitStructure.FilterScale        = CAN_FILTERSCALE_32BIT;             /* 32位滤波 */
    CAN_FilterInitStructure.SlaveStartFilterBank = 14;                              /* 过滤器开始组别，单can芯片无意义 */
    
    if(HAL_CAN_ConfigFilter(hcan, &CAN_FilterInitStructure)!=HAL_OK)
    {
		/* Filter configuration Error */
		Error_Handler();
	}

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
    TxHeader.StdId = 0;
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
    TxHeader.ExtId = 0;                                                      // 设置为发送标准帧模式
    TxHeader.IDE = CAN_ID_STD;   
    TxHeader.RTR = CAN_RTR_DATA;                                                       // 发送数据帧
    TxHeader.DLC = can_tx_instance->tx_len;                                            // can协议规定，一包can数据只能含有8位数据帧
    TxHeader.TransmitGlobalTime = DISABLE;                                             // 不发送标记时间

    /* param：hcan, 发送句柄,发送buffer,发送邮箱号 */
    if(HAL_CAN_AddTxMessage(can_tx_instance->can_handle,&TxHeader,can_tx_instance->can_tx_buff,&tx_mailbox) != HAL_OK)
    {
        LOGERROR("can STDID transmit wrong!");
    }
}

