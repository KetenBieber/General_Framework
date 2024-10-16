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

typedef struct
{
    float Kp;
    float Ki;
    float Kd;
    float ref;
}pub_vofa_pid;

typedef struct
{
    float linear_x;// x方向速度 m/s
    float linear_y;// y方向速度 m/s
    float Omega;   // 转动速度 rad/s
    uint8_t Status;
}pub_Control_Data;

#pragma pack()