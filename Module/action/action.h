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
#include "bsp_usart.h"
#include "usart.h"
#include "bsp_log.h"
#include "arm_math.h"
#include "data_pool.h"
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

/* Action设备实例，使用时需创建完实例和接口 */
typedef struct 
{
    Uart_Instance_t *action_uart_instance;// 继承自串口设备
    action_original_info *action_orin_data;// 存放未经处理的action数据
#ifdef USE_DIFF_GET_DATA
    robot_info_from_action *action_diff_data;// 存放经过差分运算过后的action数据
#endif
    uint8_t (*action_get_data)(uint8_t *, action_original_info*, robot_info_from_action* );// 获取action返回的值
    uint8_t (*action_refresh_data)(void);// 刷新action模块初始值
    uint8_t (*action_task)(void* action_instance);
}Action_Instance_t;

/*----------------------------------function----------------------------------*/

/**
 * @brief action实例初始化函数
 * 
 * @param action_uart 
 * @return Action_Instance_t 
 */
Action_Instance_t* Action_Init(Uart_Instance_t *action_uart);


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
uint8_t Action_RxCallback_Fun(Uart_Instance_t *action_instance, uint16_t data_len);


/**
 * @brief 刷新action数据值
 * 
 * @return uint8_t 
 */
uint8_t Action_RefreshData(void);

#ifdef __cplusplus
}
#endif

#endif	/* ACITON_H */
