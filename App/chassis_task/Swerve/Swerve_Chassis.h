/**
 * @file Swerve_Chassis.h
 * @author Keten (2863861004@qq.com)
 * @brief 舵轮底盘控制类
 * @version 0.1
 * @date 2025-01-07
 * 
 * @copyright Copyright (c) 2025
 * 
 * @attention :
 * @note :舵轮底盘基类，请创建具体底盘类型的实例使用
 *        人和底盘朝向同向，底盘根据前x、左y的机器人坐标系，有lf、lb、rb、rf四个舵轮（序号也是这个顺序）
 *        （若为非正方形底盘）定义lf、rf在一条length上，lf和lb在一条width
 *        舵向0度指向机器人坐标系x轴正方向
 *        -90度指向为机器人坐标系的y轴正方向
 *        90度指向为机器人坐标系的y轴负方向
 *        舵轮底盘控制器输出：
 *          舵向角度 0~360
 *          轮向速度 m/s
 *          提供舵轮的控制模式
 *          刹车加速度限制？
 *          梯形加减速限制？
 *        需要的输入：
 *          实际底盘速度反馈
 *          实际舵向反馈
 *        
 * @todo: 1.添加舵轮底盘的运动学逆解算
 *        2.添加舵轮底盘的运动学正解算
 *        3.底盘驻车模式
 *        4.底盘速度外环
 * @versioninfo :
 */
#pragma once

#include "Chassis.h"
#include "arm_math.h"

#ifdef __cplusplus
extern "C"{
#endif
/*-----------------------------------macro------------------------------------*/

/*----------------------------------typedef-----------------------------------*/

// 舵轮模块结构体
typedef struct
{
    uint8_t wheel_num;// 轮子ID
    float wheel_cmd;// 轮向电机速度输出
    float rubber_angle_cmd;// 舵向角度输出
    float rubber_angle_current;// 当前舵向角度
    float rubber_angle_last;// 上一次舵向角度命令
}Swerve_Module_t;
/*----------------------------------variable----------------------------------*/


#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
class Swerve_Chassis :public Chassis
{
public:

    Swerve_Chassis(size_t wheel_num,float wheel_radius,float wheel_to_center) :Chassis(wheel_num,wheel_radius),Wheel_To_Center(wheel_to_center){};
    virtual ~Swerve_Chassis() = default;

    virtual void Chassis_TrackingController_Init() override;
    virtual float Kinematics_Inverse_Resolution(size_t count,Robot_Twist_t ref_twist) override;
    virtual void Kinematics_forward_Resolution(float wheel_1,float wheel_2,float wheel_3,float wheel_4);
    virtual void Dynamics_Inverse_Resolution() override;
    virtual void Chassis_Parking_Control() override;
    virtual void Chassis_Reset_Output() override;

public:

    Swerve_Module_t _swerve_module[4] = {0};
    void RubberAngle_Adjust(Swerve_Module_t &swerve);
    void update_Swerve_Module(float rubber_1,float rubber_2,float rubber_3,float rubber_4);// 更新舵轮模块数据，在舵轮控制之前调用

    // 订阅数据
protected:

    float Wheel_To_Center = 0;// 轮子相对底盘中心的距离,底盘半径

private:    

    /**
     * @brief 返回归一到
     * 
     * @param total_angle_current 传入当前舵向的总角度
     * @return float 
     */
    float transfer_total_angle_to_single_angle(float total_angle_current);
    /**
     * @brief 将目标舵向单圈角度转换成目标舵向总角度，满足电机的位置控制需求
     * 
     * @param total_round_current 传入当前舵向的总角度
     * @param single_round_cmd 传入目标舵向单圈角度
     */
    float transfer_single_angle_to_total_angle(float total_angle_current,float single_angle_cmd);


private:
    float frame_length = 0.5f;// 底盘长度
    float frame_width = 0.5f;// 底盘宽度    
    int N;// 记录舵向转过的角度

    float rubber_current_angle[4] = {0};
};  

#endif