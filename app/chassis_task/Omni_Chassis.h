/**
 * @file Omni_Chassis.h
 * @author Keten (2863861004@qq.com)
 * @brief 全向轮系底盘
 * @version 0.1
 * @date 2024-10-10
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention :
 * @note :
 * @versioninfo :
 */
#pragma once
#include "Chassis.h"

#ifdef __cplusplus

class Omni_Chassis :public Chassis
{
public:
    Omni_Chassis(uint8_t wheel_num,float wheel_radius) :Chassis(wheel_num,wheel_radius){}
    virtual ~Omni_Chassis(){};

    virtual uint8_t Chassis_PID_Init() override;
    virtual void Kinematics_Inverse_Resolution() override;
    virtual void Kinematics_forward_Resolution() override;
    virtual void Dynamics_Inverse_Resolution() override;

    float APS_Wheel[4]={0};// 全向轮底盘最多是4个轮子,这里存储4个电机的参考角速度输出

};


#endif




