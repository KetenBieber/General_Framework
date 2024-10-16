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

#ifdef __cplusplus
#include "rm_motor.h"
#include "Chassis.h"


class Omni_Chassis :public Chassis
{
public:
    Omni_Chassis(size_t wheel_num,float wheel_radius,float wheel_to_center) :Chassis(wheel_num,wheel_radius),Wheel_To_Center(wheel_to_center){}
    virtual ~Omni_Chassis() = default;

    virtual float Kinematics_Inverse_Resolution(size_t count,Robot_Twist_t ref_twist) override;
    virtual void Kinematics_forward_Resolution(float wheel_1,float wheel_2,float wheel_3,float wheel_4);
    virtual void Dynamics_Inverse_Resolution() override;
    
    float Wheel_To_Center = 0;// 轮子相对底盘中心的距离,底盘半径

    float Wheel_Azimuth[4] = {
        45*DEGREE_2_RAD,
        135*DEGREE_2_RAD,
        225*DEGREE_2_RAD,
        315*DEGREE_2_RAD
    };// 轮子相对底盘中心的方位角

};

#endif
#ifdef __cplusplus
extern "C"{
#endif
#define ONETHIRD            0.333333333f
#define TWOTHIRD            0.666666667f
#define SQR3_OF_THIRD       0.577350269f


#ifdef __cplusplus
}
#endif




