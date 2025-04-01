/**
 * @file DM_motor.cpp
 * @author 冯大帅将军 
 * @brief 达妙电机控制实现
 * 
 * **************************************************************************
 * 使用说明：
 * 达妙电机一共三种工作模式：
 * 1.位置速度控制：POS_with_SPEED_CONTROL
 * 2.速度控制：SPEED_CONTROL
 * 3.位置电流控制：MIT_CONTROL
 * 第一种控制模式：
 * 位置速度控制，输入目标位置和目标速度，电机会根据目标位置和目标速度进行控制
 * 第二种控制模式：
 * 速度控制，输入目标速度，电机会根据目标速度进行控制
 * 第三种控制模式：
 * 位置电流控制，输入目标位置、目标速度、目标力矩、位置PID系数、速度PID系数，
 * 这三种控制模式输入的目标位置、目标速度单位为rad和rad/s
 * 电机会根据目标位置、目标速度、目标电流、位置PID系数、速度PID系数进行控制
 * 每次切换不同模式都要给电机重新上电，否则不会生效
 * 目前我只测试了位置速度控制，其他两种控制模式没有测试不知道能不能用
 * 还有三条指令函数：
 * 1.使能电机：DM_MOTOR_ENABLE，我已经写好，创建电机实例自动转载使能信息发送
 * 2.停止电机：DM_MOTOR_DISABLE也就是失能电机
 * 3.设置电机零点：DM_MOTOR_ZERO_POSITION设置当前位置为电机零点
 * PS.返回值非常奇怪，反正也没用。。。
 * ****************************************************************************
 * @version 0.1
 * @date 2025-4-01
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention :
 * @note :
 * @versioninfo :
 */
#include "DM_motor.h"
#include "motor_interface.h"

void DM_motor::POS_with_SPEED_CONTROL(float pos, float speed)
{
    this->speed=speed;
    this->pos=pos;
    return;
}

void DM_motor::SPEED_CONTROL(float speed)
{
    this->speed=speed;
    return;
}

void DM_motor::MIT_CONTROL(float speed, float pos, float torque,float Kp, float Kd)
{
    this->speed=speed;
    this->pos=pos;
    this->torque=torque;
    this->Kp=Kp;
    this->Kd=Kd;
    return;
}

void DM_motor::set_motor_ref(float ref)
{
    return;
}

void DM_motor::stop_the_motor()
{
    return;
}

void DM_motor::enable_the_motor()
{
    return;
}

void DM_motor::pid_control_to_motor()
{
    return;
}