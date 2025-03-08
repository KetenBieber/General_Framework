/**
 * @file xbox.h
 * @author 冯大帅将军转自原作者刘洁宇 
 * @brief 
 * @version 0.1
 * @date 2025-1-13
 * 
 * @copyright Copyright (c) 2025
 * 
 * @attention :   
 *                                    
 * @note :
 * @versioninfo :
 */
#ifndef XBOX_H
#define XBOX_H

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------include-----------------------------------*/
#include "FreeRTOS.h"
#include "task.h"

/* bsp层接口 */
#include "bsp_dwt.h"
#include "bsp_usart.h"

/* module层接口 */
#include "topics.h"
#include "data_type.h"
#include <math.h>
#include <stdbool.h>

/*----------------------------------define------------------------------------*/
#define FRAME_HEAD_0_ESP32 0xFC
#define FRAME_HEAD_1_ESP32 0xFB
#define FRAME_ID_ESP32 0x01 // 示例数据帧ID
#define FRAME_END_0_ESP32 0xFD
#define FRAME_END_1_ESP32 0xFE
#define MAX_DATA_LENGTH_ESP32 28

/*----------------------------------typedef-----------------------------------*/
typedef enum
{
    WAITING_FOR_HEADER_0_ESP32,
    WAITING_FOR_HEADER_1_ESP32,
    WAITING_FOR_ID_ESP32,
    WAITING_FOR_LENGTH_ESP32,
    WAITING_FOR_DATA_ESP32,
    WAITING_FOR_CRC_0_ESP32,
    WAITING_FOR_CRC_1_ESP32,
    WAITING_FOR_END_0_ESP32,
    WAITING_FOR_END_1_ESP32
} rx_state_esp32_t;

typedef struct serial_frame_esp32
{
    uint8_t data_length;
    uint8_t frame_head[2];
    uint8_t frame_id;
    uint16_t crc_calculated;
    union data_buff
    {
        // float msg_get[MAX_DATA_LENGTH_ESP32];
        uint8_t buff_msg[MAX_DATA_LENGTH_ESP32];
    } data_buff;
    union check_code_esp
    {
        uint16_t crc_code;
        uint8_t crc_buff[2];
    } check_code_e;
    uint8_t frame_end[2];
} serial_frame_esp32_t;
typedef struct
{
    // 按键数据（bool类型）
    bool btnY;
    bool btnY_last;
    bool btnB;
    bool btnB_last;
    bool btnA;
    bool btnA_last;
    bool btnX;
    bool btnX_last;
    bool btnShare;
    bool btnShare_last;
    bool btnStart;
    bool btnStart_last;
    bool btnSelect;
    bool btnSelect_last;
    bool btnXbox;
    bool btnXbox_last;
    bool btnLB;
    bool btnLB_last;
    bool btnRB;
    bool btnRB_last;
    bool btnLS;
    bool btnLS_last;
    bool btnRS;
    bool btnRS_last;
    bool btnDirUp;
    bool btnDirUp_last;
    bool btnDirLeft;
    bool btnDirLeft_last;
    bool btnDirRight;
    bool btnDirRight_last;
    bool btnDirDown;
    bool btnDirDown_last;

    // 霍尔值（16位数值）
    uint16_t joyLHori;
    uint16_t joyLVert;
    uint16_t joyRHori;
    uint16_t joyRVert;
    uint16_t trigLT;
    uint16_t trigRT;

    float joyLHori_map;
    float joyLVert_map;
    float joyRHori_map;
    float joyRVert_map;
    float trigLT_map;
    float trigRT_map;
    
} XboxControllerData_t;

typedef struct
{
    Uart_Instance_t *xbox_uart_instance;
    Publisher *xbox_pub;
    pub_Xbox_Data xbox_data;
    XboxControllerData_t *xbox_msgs;
}XBOX_Instance_t;
/*---------------------------------function-----------------------------------*/


/**
 * @brief 初始化Xbox手柄
 *  
 * @param gpio_instance
 * @return uint8_t
 */
uint8_t Xbox_Init(Uart_Instance_t *xbox_uart_instance);

/**
 * @brief 串口中断处理函数
 * 
 * @param byte 
 * @return uint8_t 
 */
uint8_t Xbox_Process(uint8_t byte);

/**
 * @brief 串口接收回调函数
 * 
 * @param uart_instance 
 * @return uint8_t 
 */
uint8_t Xbox_Uart_Rx_Callback(Uart_Instance_t *uart_instance, uint16_t data_len);

/**
 * @brief 判断按键是否为长按
 * 
 * @param 当前按键状态 
 * @param 上一时刻按键状态
 * @return bool 
 */
bool Button_Continue(bool currentBtnState, bool *lastBtnState);

/**
 * @brief 判断按键是否为点击
 * 
 * @param 当前按键状态 
 * @param 上一时刻按键状态
 * @return bool 
 */
bool Button_Switch(bool currentBtnState, bool *lastBtnState);

/**
 * @brief Xbox数据赋值和处理
 * 
 * @param xbox_datas 
 * @param controllerData 
 */
void Xbox_Get_Data(uint8_t *xbox_datas, XBOX_Instance_t *Instance);

/**
 * @brief Xbox数据发布
 * 
 * @return uint8_t 
 */
uint8_t Xbox_Publish();



#ifdef __cplusplus
}
#endif

#endif /* XBOX_H */ 