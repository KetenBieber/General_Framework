/**
 * @file bsp_usart.h
 * @author Keten (2863861004@qq.com)
 * @brief 
 * @version 0.1
 * @date 2024-09-24
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention : 如果不是发现底层错误，不要轻易修改bsp层
 *             bsp层在设计之初就是为了解耦，app和module层借助bsp层来实现hal库中的操作
 *              学院应该不会轻易改用rtos，所以代码和 freertos 的一些api 耦合
 *              在这里还是采用了scml的写法，直接去修改it.c中的代码，将自己的代码嵌入，移植的时候需要多加注意！
 *              module层需要做的事情：只有把自己的包的信息装填好，就是准备一个uart_package_t
 *                  1.在初始化接口中做好数据装填，创建一个串口实例；在初始化函数内部会动态创建一个串口实例，如果创建失败的话会自动清除
 *                  2.自己实现串口的回调函数，用于数据解析
 *                  3.调用bsp层提供的初始化函数，初始化串口，完成队列创建以及数据装载和回调函数注册的工作
 *                  4.通过调用结构体中提供的rtos接口来使用rtos api
 *                  5.module层服务于app层，请为app层暴露出你的接口！
 *              app层需要做的事情：只需要调用module层提供的接口，然后在回调函数中实现自己的业务逻辑
 *                  1.创建实例并且extern
 *                  2.调用初始化函数
 *                  3.it.c中包含app层的.h，以便获取串口实例
 *              2024-10-13 添加发送模块，提供几种发送模式：阻塞发送、非阻塞发送、DMA发送
 * 
 *              
 * @note :
 * @versioninfo :
 */
#ifndef BSP_USART_H 
#define BSP_USART_H 

#ifdef __cplusplus
extern "C"{
#endif

/*----------------------------------include-----------------------------------*/
/* c标准库接口 */
#include <stdint.h>

/* bsp层直接操作hal库底层 */
#include "usart.h"

/* freertos接口，用于提供堆管理 */
#include "FreeRTOS.h"
#include "task.h"
/*-----------------------------------macro------------------------------------*/
#define DEVICE_UART_CNT 8 // 学院设计的外设板至多分配8个串口
#define UART_RXBUFF_LIMIT 256 // 如果协议需要更大的buff,请修改这里
/*----------------------------------typedef-----------------------------------*/
/**
 * @brief 串口回调函数定义
 * 
 */
typedef uint8_t (*uart_callback_t)(void* uart_device,uint16_t rx_buf_num);// 定义回调函数类型

/* 串口包数据结构体类型 */
typedef struct
{
    UART_HandleTypeDef *uart_handle;// hal库usart句柄成员
    uint8_t use_static_length_data;// 是否接收定长数据，1：是 0：否
    uint16_t rx_buffer_size;// 定义buffer的大小
    uint8_t *rx_buffer;// 定义一个buffer
    uart_callback_t uart_callback;// 定义回调函数指针
}uart_package_t;

/* uart instance 串口设备实例 */
typedef struct 
{
    /* 串口接收包结构体 */
    uart_package_t uart_package;
    void *device;// 父指针，储存设备指针
    uint8_t (*Uart_Deinit)(void *);// 串口设备注销函数
}Uart_Instance_t;

typedef struct 
{
    UART_HandleTypeDef *uart_handle;// hal库usart句柄成员
    uint8_t *tx_buffer;// 发送缓存
    uint8_t tx_buffer_size;// 发送缓存大小
}Uart_Tx_Package_t;
/*----------------------------------function----------------------------------*/
/**
 * @brief 串口设备注册函数，用户通过创建一个实例指针和串口数据包，然后通过调用此函数以及将实例传入本函数来获取返回值的实例
 *        实现串口设备的动态注册，如果创建失败会自动free内存
 * 
 * @param uart_config     uart_package_t* 串口数据包
 * @param queue_length    uint32_t 队列中所能存储的元素数
 * @param queue_data      size_t 队列元素的大小，使用 sizeof() 获取，注意数据类型是size_t
 * @return Uart_Instance_t* NULL 创建失败
 *                          实例  创建成功
 */
Uart_Instance_t* Uart_Register(uart_package_t *uart_config);


/**
 * @brief 串口中断管理函数
 *        这个插入在hal库的it.c代码中，可以强行把hal库对应的串口中断管理转移到这边进行处理
 * 
 * @param uart_instance 串口设备实例
 * @return uint8_t --- 1 :success
 *                 --- 0 :failed
 */
uint8_t Uart_Receive_Handler(Uart_Instance_t *uart_instance);


/**
 * @brief 串口注销函数
 * 
 * @param uart_instance 
 * @return uint8_t 
 */
uint8_t Uart_UnRegister(void *uart_instance);


/**
 * @brief 阻塞式发送，不推荐使用，会阻塞线程
 * 
 * @param tx_package 
 * @return uint8_t 
 */
uint8_t Uart_Tx_By_Blocking(Uart_Tx_Package_t tx_package);


/**
 * @brief 不阻塞发送，利用中断发送，推荐打开发送中断，做一些操作来显示是否正常发送，当然也可以不开，可有可无的操作
 * 
 * @param tx_package 
 * @return uint8_t 
 */
uint8_t Uart_Tx_By_It(Uart_Tx_Package_t tx_package);


/**
 * @brief 利用DMA进行发送，需要先在cubemx打开配置，推荐使用！
 * 
 * @param tx_package 
 * @return uint8_t 
 */
uint8_t Uart_Tx_By_DMA(Uart_Tx_Package_t tx_package);


#ifdef __cplusplus
}
#endif

#endif	/* BSP_USART_H */
