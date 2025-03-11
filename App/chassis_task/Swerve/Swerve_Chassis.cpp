/**
 * @file Swerve_Chassis.cpp
 * @author Keten (2863861004@qq.com)
 * @brief 
 * @version 0.1
 * @date 2025-01-07
 * 
 * @copyright Copyright (c) 2025
 * 
 * @attention :
 * @note :
 * @versioninfo :
 */
#include "Swerve_Chassis.h"

void Swerve_Chassis::Chassis_TrackingController_Init()
{


}

// 运动学逆解
// 作用就是改变对应的舵轮角度和速度
float Swerve_Chassis::Kinematics_Inverse_Resolution(size_t count,Robot_Twist_t ref_twist)
{
    // 轮系解算，对单个舵轮进行速度分解
    float wheel_vx = 0.0f;
    float wheel_vy = 0.0f;
    if(this->Wheel_Num == 4)// 四舵轮解算
    {
        switch(count)
        {
            case 0:
                wheel_vx = ref_twist.linear_x - this->Wheel_To_Center * (frame_length/2.0f) * ref_twist.omega;
                wheel_vy = ref_twist.linear_y + this->Wheel_To_Center * (frame_width/2.0f) * ref_twist.omega;
                break;
            case 1:
                wheel_vx = ref_twist.linear_x - this->Wheel_To_Center * (frame_length/2.0f) * ref_twist.omega;
                wheel_vy = ref_twist.linear_y - this->Wheel_To_Center * (frame_width/2.0f) * ref_twist.omega;
                break;
            case 2:
                wheel_vx = ref_twist.linear_x + this->Wheel_To_Center * (frame_length/2.0f) * ref_twist.omega;
                wheel_vy = ref_twist.linear_y - this->Wheel_To_Center * (frame_width/2.0f) * ref_twist.omega;
                break;
            case 3:
                wheel_vx = ref_twist.linear_x + this->Wheel_To_Center * (frame_length/2.0f) * ref_twist.omega;
                wheel_vy = ref_twist.linear_y + this->Wheel_To_Center * (frame_width/2.0f) * ref_twist.omega;
                break;
        }
        arm_sqrt_f32(wheel_vx*wheel_vx + wheel_vy*wheel_vy,&this->_swerve_module[count].wheel_cmd);
        if(this->_swerve_module[count].wheel_cmd == 0) 
            // 如果线速度为0，则保持当前舵向角度
            this->_swerve_module[count].rubber_angle_cmd = _swerve_module[count].rubber_angle_last;
        else
            this->_swerve_module[count].rubber_angle_cmd = atan2f(wheel_vy,wheel_vx)*RAD_2_DEGREE;

        this->_swerve_module[count].rubber_angle_last = this->_swerve_module[count].rubber_angle_cmd;

        this->RubberAngle_Adjust(_swerve_module[count]);
    }
    else // 三舵轮解算
    {
        // todo...

    }
    return this->_swerve_module[count].wheel_cmd;
}


// 运动学正解
void Swerve_Chassis::Kinematics_forward_Resolution(float wheel_1,float wheel_2,float wheel_3,float wheel_4)
{
    float vx = 0.0f;
    float vy = 0.0f;
    float M1 = 0,M2 = 0,M3 = 0,M4 = 0;
    float rubber_1 = _swerve_module[0].rubber_angle_current * DEGREE_2_RAD;
    float rubber_2 = _swerve_module[1].rubber_angle_current * DEGREE_2_RAD;
    float rubber_3 = _swerve_module[2].rubber_angle_current * DEGREE_2_RAD;
    float rubber_4 = _swerve_module[3].rubber_angle_current * DEGREE_2_RAD;
    static float C  = (frame_length*frame_length + frame_width*frame_width);
    if(this->Wheel_Num == 4)
    {

        vx = (wheel_1 * arm_cos_f32(rubber_1) + 
            wheel_2 * arm_cos_f32(rubber_2) +
            wheel_3 * arm_cos_f32(rubber_3) +
            wheel_4 * arm_cos_f32(rubber_4))/ 4.0f;
        vy = (wheel_1 * arm_sin_f32(rubber_1) +
            wheel_2 * arm_sin_f32(rubber_2) + 
            wheel_3 * arm_sin_f32(rubber_3) +
            wheel_4 * arm_sin_f32(rubber_4))/ 4.0f;
            // 四舵轮正解算
        M1 = ((+frame_width) * arm_sin_f32(rubber_1) - frame_length  * arm_cos_f32(rubber_1)) / C * wheel_1; // LF
        M2 = ((-frame_width) * arm_sin_f32(rubber_2) - frame_length  * arm_cos_f32(rubber_2)) / C * wheel_2; // LB
        M3 = ((-frame_width) * arm_sin_f32(rubber_3) + frame_length  * arm_cos_f32(rubber_3)) / C * wheel_3; // RB
        M4 = ((+frame_width) * arm_sin_f32(rubber_4) + frame_length  * arm_cos_f32(rubber_4)) / C * wheel_4; // RF

        float wz = (M1 + M2 + M3 + M4) / 4.0f;

        this->self_RoboSpeed.linear_x = vx;
        this->self_RoboSpeed.linear_y = vy;
        this->self_RoboSpeed.omega = wz;

        // 再转换一下得到世界坐标系下速度
        this->RoboTwist_To_WorldTwist(this->self_RoboSpeed,this->self_WorldSpeed);
    }
    else
    {
        // 三舵轮正解算

    }
}


// 动力学解算，对机器人整体速度进行闭环
void Swerve_Chassis::Dynamics_Inverse_Resolution()
{
    float force_x = 0,force_y = 0,torque_omega = 0;
    if (this->Chassis_Status == CHASSIS_STOP)
    {
        this->ref_twist.linear_x = force_x;
        this->ref_twist.linear_y = force_y;
        this->ref_twist.omega = torque_omega;
        return;
    }
    // 如果是机器人坐标系下操作，则采集机器人坐标系下速度进行闭环
    if(this->Chassis_Status == ROBOT_CHASSIS)
    {
        force_x = PID_Calculate(&this->Chassis_PID_X,this->RoboSpeed.linear_x,this->Ref_RoboSpeed.linear_x);
        force_y = PID_Calculate(&this->Chassis_PID_Y,this->RoboSpeed.linear_y,this->Ref_RoboSpeed.linear_y);
        torque_omega = PID_Calculate(&this->Chassis_PID_Omega,this->RoboSpeed.omega,this->Ref_RoboSpeed.omega);
    }
    else// 世界坐标系
    {
        force_x = PID_Calculate(&this->Chassis_PID_X,this->WorldSpeed.linear_x,this->Ref_WorldSpeed.linear_x);
        force_y = PID_Calculate(&this->Chassis_PID_Y,this->WorldSpeed.linear_y,this->Ref_WorldSpeed.linear_y);
        torque_omega = PID_Calculate(&this->Chassis_PID_Omega,this->WorldSpeed.omega,this->Ref_WorldSpeed.omega);
    }

    this->ref_twist.linear_x = force_x;
    this->ref_twist.linear_y = force_y;
    this->ref_twist.omega = torque_omega;
}


void Swerve_Chassis::Chassis_Parking_Control()
{
    // 驻车模式

}


void Swerve_Chassis::Chassis_Reset_Output()
{
    // 重置底盘
    PID_Reset(&this->Chassis_PID_X);
    PID_Reset(&this->Chassis_PID_Y);
    PID_Reset(&this->Chassis_PID_Omega);
    PID_Reset(&this->Chassis_Yaw_Adjust);
}


// Jerry提出舵向转换其实远远大于轮向反转的转换速度，所以这里留下一个宏开关
// 可以选择是否暴力转向or使用劣弧优化
void Swerve_Chassis::RubberAngle_Adjust(Swerve_Module_t &swerve)
{

    // 左0~-PI，右0~PI
    float temp_delta_angle = swerve.rubber_angle_cmd - swerve.rubber_angle_current;// 计算目标和角度差值
    // 先将待转角度范围规范到-180~180
    while(temp_delta_angle > 180.0f)
        temp_delta_angle -= 360.0f;
    while(temp_delta_angle < -180.0f)
        temp_delta_angle += 360.0f;

    if(temp_delta_angle >= -PI / 2.0f && temp_delta_angle <= PI / 2.0f)
    {
        // +-PI/2 之间无需反向，就近转位
        swerve.rubber_angle_cmd = swerve.rubber_angle_current + temp_delta_angle;// 得到目标角度
    }
    else
    {
#ifdef RUBBER_ANGLE_OPTIMIZE
        // +-PI/2 之外需要反向,反转扣圈
        swerve.rubber_angle_cmd = swerve.rubber_angle_current + temp_delta_angle - 180.0f;// 得到目标角度
        swerve.wheel_cmd *= -1.0f;// 反转线速度
#else
        // 仍旧暴力转向
        swerve.rubber_angle_cmd = swerve.rubber_angle_current + temp_delta_angle;// 得到目标角度
        // 线速度仍旧保持正向输出
#endif
    }

}


// 舵向电机角度传入进行更新
void Swerve_Chassis::update_Swerve_Module(float rubber_1,float rubber_2,float rubber_3,float rubber_4)
{
    _swerve_module[0].rubber_angle_current = rubber_1;
    _swerve_module[1].rubber_angle_current = rubber_2;
    _swerve_module[2].rubber_angle_current = rubber_3;
    
}


float Swerve_Chassis::transfer_total_angle_to_single_angle(float total_angle_current)
{
    float temp_angle = total_angle_current;
    while(total_angle_current > 180.0f)
        temp_angle -= 360.0f;
    while(total_angle_current < -180.0f)
        temp_angle += 360.0f;
    return temp_angle;

}


float Swerve_Chassis::transfer_single_angle_to_total_angle(float total_angle_current,float single_angle_cmd)
{
    int N = 0;
    N = total_angle_current / 360;
    return single_angle_cmd += N * 360;
}


