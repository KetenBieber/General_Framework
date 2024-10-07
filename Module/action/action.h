/**
 * @file aciton.h
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
#ifndef ACITON_H 
#define ACITON_H 

#ifdef __cplusplus
extern "C"{
#endif

/*----------------------------------include-----------------------------------*/
/* rtos层接口 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
/* hal库接口 */
#include "usart.h"

/* bsp层接口 */
#include "bsp_usart.h"
#include "bsp_log.h"
#include "data_pool.h"
#include "soft_iwdg.h"


#include "arm_math.h"

/*-----------------------------------macro------------------------------------*/
#define USE_DIFF_GET_DATA           // 使用差分运算获取action模块返回值宏，定义了就可以获取
#define ACTION_DATA_NUM 28          // action一个数据包的字节数

/*----------------------------------typedef-----------------------------------*/
/* action获取数据的结构体，在使用action时才会创建 */
typedef struct
{
	float ANGLE_Z;
	float ANGLE_X;
	float ANGLE_Y;	
	float POS_X;
	float POS_Y;
	float W_Z;

#ifdef USE_DIFF_GET_DATA    // 如果使用差分运算，需要
	float LAST_POS_X;
	float LAST_POS_Y;
	float LAST_YAW;
	
	float DELTA_POS_X;
	float DELTA_POS_Y;	
	float DELTA_YAW;
	
	float REAL_X;
	float REAL_Y;
	float REAL_YAW;
#endif
}action_original_info;

/* action获取数据常用来获取坐标以及朝向角 */
typedef struct
{
    float x;
    float y;
    float yaw;
    float yaw_rate;// 角速度
}robot_info_from_action;



/* 引入FREERTOS接口 */
/* 其实相当于为module层露出了rtos的接口，只要挂载了对应的rtos api，就可以调用 */
typedef struct
{
    uint32_t queue_length;// 通过调整队列长度，可以调节队列缓冲
    QueueHandle_t xQueue;// 队列句柄
    BaseType_t (*queue_send)(QueueHandle_t xQueue, const void *pvItemToQueue,BaseType_t *pxHigherPriorityTaskWoken);
    BaseType_t (*queue_receive)(QueueHandle_t xQueue,void *pvBuffer,TickType_t xTicksToWait);// 从队列读数据
}rtos_interface_t;


/* Action设备实例，使用时需创建完实例和接口 */
typedef struct 
{
	rtos_interface_t *rtos_for_action;
    Uart_Instance_t *action_uart_instance;// 继承自串口设备
	IWDG_Instance_t *action_iwdg_instance;// 继承自看门狗设备,使用看门狗进行监护
    action_original_info *action_orin_data;// 存放未经处理的action数据
#ifdef USE_DIFF_GET_DATA
    robot_info_from_action *action_diff_data;// 存放经过差分运算过后的action数据
#endif
    uint8_t (*action_get_data)(uint8_t *, action_original_info*, robot_info_from_action* );// 获取action返回的值
    uint8_t (*action_refresh_data)(void);// 刷新action模块初始值
    uint8_t (*action_task)(void* action_instance);
	uint8_t (*action_deinit)(void* action_instance);
}Action_Instance_t;

/*----------------------------------function----------------------------------*/
/**
 * @brief action设备初始化函数
 * 
 * @param action_uart 
 * @param action_iwdg 
 * @param queue_length 
 * @return Action_Instance_t* 
 */
Action_Instance_t* Action_Init(Uart_Instance_t *action_uart, IWDG_Instance_t *action_iwdg,uint32_t queue_length);


/**
 * @brief action任务函数，可以直接放置在freertos中的一个线程中（目前只是单独读队列的函数）
 * 
 * @param action_instance 
 * @return uint8_t 
 */
uint8_t Action_Task(void* action_instance);


/**
 * @brief 读取action模块数据
 * 
 * @param data 
 * @param action_data 
 * @param info_from_action 
 * @return uint8_t 
 */
uint8_t Action_GetData(uint8_t *data,action_original_info *action_data,robot_info_from_action *info_from_action);


/**
 * @brief action串口设备回调函数
 * 
 * @param action_instance 
 * @param data_len 
 * @return uint8_t 
 */
uint8_t Action_RxCallback_Fun(void *action_instance, uint16_t data_len);


/**
 * @brief 刷新action数据值
 * 
 * @return uint8_t 
 */
uint8_t Action_RefreshData(void);


/**
 * @brief action设备注册到看门狗中的回调函数
 * 
 */
uint8_t action_iwdg_callback(void *instance);


/**
 * @brief action设备注销函数
 * 
 * @param action_instance 
 * @return uint8_t 
 */
uint8_t Action_Deinit(void *action_instance);

#ifdef __cplusplus
}
#endif

#endif	/* ACITON_H */
