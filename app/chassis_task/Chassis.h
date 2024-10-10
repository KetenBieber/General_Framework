/**
 * @file Chassis.h
 * @author Keten (2863861004@qq.com)
 * @brief 底盘运动学，使用扭矩控制 - - - 力控底盘
 * @version 0.1
 * @date 2024-10-10
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention :
 * @note :此为底盘基类.h，请创建具体底盘类型的实例使用，其他类型的底盘均已提供对应的底盘解算以及动力学解算
 *        底盘进行机器人坐标系和世界坐标系的灵活转换的前提是---获取了机器人的姿态，请确保机器人在posture层已经接入了姿态传感器的接口
 *                
 * @versioninfo :
 */
#pragma once

/*----------------------------------include-----------------------------------*/
#include "arm_math.h"
#include "robot_def.h"
#include "motor_base.h"
#include "pid_controller.h"
#include "topics.h"

#ifdef __cplusplus

/* 底盘基类 */
class Chassis 
{
public:
    Chassis(uint8_t wheel_num,float wheel_radius) : Wheel_Num(wheel_num),Wheel_Radius(wheel_radius){}
    virtual ~Chassis(){}

    uint8_t Wheel_Num = 0;
    float Wheel_Radius = 0;
      
protected:

    /* 底盘速度值PID */
    PID_t Chassis_PID_X;
    PID_t Chassis_PID_Y;
    PID_t Chassis_PID_Omega;

    virtual uint8_t Chassis_PID_Init(){}

    /* 获取当前底盘姿态，必须重写，否则无法进行机器人和世界坐标系的转换 */
    virtual void Get_Current_Posture(){}

    /* 运动学逆解算，由派生类重写 */
    virtual void Kinematics_Inverse_Resolution(){}
    /* 运动学正解算,由派生类重写 */
    virtual void Kinematics_forward_Resolution(){}
    /* 动力学，扭矩计算 */
    void Output_To_Dynamics();
    /* 动力学，扭矩分配 */
    virtual void Dynamics_Inverse_Resolution(){}


    /* 设置期望速度 */
    virtual void Set_Target_Velocity_X(){}
    virtual void Set_Target_Velocity_Y(){}
    virtual void Set_Target_Velocity_Omega(){}

    float Velocity_X = 0;
    float Velocity_Y = 0;
    float Velocity_Omega = 0;

    

};

extern "C"{
#endif

/*-----------------------------------macro------------------------------------*/
#define PI                          3.1415926535f         // 圆周率

#define WHEEL_TO_MOTOR              RAD_2_DEGREE/(WHEEL_R*MOTOR_REDUCTION_RATIO)     //将车身速度逆解算得到的驱动轮速度向电机（经过减速箱之后的）的转子的角速度，单位为 度/s
#define MOTOR_TO_WHEEL              WHEEL_R*MOTOR_REDUCTION_RATIO/RAD_2_DEGREE  // 将转子角速度换算到轮子实际线速度
#define SPEED_TO_RPM                60/(2*PI*WHEEL_R) // 将m/s转换为rpm/s
#define RPM_TO_SPEED                2*PI*WHEEL_R/60   // 将rpm/s转换为m/s

/*----------------------------------typedef-----------------------------------*/

/* 机器人坐标系结构体定义 */
typedef struct
{
    /* 平动速度 */
    float linear_x;
    float linear_y;
    /* 转动速度 */
    float angular_x;
}Robot_Twist_t;


/*----------------------------------function----------------------------------*/


#ifdef __cplusplus
}
#endif

