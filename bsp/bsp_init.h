/**
 * @file bsp_init.h
 * @author Keten (2863861004@qq.com)
 * @brief bsp层 初始化文件 只初始化需要用的模块
 * @version 0.1
 * @date 2024-09-14
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention :
 * @note :
 * @versioninfo :
 */
#pragma once

#include "bsp_log.h"
#include "bsp_dwt.h"
#include "bsp_can.h"

void Bsp_Init()
{
    /* stm32f4 芯片主频 168MHz */
    DWT_Init(168);    
    BSPLogInit();
    /* 初始化can */
    CAN_Filter_Init(&hcan1,CanFilter_0|CanFifo_0|Can_STDID|Can_DataType,0,0);
    CAN_Filter_Init(&hcan1,CanFilter_1|CanFifo_1|Can_STDID|Can_DataType,0,0);
}
