/**
 * @file ros_com.h
 * @author Keten (2863861004@qq.com)
 * @brief 
 * @version 0.1
 * @date 2024-10-14
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention :
 * @note :  2025.1.14: 1.补充ros通讯中 下位机->上位机的组包
 *          实现？调用一次即发送一次，非定时发送？
 *          目前补和接收的ros包一样格式的发送包
 * @versioninfo :
 */
#ifndef ROS_COM_H 
#define ROS_COM_H 

#ifdef __cplusplus
extern "C"{
#endif

/*----------------------------------include-----------------------------------*/
#include "rtos_interface.h"
#include "bsp_log.h"
#include "bsp_usart.h"
#include "soft_iwdg.h"
#include "topics.h"
#include "data_type.h"
#include "user_tool.h"
#include "usbd_cdc_if.h"
/*-----------------------------------macro------------------------------------*/
#define HEAD_0          0xFC
#define HEAD_1          0xFB
#define END_0           0xFD
#define END_1           0xFE 
#define MAX_DATA_LENGTH   36

#define ROSCOM_QUEUE_LENGTH    10

/*----------------------------------typedef-----------------------------------*/


typedef struct
{
    Uart_Instance_t *uart_instance;
    rtos_for_module_t *rtos_for_roscom;
    IWDG_Instance_t *iwdg_instance;
    float data_send[6];
    float data_get[6]; 
}ROS_Com_Instance_t;

/*----------------------------------variable----------------------------------*/


/*----------------------------------function----------------------------------*/
uint8_t ROS_Communication_Init();

uint8_t ROS_GetData(uint8_t *data,uint16_t data_len);

uint8_t ROSCom_RxCallback_Fun(void *uart_instance, uint16_t data_len);

uint8_t ROSCOM_Task_Function(void *ros_instance);

uint8_t IWDG_For_ROSCOM_Rx(void *device);

uint8_t ROSCom_SendData(float *data);

uint8_t ROSCOM_DeInit(void *ros_instance);



#ifdef __cplusplus
}
#endif

#endif	/* ROS_COM_H */
