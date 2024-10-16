/**
 * @file vofa.h
 * @author Keten (2863861004@qq.com)
 * @brief 
 * @version 0.1
 * @date 2024-10-14
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention :
 * @note :
 * @versioninfo :
 */
#ifndef VOFA_H 
#define VOFA_H 

#ifdef __cplusplus
extern "C"{
#endif

/*----------------------------------include-----------------------------------*/
/* freertos接口，提供堆管理 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "bsp_usart.h"
#include "bsp_log.h"
#include "topics.h"
#include "data_type.h"
/*-----------------------------------macro------------------------------------*/

/*----------------------------------typedef-----------------------------------*/
typedef struct
{
    uint32_t queue_length;// 通过调整队列长度，可以调节队列缓冲
    QueueHandle_t xQueue;// 队列句柄
    BaseType_t (*queue_send)(QueueHandle_t xQueue, const void *pvItemToQueue,BaseType_t *pxHigherPriorityTaskWoken);
    BaseType_t (*queue_receive)(QueueHandle_t xQueue,void *pvBuffer,TickType_t xTicksToWait);// 从队列读数据
}rtos_for_vofa_t;

typedef struct
{
    Uart_Instance_t *vofa_uart_instance;
    rtos_for_vofa_t *rtos_for_vofa;
    Publisher *vofa_pub;
    pub_vofa_pid pub_data;
    uint8_t (*vofa_task)(void* vofa_instance);
    uint8_t (*get_data)(uint8_t*,pub_vofa_pid*);
    uint8_t (*vofa_deinit)(void* vofa_instance);
}VOFA_Instance_t;


/*----------------------------------function----------------------------------*/

/**
 * @brief 
 * 
 * @param vofa_uart_instance 
 * @param queue_length 
 * @return VOFA_Instance_t* 
 */
VOFA_Instance_t* VOFA_init(Uart_Instance_t *vofa_uart_instance,uint32_t queue_length);


/**
 * @brief 
 * 
 * @param vofa_instance 
 * @return uint8_t 
 */
uint8_t VOFA_Task(void* vofa_instance);


/**
 * @brief 
 * 
 * @param data 
 * @param vofa_pid 
 * @return uint8_t 
 */
uint8_t VOFA_Get_Data(uint8_t *data,pub_vofa_pid *vofa_pid);


/**
 * @brief 
 * 
 * @param uart_instance 
 * @param data_len 
 * @return uint8_t 
 */
uint8_t VOFA_Uart_Rx_Callback(void *uart_instance,uint16_t data_len);


/**
 * @brief 
 * 
 * @param vofa_instance 
 * @return uint8_t 
 */
uint8_t VOFA_DeInit(void *vofa_instance);

#ifdef __cplusplus
}
#endif

#endif	/* VOFA_H */
