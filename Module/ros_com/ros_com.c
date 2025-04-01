/**
 * @file ros_com.c
 * @author Keten (2863861004@qq.com)
 * @brief ros和32通讯包解析，解析之后数据通过话题发布出去
 * @version 0.1
 * @date 2024-10-14
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention :
 * @note :
 * @versioninfo :
 */
#include "ros_com.h"

static uint8_t ROSCom_Rtos_init(ROS_Com_Instance_t* ros_instance,uint32_t queue_length);

uint8_t ros_uart_buffer[29];


ROS_Com_Instance_t *ros_instance;

uart_package_t ros_uart_package = {
    .uart_handle = &huart1,
    .rx_buffer = ros_uart_buffer,
    .rx_buffer_size = 29,
    .uart_callback = ROSCom_RxCallback_Fun,
};
Uart_Instance_t *ros_uart_instance;

iwdg_config_t ros_iwdg_config = {
    .reload_count = 1000,
    .init_count = 0,
    .callback = IWDG_For_ROSCOM_Rx,
};
IWDG_Instance_t *ros_iwdg_instance;

// 全局下的rx联合体
union roscom_rx
{
    uint8_t data[4];// 6个float + 2个帧头 + 2个帧尾 + 1crc
    float value;// 6个数据位
}roscom_data_rx[6];

// 全局下的tx联合体
union roscom_tx
{
    uint8_t data[6*4];
    float value[6];
}roscom_data_tx;


uint8_t ROS_Communication_Init()
{
    /* 动态创建 */
    ros_instance = (ROS_Com_Instance_t*)pvPortMalloc(sizeof(ROS_Com_Instance_t));
    if(ros_instance == NULL)
    {
        LOGERROR("ros_instance malloc failed!");
        return 0;
    }

    /* 挂载uart串口接口 */
    ros_uart_instance = Uart_Register(&ros_uart_package);
    if(ros_uart_instance == NULL)
    {
        LOGERROR("ros uart instance init failed!");
        return 0;
    }
    ros_instance->uart_instance = ros_uart_instance;

    /* 挂载iwdg接口 */    
    ros_iwdg_instance = IWDG_Register(&ros_iwdg_config);
    if(ros_iwdg_instance == NULL)
    {
        LOGERROR("ros iwdg instance init failed!");
        vPortFree(ros_instance);
        ros_instance = NULL;
        return 0;
    }
    ros_instance->iwdg_instance = ros_iwdg_instance;

    /* rtos api ini */
    if(ROSCom_Rtos_init(ros_instance,ROSCOM_QUEUE_LENGTH) != 1)
    {
        LOGERROR("ros rtos init failed!");
        vPortFree(ros_instance);
        ros_instance = NULL;
        return 0;
    }

    /* 初始化成功 */
    return 1;
}


static uint8_t ROSCom_Rtos_init(ROS_Com_Instance_t* ros_instance,uint32_t queue_length)
{
    if(ros_instance == NULL)
    {
        LOGERROR("ros_instance rtos init failed!");
        return 0;
    }
    rtos_for_module_t *rtos_for_roscom = (rtos_for_module_t *)pvPortMalloc(sizeof(rtos_for_module_t));
    if(rtos_for_roscom == NULL)
    {
        LOGERROR("rtos_for_roscom malloc failed!");
        return 0;
    }
    memset(rtos_for_roscom,0,sizeof(rtos_for_module_t));

    rtos_for_roscom->queue_send = queue_send_wrapper;
    rtos_for_roscom->queue_receive = xQueueReceive;

    QueueHandle_t queue = xQueueCreate(queue_length,sizeof(UART_TxMsg));
    if(queue == NULL)
    {
        LOGERROR("queue create failed!");
        vPortFree(rtos_for_roscom);
        rtos_for_roscom = NULL;
        return 0;
    }
    else
    {
        rtos_for_roscom->xQueue = queue;
    }

    ros_instance->rtos_for_roscom = rtos_for_roscom;  
    LOGINFO("ros rtos init success!");
    return 1;
}


uint8_t ROS_GetData(uint8_t *data,uint16_t data_len)
{
    if(data == NULL || data_len != 29)// 如果长度小于29，那么容易溢出访问数组
    {
        LOGERROR("ros data is NULL!");
        return 0;
    }
    if(data[0] != HEAD_0 && data[1] != HEAD_1)// 检查帧头
        return 0;
    if(data[27] != END_0 && data[28] != END_1)// 检查帧尾
        return 0;

    for(size_t i = 0;i < 4;i++)
    {
        roscom_data_rx[0].data[i] = data[2+i];
        roscom_data_rx[1].data[i] = data[6+i];
        roscom_data_rx[2].data[i] = data[10+i];
        roscom_data_rx[3].data[i] = data[14+i];
        roscom_data_rx[4].data[i] = data[18+i];
        roscom_data_rx[5].data[i] = data[22+i];
    }

    if(data[26] != serial_get_crc8_value(data,26))// 检查crc
    {
        memset(roscom_data_rx,0,sizeof(roscom_data_rx));
        return 0;
    }
    ros_instance->data_get[0] = roscom_data_rx[0].value;
    ros_instance->data_get[1] = roscom_data_rx[1].value;
    ros_instance->data_get[2] = roscom_data_rx[2].value;
    ros_instance->data_get[3] = roscom_data_rx[3].value;
    ros_instance->data_get[4] = roscom_data_rx[4].value;
    ros_instance->data_get[5] = roscom_data_rx[5].value;
    return 1;
}


uint8_t ROSCom_RxCallback_Fun(void *uart_instance, uint16_t data_len)
{
    UART_TxMsg Msg;
    if(uart_instance == NULL)
    {
        LOGERROR("uart instance is NULL");
        return 0;
    }
    Uart_Instance_t *temp_uart_instance = (Uart_Instance_t*)uart_instance;
    ROS_Com_Instance_t *temp_ros_instance = temp_uart_instance->device;

    if(temp_ros_instance->rtos_for_roscom->xQueue != NULL && temp_ros_instance->rtos_for_roscom->queue_send != NULL)
    {
        Msg.data_addr = temp_ros_instance->uart_instance->uart_package.rx_buffer;
        Msg.len = data_len;
        Msg.huart = temp_ros_instance->uart_instance->uart_package.uart_handle;
        if(Msg.data_addr != NULL)
        {
            temp_ros_instance->rtos_for_roscom->queue_send(temp_ros_instance->rtos_for_roscom->xQueue,&Msg,NULL);
            return 1;
        }
    }

    return 0;
}



uint8_t ROSCOM_Task_Function(void *ros_instance)
{
    UART_TxMsg Msg;
    if(ros_instance == NULL)
    {
        LOGERROR("ros_instance is NULL!");
        return 0;
    }
    ROS_Com_Instance_t *temp_ros_instance = ros_instance;
    if(temp_ros_instance->rtos_for_roscom->xQueue != NULL && temp_ros_instance->rtos_for_roscom->queue_receive != NULL)
    {
        if(temp_ros_instance->rtos_for_roscom->queue_receive(temp_ros_instance->rtos_for_roscom->xQueue,&Msg,0) == pdPASS)
        {
            if(ROS_GetData(Msg.data_addr,Msg.len) == 1)
            {
                // 在这里发布的对象应该是待定的 ...
                // 将需要发送的数据分别发送出去！
                // 或者在外面做数据发送工作...
                return 1;
            }
        }
    }
    return 0;
}


uint8_t IWDG_For_ROSCOM_Rx(void *device)
{
    if(device == NULL)
    {
        LOGERROR("device is NULL!");
        return 0;
    }

    /* 在这里做相应没喂狗的处理，比如使用蜂鸣器警告，然后stm32重新打开串口的处理 */
    return 1;
}


// ROS发送函数，组包
uint8_t ROSCom_SendData(float *data)
{
    uint8_t temp_data[29];
    memset(temp_data, 0, sizeof(temp_data));

    temp_data[0] = HEAD_0;temp_data[1] = HEAD_1;temp_data[27] = END_0;temp_data[28] = END_1;

    for(size_t i = 0;i < 6;i++)
        roscom_data_tx.value[i] = data[i];
    
    memcpy(&temp_data[2], roscom_data_tx.data, 24);
    // crc校验
    temp_data[26] = serial_get_crc8_value(temp_data,26);
    
    uint8_t transmit_result = CDC_Transmit_FS(temp_data, sizeof(temp_data));
    if (transmit_result != USBD_OK)
    {
        LOGERROR("ROSCom_SendData: CDC_Transmit_FS failed!");
        return 0;
    }
    return 1;
}

uint8_t ROSCOM_DeInit(void *ros_instance)
{
    if(ros_instance == NULL)
    {
        LOGERROR("ros_instance is NULL!");
        return 0;
    }
    ROS_Com_Instance_t *temp_ros_instance = ros_instance;

    if(temp_ros_instance->rtos_for_roscom != NULL)
    {
        temp_ros_instance->rtos_for_roscom->queue_receive = NULL;
        temp_ros_instance->rtos_for_roscom->queue_send = NULL;
        if(temp_ros_instance->rtos_for_roscom->xQueue != NULL)
        {
            vQueueDelete(temp_ros_instance->rtos_for_roscom->xQueue);
            temp_ros_instance->rtos_for_roscom->xQueue = NULL;
        }
        vPortFree(temp_ros_instance->rtos_for_roscom);
        temp_ros_instance->rtos_for_roscom = NULL;
    }

    /* 清除roscom中串口接口 */
    if(temp_ros_instance->uart_instance != NULL)
    {
        temp_ros_instance->uart_instance->Uart_Deinit(temp_ros_instance->uart_instance);
    }

    vPortFree(temp_ros_instance);
    temp_ros_instance = NULL;
    ros_instance = NULL;
    return 1;
}


