/**
 * @file rtos_interface.h
 * @author Keten (2863861004@qq.com)
 * @brief rtos接口层，为Module层提供freertos的api接口，实现Module层和rtos层的解耦
 * @version 0.1
 * @date 2024-10-16
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention :
 * @note :  以后更换不同的rtos，只需要修改这里的rtos接口层，就可以将module直接移植到不同的rtos上
 * @versioninfo :
 */
#pragma once

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"   
#include "cmsis_os2.h"

typedef struct
{
    uint32_t queue_length;// 通过调整队列长度，可以调节队列缓冲
    QueueHandle_t xQueue;// 队列句柄
    BaseType_t (*queue_send)(QueueHandle_t xQueue, const void *pvItemToQueue,BaseType_t *pxHigherPriorityTaskWoken);
    BaseType_t (*queue_receive)(QueueHandle_t xQueue,void *pvBuffer,TickType_t xTicksToWait);// 从队列读数据
}rtos_for_module_t;


/* 由于xQueueSendFromISR为宏定义，不能直接挂载函数指针，所以先用函数包装一下 */
BaseType_t queue_send_wrapper(QueueHandle_t xQueue, const void *pvItemToQueue, BaseType_t *pxHigherPriorityTaskWoken);