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
 *              貌似有点思维误区了，我需要的只是先把接口引入，然后爽用罢了
 *             创建串口实例时，创建一个串口类型的结构体，把需要的接口提供给这个结构体，然后在初始化串口的时候，把这个结构体传入
 *             bsp层是唯一操作hal库函数的，所以在这里直接引入hal库的头文件
 *              学院应该不会轻易改用rtos，所以代码和rtos 的一些api 耦合
 *              在这里还是采用了scml的写法，直接去修改it.c中的代码，将自己的代码嵌入，移植的时候需要多加注意！
 *              设计的时候同时要考虑module层的调用，如何服务于module层？我这样一个bsp层，module层需要怎么用？
 *              module层需要做的事情：只有把自己的包的信息装填好，就是准备一个uart_package_t，关于hal库的操作需要bsp层操作！
 *                  1.创建一个串口实例和串口数据包，然后需要做数据装填
 *                  2.自己实现串口的回调函数，用于数据解析
 *                  3.调用bsp层提供的初始化函数，初始化串口，完成队列创建以及数据装载和回调函数注册的工作
 *              但其实module层也只是提供各种接口，真正调用还是在app层
 *              所以更多还是在module层做函数指针的封装，到时可以直接向其提供bsp层的接口，bsp层由于需要和hal库和rtos层打交道，比较难封装
 *            如果使用rtos，需要在调用自己实例初始化之后，执行一次rtos于uart的初始化，之后就可以使用rtos的接口了
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

/* rtos层接口头文件 */
#ifdef USE_RTOS_FOR_UART
#include "FreeRTOS.h"
#include "queue.h"
#include "Semphr.h"
#endif

/*-----------------------------------macro------------------------------------*/
#define DEVICE_UART_CNT 8 // 学院设计的外设板至多分配8个串口
#define UART_RXBUFF_LIMIT 256 // 如果协议需要更大的buff,请修改这里
/*----------------------------------typedef-----------------------------------*/
#ifdef USE_RTOS
/* 引入FREERTOS接口 */
/* 其实相当于为module层露出了rtos的接口，只要挂载了对应的rtos api，就可以调用 */
typedef struct
{
    QueueHandle_t xQueue;// 队列句柄
    BaseType_t (*queue_send)(QueueHandle_t xQueue, const void *pvItemToQueue,BaseType_t *pxHigherPriorityTaskWoken);
    BaseType_t (*queue_receive)(QueueHandle_t xQueue,void *pvBuffer,TickType_t xTicksToWait);// 从队列读数据
}rtos_interface_t;
#endif //USE_RTOS

/**
 * @brief 串口回调函数指针
 * 
 * @param uart_device 串口设备实例，记得在回调函数内部定义局部实例获取强转之后的void* 
 * @param rx_buf_num  收到的数据量，这个就是你串口一次中断收到的数据量，可以用于解析
 */
typedef uint8_t (*uart_callback_t)(void* uart_device,uint32_t rx_buf_num);// 定义回调函数类型

/* 串口包数据结构体类型 */
typedef struct
{
    UART_HandleTypeDef *uart_handle;// hal库usart句柄成员
    uint16_t rx_buffer_size;// 定义buffer的大小
    uint8_t *rx_buffer;// 定义一个buffer
    uart_callback_t uart_callback;// 定义回调函数指针
}uart_package_t;

/* uart instance 串口设备实例 */
typedef struct
{
#ifdef USE_RTOS
    /* rtos的接口 */
    rtos_interface_t *rtos_for_uart;
#endif
    /* 串口接收包结构体 */
    uart_package_t *uart_package;
}Uart_Instance_t;

/*----------------------------------function----------------------------------*/

/**
 * @brief 串口设备注册函数，用户通过创建一个实例指针和串口数据包，然后通过调用此函数以及将实例传入本函数来获取返回值的实例
 *        实现串口设备的动态注册，如果创建失败会自动free内存
 * 
 * @param uart_config 
 * @param queue_handler 
 * @param queue_length 
 * @param queue_data 
 * @return Uart_Instance_t* NULL 创建失败
 *                          实例  创建成功
 */
Uart_Instance_t* Uart_Register(uart_package_t *uart_config,void **queue_handler,uint32_t queue_length,size_t queue_data);

/**
 * @brief 串口中断管理函数
 *        这个插入在hal库的it.c代码中，可以强行把hal库对应的串口中断管理转移到这边进行处理
 * 
 * @param uart_instance 
 * @return uint8_t --- 1 :success
 *                 --- 0 :failed
 */
uint8_t Uart_Receive_Handler(Uart_Instance_t *uart_instance);

#ifdef __cplusplus
}
#endif

#endif	/* BSP_USART_H */
