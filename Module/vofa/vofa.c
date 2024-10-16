/**
 * @file vofa.c
 * @author Keten (2863861004@qq.com)
 * @brief vofa发布命令接收解析
 *        vofa发布话题 “vofa_pub” ，创建订阅者订阅vofa_pub即可获取调试数据
 * @version 0.1
 * @date 2024-10-14
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention :
 * @note :
 * @versioninfo :
 */
#include "vofa.h"
#include "topics.h"

/* 由于xQueueSendFromISR为宏定义，不能直接挂载函数指针，所以先用函数包装一下 */
static BaseType_t queue_send_wrapper(QueueHandle_t xQueue, const void *pvItemToQueue, BaseType_t *pxHigherPriorityTaskWoken) {
    return xQueueSendFromISR(xQueue, pvItemToQueue, pxHigherPriorityTaskWoken);
}

static uint8_t VOFA_Rtos_Init(VOFA_Instance_t *vofa_instance,uint32_t queue_length);


uint8_t vofa_rx_buffer[100];

/* vofa接收数据包配置 */
uart_package_t VOFA_uart_package = {
    .uart_handle = &huart1,
    .rx_buffer = vofa_rx_buffer,
    .rx_buffer_size = 9,
    .uart_callback = VOFA_Uart_Rx_Callback,
};

union vofa
{
    uint8_t data[4];
    float value;
}vofa_data;


VOFA_Instance_t* VOFA_init(Uart_Instance_t *vofa_uart_instance,uint32_t queue_length)
{
    if(vofa_uart_instance == NULL)
    {
        LOGERROR("vofa init failed!");
        return NULL;
    }
    VOFA_Instance_t *temp_vofa_instance = (VOFA_Instance_t *)pvPortMalloc(sizeof(VOFA_Instance_t));
    if(temp_vofa_instance == NULL)
    {
        LOGERROR("vofa instance malloc failed!");
        return NULL;
    }
    memset(temp_vofa_instance,0,sizeof(VOFA_Instance_t));
    /* 挂载外部串口接口 */
    temp_vofa_instance->vofa_uart_instance = vofa_uart_instance;
    /* 注册vofa发布者,并挂载内部接口 */
    temp_vofa_instance->vofa_pub = register_pub("vofa_pub");
    if(temp_vofa_instance->vofa_pub == NULL)
    {
        LOGERROR("vofa pub register failed!");
        vPortFree(temp_vofa_instance);
        temp_vofa_instance = NULL;
        return NULL;
    }
    /* 挂载rtos接口 */
    if(VOFA_Rtos_Init(temp_vofa_instance,queue_length) != 1)
    {
        LOGERROR("vofa rtos init failed!");
        vPortFree(temp_vofa_instance);
        temp_vofa_instance = NULL;
        return NULL;
    }

    temp_vofa_instance->get_data = VOFA_Get_Data;
    temp_vofa_instance->vofa_task = VOFA_Task;
    temp_vofa_instance->vofa_deinit = VOFA_DeInit;

    temp_vofa_instance->vofa_uart_instance->device = temp_vofa_instance;
    LOGINFO("vofa instance init success!");
    return temp_vofa_instance;
}   


/**
 * @brief 挂载rtos接口
 * 
 * @param vofa_instance 
 * @param queue_length 
 * @return uint8_t 
 */
static uint8_t VOFA_Rtos_Init(VOFA_Instance_t *vofa_instance,uint32_t queue_length)
{
    if(vofa_instance == NULL)
    {
        LOGERROR("vofa instance is NULL!");
        return 0;
    }
    rtos_for_vofa_t *rtos_for_vofa = (rtos_for_vofa_t *)pvPortMalloc(sizeof(rtos_for_vofa_t));
    if(rtos_for_vofa == NULL)
    {
        LOGERROR("rtos_for_vofa malloc failed!");
        return 0;
    }
    memset(rtos_for_vofa,0,sizeof(rtos_for_vofa_t));

    rtos_for_vofa->queue_send = queue_send_wrapper;
    rtos_for_vofa->queue_receive = xQueueReceive;

    QueueHandle_t queue = xQueueCreate(queue_length,sizeof(UART_TxMsg));
    if(queue == NULL)
    {
        LOGERROR("queue create failed!");
        vPortFree(rtos_for_vofa);
        rtos_for_vofa = NULL;
        return 0;
    }
    else
    {
        rtos_for_vofa->xQueue = queue;
    }
    vofa_instance->rtos_for_vofa = rtos_for_vofa;
    LOGINFO("vofa rtos init success!");
    return 1;
}


uint8_t VOFA_Task(void* vofa_instance)
{
    UART_TxMsg Msg;
    if(vofa_instance == NULL)
    {
        LOGERROR("vofa instance is NULL!");
        return 0;
    }
    VOFA_Instance_t *temp_vofa_instance = vofa_instance;
    if(temp_vofa_instance->rtos_for_vofa->xQueue != NULL && temp_vofa_instance->rtos_for_vofa->queue_receive != NULL)
    {
        if(temp_vofa_instance->rtos_for_vofa->queue_receive(temp_vofa_instance->rtos_for_vofa->xQueue,&Msg,0) == pdPASS)
        {
            if(VOFA_Get_Data(Msg.data_addr,&temp_vofa_instance->pub_data) == 1)
            {
                publish_data temp_data;
                temp_data.data = &temp_vofa_instance->pub_data;
                temp_data.len = sizeof(pub_vofa_pid);
                temp_vofa_instance->vofa_pub->publish(temp_vofa_instance->vofa_pub,temp_data);
                return 1;
            }
        }
    }
    return 0;
}


// 自定义串口协议 C5 C6 A(1~4) (float) D5 D6
uint8_t VOFA_Get_Data(uint8_t *data,pub_vofa_pid *vofa_pid)
{
    if(data == NULL)
    {
        LOGERROR("vofa instance is NULL!");
        return 0;
    }
    if(data[0] != 0xC5 || data[1] != 0xC6)
    {
        LOGERROR("header is wrong!");
        return 0;
    }
    if(data[7] != 0xD5 || data[8] != 0xD6)
    {
        LOGERROR("tail is wrong!");
        return 0;
    }

    for(size_t i = 0; i < 4; i++)
    {
        vofa_data.data[i] = data[i+3];
    }
    
    /* vofa包协议，通过vofa命令面板中的编辑进行修改 */
    switch(data[2])
    {
        case 0xA1:
            vofa_pid->Kp = vofa_data.value;
            break;
        case 0xA2:
            vofa_pid->Ki = vofa_data.value;
            break;
        case 0xA3:
            vofa_pid->Kd = vofa_data.value;
            break;
        case 0xA4:
            vofa_pid->ref = vofa_data.value;
            break;
    }
    memset(data,0,28);    
    return 1;
}


uint8_t VOFA_Uart_Rx_Callback(void *uart_instance,uint16_t data_len)
{
    UART_TxMsg Msg;
    if(uart_instance == NULL)
    {
        LOGERROR("uart_instance is NULL!");
        return 0;
    }
    Uart_Instance_t *temp_uart_instance = (Uart_Instance_t*)uart_instance;
    VOFA_Instance_t *temp_vofa_instance = temp_uart_instance->device;

    if(temp_vofa_instance->rtos_for_vofa->xQueue != NULL && temp_vofa_instance->rtos_for_vofa->queue_send != NULL)
    {
        Msg.data_addr = temp_vofa_instance->vofa_uart_instance->uart_package.rx_buffer;
        Msg.len = data_len;
        Msg.huart = temp_vofa_instance->vofa_uart_instance->uart_package.uart_handle;
        if(Msg.data_addr != NULL)
        {
            temp_vofa_instance->rtos_for_vofa->queue_send(temp_vofa_instance->rtos_for_vofa->xQueue,&Msg,NULL);
            return 1;
        }
    }

    return 0;
}


uint8_t VOFA_DeInit(void *vofa_instance)
{
    if(vofa_instance == NULL)
    {
        LOGERROR("vofa instance is NULL!");
        return 0;
    }
    VOFA_Instance_t *temp_vofa_instance = vofa_instance;

    /* 清除vofa设备中rtos接口 */
    if(temp_vofa_instance->rtos_for_vofa != NULL)
    {
        temp_vofa_instance->rtos_for_vofa->queue_receive = NULL;
        temp_vofa_instance->rtos_for_vofa->queue_send = NULL;
        if(temp_vofa_instance->rtos_for_vofa->xQueue != NULL)
        {
            vQueueDelete(temp_vofa_instance->rtos_for_vofa->xQueue);
            temp_vofa_instance->rtos_for_vofa->xQueue = NULL;
        }
        vPortFree(temp_vofa_instance->rtos_for_vofa);
        temp_vofa_instance->rtos_for_vofa = NULL;
    }

    /* 清除vofa中串口接口 */
    if(temp_vofa_instance->vofa_uart_instance != NULL)
    {
        temp_vofa_instance->vofa_uart_instance->Uart_Deinit(temp_vofa_instance->vofa_uart_instance);
    }

    temp_vofa_instance->get_data = NULL;
    temp_vofa_instance->vofa_task = NULL;
    temp_vofa_instance->vofa_deinit = NULL;

    vPortFree(temp_vofa_instance);
    temp_vofa_instance = NULL;
    vofa_instance = NULL;
    return 1;
}