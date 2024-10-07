/**
 * @file data_type.h
 * @author Keten (2863861004@qq.com)
 * @brief 模块所依赖的数据类型结构体，有些模块会依赖这些数据类型结构体进行数据传输，所以移植module层都
 *        必须携带这个包
 * @version 0.1
 * @date 2024-10-03
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention :
 * @note :
 * @versioninfo :
 */
#pragma once
#include "usart.h"
#include "can.h"

#pragma pack(1)

/**
 * @brief 串口数据传输结构体封装
 * 
 */
typedef struct 
{
    UART_HandleTypeDef *huart;//串口句柄 
    uint16_t len;// 数据长度
    void* data_addr;// 数据地址，使用时把地址赋值给这个指针，数值强转为uint8_t
}UART_TxMsg;


/**
 * @brief can数据传输结构体封装
 * 
 */
typedef struct CAN_TxMsg
{
    uint8_t data[8];//数据数组，8位
    uint32_t id;	//ID
    uint8_t len;	//数据长度
}CAN_TxMsg;


#pragma pack()