/**
 * @file bsp_usart.c
 * @author Keten (2863861004@qq.com)
 * @brief 
 * @version 0.1
 * @date 2024-09-24
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention :串口管理方面，用一个 static instance 数组来管理，在每次接收到数据的时候去调用
 *              功能方面，目前暂时先支持DMA空闲中断接收
 * @note :
 * @versioninfo :
 */
#include "bsp_usart.h"
#include "bsp_log.h"
#include "queue.h"
#include <string.h>
#include <stddef.h>
#include <stdlib.h>

/* 由于xQueueSendFromISR为宏定义，不能直接挂载函数指针，所以先用函数包装一下 */
static BaseType_t queue_send_wrapper(QueueHandle_t xQueue, const void *pvItemToQueue, BaseType_t *pxHigherPriorityTaskWoken) {
    return xQueueSendFromISR(xQueue, pvItemToQueue, pxHigherPriorityTaskWoken);
}

/* uart实例数组，所有注册了uart的模块信息会被保存在这里 */
static uint8_t idx = 0;
static Uart_Instance_t *Usart_Device[DEVICE_UART_CNT] = {NULL};

static uint8_t Uart_Rtos_Init(Uart_Instance_t* uart_instance,uint32_t queue_length,size_t queue_data);
static uint8_t Uart_Rx_Idle_Callback(Uart_Instance_t *uart_instance);
static uint8_t Uart_Deinit(Uart_Instance_t **uart_instance);


Uart_Instance_t* Uart_Register(uart_package_t *uart_config,uint32_t queue_length,size_t queue_data)
{   
    if(uart_config == NULL)
    {
        LOGERROR("uart init failed!");
        return NULL;   
    }
    
    if(idx >= DEVICE_UART_CNT)
    {
        /* 注册设备已经太多了！ */
        LOGERROR("uart exceed max instance count!");
        return NULL;
    }
    for(uint8_t i = 0 ; i < idx ; i++)
    {
        if(uart_config->uart_handle == Usart_Device[i]->uart_package->uart_handle)
        {
            LOGINFO("uart instance already registered!");// 如果已经被注册过了，即重复注册
            return NULL;
        }
    }

    // 经过上面的检查，可以进行注册了！
    Uart_Instance_t *uart_instance = (Uart_Instance_t *)pvPortMalloc(sizeof(Uart_Instance_t));// 为uart实例分配内存
    // Uart_Instance_t *uart_instance = (Uart_Instance_t *)malloc(sizeof(Uart_Instance_t));// 为uart实例分配内存

    /* 为什么使用pvPortMalloc，动态创建，这样你在函数内部能管理的东西就更多了 */
    if(uart_instance == NULL)
    {
        LOGERROR("pvPortMalloc failed!");
        return NULL;
    }
    memset(uart_instance,0,sizeof(Uart_Instance_t));// 初始化uart实例

    /* 挂载串口包结构体 */
    memset(uart_config->rx_buffer,0,uart_config->rx_buffer_size);// 清空接收缓存
    uart_instance->uart_package = uart_config;
    uart_instance->Uart_Deinit = Uart_UnRegister;// 挂载注销函数
    /* hal库硬件设置 */
    __HAL_UART_CLEAR_IDLEFLAG(uart_instance->uart_package->uart_handle);// 清除UART的空闲中断标志位
    __HAL_UART_ENABLE_IT(uart_instance->uart_package->uart_handle, UART_IT_IDLE);// 使能UART的空闲中断
    HAL_UART_Receive_DMA(uart_instance->uart_package->uart_handle, uart_instance->uart_package->rx_buffer, uart_instance->uart_package->rx_buffer_size);// 启动DMA接收


    /* rtos层接口初始化挂载 */
    if(Uart_Rtos_Init(uart_instance,queue_length,queue_data) != 1)
    {
        /* 调用vPortFree之后请务必顺便将其置为NULL,避免指针悬空 */
        vPortFree(uart_instance);
        // free(uart_instance);
        uart_instance = NULL;
        return NULL;
    }

    /* 将实例添加到数组中 */
    Usart_Device[idx++] = uart_instance;
    // 注册成功，返回实例
    LOGINFO("uart instance create successfully!");
    return uart_instance;
}

/**
 * @brief rtos对uart的支持初始化函数 static
 *      
 * @param uart_instance 
 * @param queue_length 
 * @param queue_data 
 * @return uint8_t --- 1 :success
 *                 --- 0 :failed
 */
static uint8_t Uart_Rtos_Init(Uart_Instance_t* uart_instance,uint32_t queue_length,size_t queue_data)
{
    if(uart_instance == NULL)
    {
        LOGERROR("uart_rtos init failed!");
        return 0;
    }
    rtos_interface_t *rtos_interface = (rtos_interface_t *)pvPortMalloc(sizeof(rtos_interface_t));
    if(rtos_interface == NULL)
    {
        LOGERROR("rtos_interface pvPortMalloc failed!");
        return 0;
    }
    memset(rtos_interface,0,sizeof(rtos_interface_t));
    rtos_interface->queue_receive = xQueueReceive;
    rtos_interface->queue_send = queue_send_wrapper;

    // 注册获取一个Freertos 队列句柄
    QueueHandle_t queue = xQueueCreate(queue_length,queue_data);
    if(queue == NULL)
    {
        /* 如果队列创建失败 */
        LOGERROR("queue for uart create failed!");
        vPortFree(rtos_interface);
        rtos_interface = NULL;
        return 0;
    }
    else
    {
        // 如果队列创建成功，将队列句柄传递给rtos接口
        rtos_interface->xQueue = queue;
    }
    uart_instance->rtos_for_uart = rtos_interface;
    LOGINFO("uart rtos init success!");
    return 1;
}


uint8_t Uart_Receive_Handler(Uart_Instance_t *uart_instance)
{
    if(uart_instance == NULL)
    {
        LOGERROR("Uart_Receive_Handler failed!");
        return 0;
    }
    /* 检查UART的空闲中断标志位是否置位 */
    if(__HAL_UART_GET_FLAG(uart_instance->uart_package->uart_handle, UART_FLAG_IDLE) != RESET)
    {
        Uart_Rx_Idle_Callback(uart_instance);
    }
    else
    {
        /* UART的空闲中断标志位无置位 */
        LOGERROR("Uart_Receive_Handler NO FLAG!");
        return 0;
    }
    return 1;
}


/**
 * @brief 串口空闲中断回调函数，static
 * 
 * @param uart_instance 
 * @return uint8_t --- 1 :success
 *                 --- 0 :failed
 */
static uint8_t Uart_Rx_Idle_Callback(Uart_Instance_t *uart_instance)
{
    if(uart_instance == NULL)
    {
        LOGERROR("Rx_Idle_Callback failed!");
        return 0;
    }

    static uint16_t uart_rx_num;

    /* 清除空闲中断标志位 */
    __HAL_UART_CLEAR_IDLEFLAG(uart_instance->uart_package->uart_handle);

    /* 停止DMA传输 */
    HAL_UART_DMAStop(uart_instance->uart_package->uart_handle);

    uart_rx_num = uart_instance->uart_package->rx_buffer_size - ((DMA_Stream_TypeDef*)uart_instance->uart_package->uart_handle->hdmarx->Instance)->NDTR;// 获取接收到的数据长度
    if(uart_instance->uart_package->uart_callback != NULL)
    {
        /* 如果用户自己实现了串口回调中断函数，则调用 */
        uart_instance->uart_package->uart_callback(uart_instance,uart_rx_num);
    }
    else
    {
        LOGERROR("uart_callback is NULL!");
        return 0;
    }

    /* 重新开启DMA中断 */
    HAL_UART_Receive_DMA(uart_instance->uart_package->uart_handle, uart_instance->uart_package->rx_buffer, uart_instance->uart_package->rx_buffer_size);// 重新启动DMA传输
    return 1;
}

uint8_t Uart_UnRegister(void *uart_instance)
{
    if (uart_instance == NULL)
    {
        LOGERROR("uart remove list failed!");
        return 0;
    }
    Uart_Instance_t *temp_uart_instance = uart_instance;
    for (uint8_t i = 0; i < idx; i++)
    {
        if (temp_uart_instance == Usart_Device[i])
        {
            // 调用 Deinit 函数来清除实例和父类实例
            Uart_Deinit(&temp_uart_instance);

            // 从数组中移除指定项
            for (uint8_t j = i; j < idx - 1; j++)
            {
                Usart_Device[j] = Usart_Device[j + 1];
            }
            Usart_Device[idx - 1] = NULL;
            idx--;

            LOGINFO("uart instance removed from list successfully!");
            return 1;
        }
    }

    LOGERROR("uart instance not found in list!");
    return 0;
}

static uint8_t Uart_Deinit(Uart_Instance_t **uart_instance)
{
    if(uart_instance == NULL || *uart_instance == NULL) 
    {
        LOGERROR("In Deinit uart_instance is not valid!");
        return 0;
    }
    Uart_Instance_t *temp_uart_instance = *uart_instance;
    /* 删除uart->rtos */
    if(temp_uart_instance->rtos_for_uart != NULL)
    {
        if(temp_uart_instance->rtos_for_uart->xQueue != NULL)
        {
            vQueueDelete(temp_uart_instance->rtos_for_uart->xQueue);
            temp_uart_instance->rtos_for_uart->xQueue = NULL;
        }
        temp_uart_instance->rtos_for_uart->queue_receive = NULL;
        temp_uart_instance->rtos_for_uart->queue_send = NULL;
        vPortFree(temp_uart_instance->rtos_for_uart);
        temp_uart_instance->rtos_for_uart = NULL;
    }
    
    /* 删除uart->package */
    /* 规定uart->package必须将其生命域锁在任务函数中，所以不能对这块内存进行释放，需要操作的就只有将函数置空 */
    if(temp_uart_instance->uart_package != NULL)
    {
        temp_uart_instance->uart_package->uart_callback = NULL;
    }

    temp_uart_instance->Uart_Deinit = NULL;
    /* 删除uart实例 */
    vPortFree(temp_uart_instance);
    *uart_instance = NULL;
    return 1;
}

