/**
 * @file motor_new.h  --adapted from Yang JianYi
 * @author Keten (2863861004@qq.com)
 * @brief  电机封装的优化，目前打算将某些数据存进CCMRAM，提高读写能力，以及将部分函数进行inline，减少函数调用的开销
 *         10-05终于完成bsp_can层封装，开始进行motor和bsp_can的联系
 * @version 0.1
 * @date 2024-10-02
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention : 注意inline 关键字不会自动继承，所以需要自行在派生类中添加inline声明
 *              在基类中的每一个inline声明都是提醒你，当你在派生类中重写这个函数时，必须使用inline声明，减少函数调用开销
 *              存进CCMRAM的操作，直接对整个类的实例进行操作即可，经过试验不可以在类内进行数据存进CCMRAM的操作
 *              对于派生类中重载基类的函数，请务必加上 override 声明，以便于阅读
 *              对于不会修改的变量，请加上 const 声明，以便于阅读，这是一种声明，也是一种约定
 * 
 * @note :      有点莫名其妙的，类的创建最好都放在头文件最上面，不然编译器会报错...奇奇怪怪的
 * @versioninfo :
 */
#pragma once

#include <stdint.h>
#include "ccmram.h"
#include "stdbool.h"
#include "data_pool.h"
#include "bsp_can.h"

#ifdef __cplusplus

/*----------------------------------class-----------------------------------*/
/**
 * @brief 电机基类
 *        基本不需要private部分，因为属于基类，不需要拥有自己的太多东西
 *        需要保护的、被派生类重载的 ---> protected
 *        可被外部调用的，用或不用被派生类重载的 ---> private
 */
class Motor
{
public:
    Motor(uint8_t id, 
          const CAN_Rx_Instance_t& can_rx_instance,const CAN_Tx_Instance_t& can_tx_instance,
          float max_current,uint8_t reduction_ratio) 
        : ID(id), 
          can_tx_for_motor{can_tx_instance},can_rx_for_motor{can_rx_instance},
          max_current(max_current), motor_reduction_ratio(reduction_ratio){}
    virtual ~Motor(){}
    const uint8_t ID = 0;

    /* 电机数值更新 */
    virtual void update(uint8_t can_rx_data[]) = 0;

    /* 可供调用的外部接口 */
    inline float get_angle() const { return angle; }// 获取当前电机角度
    inline float get_encoder() const { return encoder; }// 获取当前电机编码器数值
    inline float get_speed() const {return speed; }// 获取当前电机转速


     float encoder_offset;// 编码器初始偏移值
     float Out;// 输出值
    /* 声明can句柄，表明这里是一个can设备 */
     CAN_Tx_Instance_t can_tx_for_motor = {0};
     CAN_Rx_Instance_t can_rx_for_motor = {0};
protected:
    /* 编码器相关 */
     uint16_t encoder = 0;// 编码器位置
     float last_encoder = 0;// 上一次编码器位置
     bool encoder_is_init = false;// 编码器是否初始化标志位
    /* 角度相关 */
     float angle = 0;// 电机角度
    /* 速度相关 */
     int16_t speed = 0;
public:
    /* 核心待重载函数---须经具体电机种类型号进行重载 */
    virtual void CanMsg_Process(CAN_Tx_Instance_t &can_txmsg) = 0;// can帧接收数据解包
    inline virtual void update_angle(uint8_t can_rx_data[]){}// 更新电机角度
    inline virtual void update_speed(uint8_t can_rx_data[]){}// 更新电机速度
protected:
    /* 基类的私有变量，初始化按的大疆电机初始化来的 */
     uint16_t round_cnt = 0;// 编码器圈数
     int16_t encoder_max = 8192;//  
     float encoder_angle_ratio = 8192.0f / 360.0f;// 编码器角度比
     float max_current = 65535;// 最大电流
     uint8_t motor_reduction_ratio = 36;// 电机减速比

};

/*----------------------------------Template-----------------------------------*/

/**
 * @brief     电机输出限幅
 * 
 * @tparam T  为电机输出的数据类型
 * @param val 为电机输出的数据
 * @param min 为电机输出的最小值
 * @param max 为电机输出的最大值
 */
template <typename T>
inline void motor_constraint(T *val, T min, T max)
{
    if(*val > max)
    {
        *val = max;
    }
    else if(*val < min)
    {
        *val = min;
    }
}

#endif

