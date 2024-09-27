/**
 * @file com2ros.h
 * @author Keten (2863861004@qq.com)
 * @brief 
 * @version 0.1
 * @date 2024-09-27
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
/* bsp层头文件接口 */
#include "bsp_usart.h"
#include "bsp_log.h"
/*-----------------------------------macro------------------------------------*/

/*----------------------------------typedef-----------------------------------*/

/*----------------------------------variable----------------------------------*/

/*-------------------------------------os-------------------------------------*/

/*----------------------------------function----------------------------------*/

// 和ros通讯的初始化
uint8_t Com2Ros_Init(void);

// 接收ros信息回调函数
uint8_t Com2Ros_Callback(Uart_Instance_t* uart_device,uint32_t rx_buf_num);
/*------------------------------------test------------------------------------*/

#ifdef __cplusplus
}
#endif
