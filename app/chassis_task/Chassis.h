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
 *        注意使用坐标系为
 *          前x、左y       
 *        机器人使用pub-sub获取姿态数据，订阅chassis_imu_pub 获取 yaw角数据
 *                                     订阅chassis_pos_pub 获取位置数据（全局）
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
    Chassis(size_t wheel_num,float wheel_radius) : Wheel_Num(wheel_num),Wheel_Radius(wheel_radius){}
    virtual ~Chassis(){}

    uint8_t Wheel_Num = 0;
    float Wheel_Radius = 0;
    /* 底盘速度值PID */
    PID_t Chassis_PID_X;
    PID_t Chassis_PID_Y;
    PID_t Chassis_PID_Omega;

    /* 底盘所需pub-sub操作的初始化，必须调用 */
    uint8_t Chassis_Subscribe_Init()
    {
        this->chassis_imu_sub = register_sub("chassis_imu_pub",1);
        this->chassis_pos_sub = register_sub("chassis_pos_pub",1);
        this->imu_data = (pub_imu_yaw*)pvPortMalloc(sizeof(pub_imu_yaw));
        assert_param(this->imu_data != NULL);
        this->pos_data = (pub_Chassis_Pos*)pvPortMalloc(sizeof(pub_Chassis_Pos)); 
        assert_param(this->pos_data != NULL);
    }

    /* 获取当前底盘姿态，必须重写，否则无法进行机器人和世界坐标系的转换 */
    void Get_Current_Posture()
    {
        publish_data chassis_imu_data;
        chassis_imu_data = this->chassis_imu_sub->getdata(this->chassis_imu_sub);
        if(chassis_imu_data.len != -1)
        {
            imu_data = (pub_imu_yaw*)(chassis_imu_data.data);
        }
    }
    void Get_Current_Position()
    {
        publish_data chassis_pos_data;
        chassis_pos_data = this->chassis_imu_sub->getdata(this->chassis_pos_sub);
        if(chassis_pos_data.len != -1)
        {
            pos_data = (pub_Chassis_Pos*)(chassis_pos_data.data);
        }
    }
    void Output_To_Dynamics();
protected:

    /* 运动学逆解算，由派生类重写 */
    virtual void Kinematics_Inverse_Resolution(Robot_Twist_t ref_twist){}
    /* 运动学正解算,由派生类重写 */
    virtual Robot_Twist_t Kinematics_forward_Resolution(){}
    /* 机器人坐标系向世界坐标系转换 */
    virtual Robot_Twist_t RoboSpeed_To_WorldSpeed(){}
    /* 动力学，扭矩计算 */
    /* 动力学，扭矩分配 */
    virtual void Dynamics_Inverse_Resolution(){}

    /* 设置期望速度 */
    virtual void Set_Target_Velocity_X(){}
    virtual void Set_Target_Velocity_Y(){}
    virtual void Set_Target_Velocity_Omega(){}

public:
    Robot_Twist_t RoboSpeed = {0};// 机器人坐标系下速度
    Robot_Twist_t WorldSpeed = {0};// 世界坐标系下速度

    Robot_Twist_t Ref_RoboSpeed = {0};// 期望机器人坐标系下速度
    Robot_Twist_t Ref_WorldSpeed = {0};// 期望世界坐标系下速度

    float posture_robot_angle = 0;

    /* 订阅发布 */
    Subscriber* chassis_imu_sub;
    Subscriber* chassis_pos_sub; 

    /* 订阅数据类型指针 */
    pub_imu_yaw *imu_data;// 调用它来读yaw值
    pub_Chassis_Pos *pos_data;// 调用它来读位置值

};

extern "C"{
#endif

/*-----------------------------------macro------------------------------------*/
#define PI                          3.1415926535f         // 圆周率

#define WHEEL_TO_MOTOR              RAD_2_DEGREE/(WHEEL_R*MOTOR_REDUCTION_RATIO)     //将车身速度逆解算得到的驱动轮速度向（经过减速箱之后的）电机的转子的角速度，单位为 度/s
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
    float omega;
}Robot_Twist_t;


/*----------------------------------function----------------------------------*/


#ifdef __cplusplus
}
#endif

