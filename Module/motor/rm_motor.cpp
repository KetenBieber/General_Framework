/**
 * @file rm_motor.cpp
 * @author Keten (2863861004@qq.com)
 * @brief 大疆电机的内环实现
 * @version 0.1
 * @date 2024-10-08
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention :
 * @note :
 * @versioninfo :
 * @todo :使用pid_controller ,将内环pid计算封装，露出三种方法：开环、速度环、位置环
 *        计划加入前馈控制选项！
 *        实现外部app使用电机设备时，只需传入ref参考值
 *        外部app调用为外部环，内部pid计算为内环
 *        但是这样设计的话，需要注意一个问题，就是实际参考速度向转子角速度的转换需要在各自app中独立完成，也就是实际上有没有用减速箱，有没有换别的减速箱
 *        这些都由app的调用者决定，你在motor得到的都是转子速度（当然也暴露了官方的减速箱减速之后的速度）
 *        也就是说，最终调用的set_motor_ref（必须是转子的期望速度）
 */
#include "rm_motor.h"
#include "motor_interface.h"
#include "pid_controller.h"

void RM_Common::Motor_Ctrl(float ref)
{
    this->enable_the_motor();
    this->set_motor_ref(ref);
    this->pid_control_to_motor();
}


/**
 * @brief 设定电机参考量 
 *        外环为角度控制模式时 单位：角度（总角度：无限）
 *        外环为速度控制模式时 单位：角速度 rad/min 度每秒
 * 
 * @param ref 
 */
void RM_Common::set_motor_ref(float ref)
{
    this->ctrl_motor_config.motor_controller_setting.pid_ref = ref;
}


/**
 * @brief 失能电机
 * 
 */
void RM_Common::stop_the_motor()
{
    this->ctrl_motor_config.motor_working_status = MOTOR_STOP;
}


/**
 * @brief 使能电机
 * 
 */
void RM_Common::enable_the_motor()
{
    this->ctrl_motor_config.motor_working_status = MOTOR_ENABLED;
}

float ref_in = 0;
/**
 * @brief 电机内环计算函数，适用于各种大疆电机
 *        目标就是计算Out值，这个值是电流值，单位为mA
 * 
 */
void RM_Common::pid_control_to_motor() 
{
    if(this->ctrl_motor_config.motor_working_status == MOTOR_STOP)
    {
        /* 电机失能，直接让输出电流为0 */
        this->Out = 0;
        return;
    }
    float pid_ref,pid_measure;
    pid_ref = this->ctrl_motor_config.motor_controller_setting.pid_ref;

    if(this->ctrl_motor_config.motor_is_reverse_flag == MOTOR_DIRECTION_REVERSE)
    {
       pid_ref *=-1;
    }

    /* 
        常见搭配：外位置环+内速度环 = outer 为位置环 inner 为速度环
                单速度环 = out 和 inner 都为速度环
                单位置环 = out 和 inner 都为位置环
     */

    /* pid_ref会顺次通过被启用的闭环充当数据的载体 */
    /* 位置环计算，只要外环设置为位置环，就执行位置环 */
    if(this->ctrl_motor_config.outer_loop_type & ANGLE_LOOP)
    {
        pid_measure = this->angle;
        pid_ref = PID_Calculate(&this->ctrl_motor_config.motor_controller_setting.angle_PID,pid_measure,pid_ref);
    }

    /* 速度环计算，只要内环设置为速度环，就执行速度环 */
    if(this->ctrl_motor_config.inner_loop_type & SPEED_LOOP)
    {
        pid_measure = this->speed_aps;
        pid_ref = PID_Calculate(&this->ctrl_motor_config.motor_controller_setting.speed_PID,pid_measure,pid_ref);
    }
    this->Out = this->aps_to_current(pid_ref);
    ref_in = this->Out;
}








