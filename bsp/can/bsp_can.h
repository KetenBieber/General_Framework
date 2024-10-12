/**
 * @file bsp_can.h
 * @author Keten (2863861004@qq.com)
 * @brief 
 * @version 0.1
 * @date 2024-09-23
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention : 
 * @note :编写can驱动层，目前想要实现的是：CubeMX默认开启两路can 并且均启用双FIFO模式；其使用取决于你挂载了多少设备
 *        对于电机设备，一条总线上最多挂载8个电机，但总线负载会过大，所以这里限制最大电机设备数为6
 *        对于电机设备需要完成注册操作，注册到对应的总线上，并且配置好id
 *        定义为can设备，所以应该是每一个都是设备性质 - - - 可以发也可以收
 *        做些约定：can1 用来挂载收标准帧id的
 *                 can2 用来挂载收扩展帧id的
 *        通过位操作来实现便携的can配置，更快！
 *         
 * @versioninfo :
 */
#ifndef BSP_CAN_H 
#define BSP_CAN_H 

#ifdef __cplusplus
extern "C"{
#endif

/*----------------------------------include-----------------------------------*/
#ifdef USE_RTOS_FOR_CAN
#include "FreeRTOS.h"
#include "queue.h"
#endif

/* c语言库接口头文件 */
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
/* hal层接口头文件 */
#include "can.h"

/* bsp层接口头文件 */
#include "bsp_log.h"
/*-----------------------------------macro------------------------------------*/
#define DEVICE_CAN_CNT  6 // 一条can总线上至多挂载6个设备


/* 简单介绍一下这套can id配置


 */


/* 通过将下面四个字段进行 按位或 ，可组合成一个八位字段 */

/* 过滤器组编号 占据字段中第3位到第7位的位置，可配置27位过滤器编号 */
#define CanFilter_0     (0  << 3)
#define CanFilter_1     (1  << 3)
#define CanFilter_2     (2  << 3)
#define CanFilter_3     (3  << 3)
#define CanFilter_4     (4  << 3)
#define CanFilter_5     (5  << 3)
#define CanFilter_6     (6  << 3)
#define CanFilter_7     (7  << 3)
#define CanFilter_8     (8  << 3)
#define CanFilter_9     (9  << 3)
#define CanFilter_10    (10 << 3)
#define CanFilter_11    (11 << 3)
#define CanFilter_12    (12 << 3)
#define CanFilter_13    (13 << 3)
#define CanFilter_14    (14 << 3)
#define CanFilter_15    (15 << 3)
#define CanFilter_16    (16 << 3)
#define CanFilter_17    (17 << 3)
#define CanFilter_18    (18 << 3)
#define CanFilter_19    (19 << 3)
#define CanFilter_20    (20 << 3)
#define CanFilter_21    (21 << 3)
#define CanFilter_22    (22 << 3)
#define CanFilter_23    (23 << 3)
#define CanFilter_24    (24 << 3)
#define CanFilter_25    (25 << 3)
#define CanFilter_26    (26 << 3)
#define CanFilter_27    (27 << 3)

/* FIFO选择 占据字段中第2位的位置 可配置双FIFO */
#define CanFifo_0       (0 << 2)
#define CanFifo_1       (1 << 2)

/* std&ext 选择 占据字段中第1位的位置 可配置扩展帧or标准帧 区别是扩展29位 标准11位 */
#define Can_STDID       (0 << 1)
#define Can_EXTID       (1 << 1)

/* data&remote 选择 占据字段中第0位的位置 可配置数据帧or遥控帧 */
#define Can_DataType    (0 << 0)
#define Can_RemoteType  (1 << 0)

#define CAN_LINE_BUSY 0
#define CAN_SUCCESS   1
#define CAN_FIFO_SIZE 1024
/*----------------------------------typedef-----------------------------------*/
/* can发送设备实例 */
typedef struct
{
    CAN_HandleTypeDef *can_handle; // hal库can句柄成员
    bool isExTid;                  // 要发送的是否为扩展帧,如果是的话，为1，否则为标准帧
    uint32_t tx_mailbox;           // 发送邮箱号
    uint32_t tx_id;                // 发送id
    uint8_t tx_len;                // 发送数据帧长度，长度范围为0~8
    uint8_t can_tx_buff[8];          // can发送buffer
}CAN_Tx_Instance_t;

/* can接收设备实例 */
typedef struct 
{
    CAN_HandleTypeDef *can_handle;  // hal库can句柄成员
    CAN_RxHeaderTypeDef RxHeader;
    uint8_t rx_len;                 // 接收数据帧长度，长度范围为0~8
    uint32_t rx_id;                 // 接收id   
    uint8_t object_para;            // 过滤器配置id 按CanFilter_N | CanFifo_N | Can_XXXID | Can_XXXXType 格式设置
    uint32_t mask_id;               // 过滤器掩码id,设置为0则表示不过滤
    uint8_t can_rx_buff[8];         // can接收buffer
}CAN_Rx_Instance_t;

/*----------------------------------function----------------------------------*/

/**
 * @brief 
 * 
 * @param hcan 
 * @param pFunc 
 * @return uint8_t 
 */
uint8_t CAN_Init(CAN_HandleTypeDef* hcan, void (*pFunc)(CAN_Rx_Instance_t*));


/**
 * @brief can接收设备添加过滤器
 * 
 * @param hcan            can句柄
 * @param can_rx_instance can接收设备实例
 * @return uint8_t 
 */
// uint8_t CAN_Add_Filter(CAN_HandleTypeDef *hcan,CAN_Rx_Instance_t *can_rx_instance);
void CAN_Filter_Init(CAN_HandleTypeDef * hcan, uint8_t object_para,uint32_t Id,uint32_t MaskId);


/**
 * @brief 扩展帧发送函数
 * 
 * @param can_tx_instance 
 */
void CAN_Transmit_ExtId(CAN_Tx_Instance_t *can_tx_instance);


/**
 * @brief 标准帧发送函数
 * 
 * @param can_tx_instance 
 */
void CAN_Transmit_StdID(CAN_Tx_Instance_t *can_tx_instance);

/*------------------------------------test------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif	/* BSP_CAN_H */
