/**
 * @file Omni_Chassis.h
 * @author Keten (2863861004@qq.com)
 * @brief 全向轮系底盘
 * @version 0.1
 * @date 2024-10-10
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention :最经典的顺时针旋转（角度为0~360），如果为四全向轮的话，则lf、rf、rb、lb的顺序分别为1、2、3、4
 *             如果为三全向轮的话，则默认顺时针从上到下1、2、3
 *                      *
 *                     * *
 * @note :
 * @versioninfo :
 */
#pragma once
#include "Chassis.h"

#ifdef __cplusplus
#include "rm_motor.h"

class Omni_Chassis :public Chassis
{
public:
    Omni_Chassis(size_t wheel_num,float wheel_radius,float wheel_to_center) 
    :Chassis(wheel_num,wheel_radius),Wheel_To_Center(wheel_to_center){}
    virtual ~Omni_Chassis(){};

    virtual void Kinematics_Inverse_Resolution(Robot_Twist_t ref_twist) override;
    virtual Robot_Twist_t Kinematics_forward_Resolution() override;
    virtual Robot_Twist_t RoboSpeed_To_WorldSpeed() override;
    virtual void Dynamics_Inverse_Resolution() override;

    float APS_Wheel[4]={0};// 全向轮底盘最多是4个轮子,这里存储4个电机角速度的参考角速度输出

    float Wheel_To_Center = 0;
    
};


#endif




