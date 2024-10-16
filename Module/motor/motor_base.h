/**
 * @file motor_new.h  --adapted from Yang JianYi
 * @author Keten (2863861004@qq.com)
 * @brief  电机封装的优化，目前打算将某些数据存进CCMRAM，提高读写能力，以及将部分函数进行inline，减少函数调用的开销
 *         10-05终于完成bsp_can层封装，开始进行motor和bsp_can的联系
 *         
 * @version 0.1
 * @date 2024-10-02
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention : 注意inline 关键字不会自动继承，所以需要自行在派生类中添加inline声明
 *              在基类中的每一个inline声明都是提醒你，当你在派生类中重写这个函数时，必须使用inline声明，减少函数调用开销
 *              !!!存进CCMRAM的操作，直接对整个类的实例进行操作即可，经过试验不可以在类内进行数据存进CCMRAM的操作
 *              对于派生类中重载基类的函数，请务必加上 override 声明，以便于阅读
 *              对于不会修改的变量，请加上 const 声明，以便于阅读，这是一种声明，也是一种约定
 * 
 * @note :      
 * @versioninfo :
 * @todo : 1.为电机类添加新成员：mesure_data，mesure_data将用于内环pid计算
 *         并且使用指数平滑滤波器对电调返回数据做适当平滑处理，将其存入mesure_data
 *         更新机制放在update方法中
 *         2.明确各模块职责，这里只是电机封装，但是电机并非只是用来充当底盘的动力系统
 *          电机可以是其他的机构，所以这里不做对什么轮子做处理,一切均为以转子刻画的速度以及减速箱减速下的速度
 */
#pragma once

#include <stdint.h>
#include "ccmram.h"
#include "stdbool.h"
#include "data_pool.h"
#include "bsp_can.h"
#include "filter.h"
#include "pid_controller.h"

#ifdef __cplusplus
extern "C"{
#endif

#define RAD_2_DEGREE                        57.2957795f    // 180/pi  角度转换成弧度
#define DEGREE_2_RAD                        0.01745329252f // pi/180  弧度转化位角度
#define RPM_PER_MIN_2_ANGLE_PER_SEC         6.0f        // 360/60,转每分钟 转化 度每秒    
#define RPM_PER_MIN_2_RAD_PER_SEC           0.104719755f // ×2pi/60sec,转每分钟 转化 弧度每秒

/**
 * @brief 闭环类型，如果需要多个闭环，使用 | 运算
 *        注意m3508、m2006内置电流环
 * 
 */
typedef enum
{
    OPEN_LOOP = 0b0000,
    CURRENT_LOOP = 0b0001,
    SPEED_LOOP = 0b0010,
    ANGLE_LOOP = 0b0100,
}Close_Loop_Type_e;

typedef enum
{
    MOTOR_DIRECTION_NORMAL = 0,
    MOTOR_DIRECTION_REVERSE = 1
}Motor_Reverse_Flag_e;


typedef enum
{
    MOTOR_STOP = 0,
    MOTOR_ENABLED = 1,
}Motor_Working_Type_e;


typedef struct 
{
    float *other_angle_feedback_ptr;// 角度反馈数据指针
    float *other_speed_feedback_ptr;// 速度反馈数据指针

    PID_t speed_PID;// 速度环
    PID_t angle_PID;// 位置环（或者叫角度环也可，避免和位置式pid混淆）
    
    float pid_ref;// 将会作为每一环的pid输出，一环扣一环
}Motor_PIDController_Setting_t;


/**
 * @brief 电机内环配置，每一个创建出来的电机，都需要配置好下面这些选项
 * 
 */
typedef struct
{
    Motor_PIDController_Setting_t motor_controller_setting;// 电机内环pid配置
    Close_Loop_Type_e outer_loop_type;// 外层闭环类型
    Close_Loop_Type_e inner_loop_type;// 内层闭环类型
    Motor_Reverse_Flag_e motor_is_reverse_flag;// 电机是否要反转
    Motor_Working_Type_e motor_working_status;// 电机状态

}Motor_Control_Setting_t;



#ifdef __cplusplus
}

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
          const Motor_Control_Setting_t& ctrl_config,
          int16_t max_current,uint8_t reduction_ratio) 
        : ID(id), 
          can_tx_for_motor{can_tx_instance},can_rx_for_motor{can_rx_instance},
          ctrl_motor_config(ctrl_config),
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
     int16_t Out = 0.0;// 输出值,为电流值 单位：mA
    /* 声明can句柄，表明这里是一个can设备 */
     CAN_Tx_Instance_t can_tx_for_motor = {0};
     CAN_Rx_Instance_t can_rx_for_motor = {0};

     Motor_Control_Setting_t ctrl_motor_config = {0};
     virtual void set_motor_ref(float ref)= 0;// 设置电机参考值
     virtual void stop_the_motor() = 0;// 急停电机
     virtual void enable_the_motor() = 0;// 使能电机
     virtual void pid_control_to_motor() = 0;// 电机pid计算out输出
     
public:
    /* 编码器相关 */
     uint16_t encoder = 0;// 编码器位置
     float last_encoder = 0;// 上一次编码器位置
     bool encoder_is_init = false;// 编码器是否初始化标志位
    /* 角度相关 */
     float angle = 0;// 电机角度
    /* 速度相关 */
     float speed = 0;// 转子速度
     float speed_aps = 0;// 转子角速度 
     inline float get_speed_after_low(){ return this->speed / motor_reduction_ratio; }// 获取减速箱操作之后的速度
     inline float get_speed_aps_after_low(){ return this->speed_aps / motor_reduction_ratio; }// 获取减速箱操作之后的角速度
    /* 电机当前电流值 */
     uint8_t motor_current = 0;
     /* 电机当前温度 */
     int16_t motor_temperature = 0;


public:

    /* 核心待重载函数---须经具体电机种类型号进行重载 */
    virtual void CanMsg_Process(CAN_Tx_Instance_t &can_txmsg) = 0;// can帧接收数据解包
    inline virtual void update_angle(uint8_t can_rx_data[]){}// 更新电机角度
    inline virtual void update_speed(uint8_t can_rx_data[]){}// 更新电机速度
    inline virtual void update_speed_aps(){}// 更新电机角速度,依赖更新电机速度，要在更新电机速度后才能调用
    inline virtual void update_current(uint8_t can_rx_data[]){}// 更新电机电流值
    inline virtual void updata_temperature(uint8_t can_rx_data[]){}// 更新电机温度
protected:
    /* 基类的保护变量，初始化按的大疆电机初始化来的 */
     float total_angle = 0;// 总角度，注意方向
     uint32_t round_cnt = 0;// 编码器圈数
     int16_t encoder_max = 8192;//  
     float encoder_angle_ratio = 8192.0f / 360.0f;// 编码器角度比
     int16_t max_current = 0;// 最大电流
     uint8_t motor_reduction_ratio = 0;// 电机减速比

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


