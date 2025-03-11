/**
 * @file Omni_Chassis.cpp
 * @author Keten (2863861004@qq.com)
 * @brief 
 * @version 0.1
 * @date 2024-10-10
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention :人和车头同向，lf rf   1 4
 *                          lb rb   2 3
 * @note :
 * @versioninfo :
 */
#include "Omni_Chassis.h"

void Omni_Chassis::Chassis_TrackingController_Init()
{
   // todo:对底盘加入前馈控制器
   PID_Init(&this->Chassis_PID_X);
   PID_Init(&this->Chassis_PID_Y);
   PID_Init(&this->Chassis_PID_Omega);
   PID_Init(&this->Chassis_Yaw_Adjust);
}


/**
 * @brief 运动学逆解算
 *        轮子序号：
 *          按.h里面那样定义，尽量改实车上的电调id而不是来这里改程序...
 * @param ref_twist 参考机器人坐标系下速度
 * @return float    就是电机目标角速度
 */
float Omni_Chassis::Kinematics_Inverse_Resolution(size_t count,Robot_Twist_t ref_twist)
{
     if(this->Wheel_Num == 4)
     {
      static float COS45 = arm_cos_f32(45*DEGREE_2_RAD*1);
      static float SIN45 = arm_sin_f32(45*DEGREE_2_RAD*1);
      // 四全向轮解算
      switch (count)
      {
         case 0:
            return WHEEL_TO_MOTOR*(ref_twist.linear_x * (-COS45) + ref_twist.linear_y * SIN45 + ref_twist.omega * this->Wheel_To_Center);
         case 1:
            return WHEEL_TO_MOTOR*(ref_twist.linear_x * (-COS45) - ref_twist.linear_y * SIN45 + ref_twist.omega * this->Wheel_To_Center); 
         case 2:
            return WHEEL_TO_MOTOR*(ref_twist.linear_x * COS45 - ref_twist.linear_y * SIN45 + ref_twist.omega * this->Wheel_To_Center);
         case 3:
            return WHEEL_TO_MOTOR*(ref_twist.linear_x * COS45 + ref_twist.linear_y * SIN45 + ref_twist.omega * this->Wheel_To_Center);
      }
     } 
     else 
     {
         static float COS30 = arm_cos_f32(30*DEGREE_2_RAD);
         static float SIN30 = arm_sin_f32(30*DEGREE_2_RAD);
        // 三全向轮解算
        switch (count)
        {
         case 0:
            return WHEEL_TO_MOTOR*(ref_twist.linear_x * (-COS30) + ref_twist.linear_y * SIN30 + ref_twist.omega * this->Wheel_To_Center);
         case 1:
            return WHEEL_TO_MOTOR*(ref_twist.linear_x * COS30 - ref_twist.linear_y * SIN30 + ref_twist.omega * this->Wheel_To_Center);
         case 2:
            return WHEEL_TO_MOTOR*(ref_twist.linear_x * 0 + ref_twist.linear_y * 1 + ref_twist.omega * this->Wheel_To_Center);
        }
     }
     return 0;
}


/**
 * @brief 速度正解算,通过电机速度得到机器人速度
 * 
 * @param wheel_1 lf
 * @param wheel_2 rf
 * @param wheel_3 rb
 * @param wheel_4 lb
 */
void Omni_Chassis::Kinematics_forward_Resolution(float wheel_1,float wheel_2,float wheel_3,float wheel_4)
{
     if(this->Wheel_Num == 4)
     {
         static float COS45 = arm_cos_f32(45*DEGREE_2_RAD);
         static float SIN45 = arm_sin_f32(45*DEGREE_2_RAD);
         // 四全向轮正解算
         this->self_RoboSpeed.linear_x = (wheel_1 * (-COS45) + wheel_2 * (-COS45) + wheel_3 * COS45 + wheel_4 * COS45) * MOTOR_TO_WHEEL;
         this->self_RoboSpeed.linear_y = (wheel_1 * SIN45 + wheel_2 * (-SIN45) + wheel_3 * (-SIN45) + wheel_4 * SIN45) * MOTOR_TO_WHEEL;
         this->self_RoboSpeed.omega = (wheel_1 + wheel_2 + wheel_3 + wheel_4) * MOTOR_TO_WHEEL / (4 * this->Wheel_To_Center);
     }
     else
     {
         // 三全向轮正解算
         this->self_RoboSpeed.linear_x = (TWOTHIRD*wheel_1 - ONETHIRD*wheel_2 - ONETHIRD*wheel_3)*MOTOR_TO_WHEEL;
         this->self_RoboSpeed.linear_y = (0 * wheel_1 + (-SQR3_OF_THIRD)*wheel_2 + SQR3_OF_THIRD*wheel_3)*MOTOR_TO_WHEEL;
         this->self_RoboSpeed.omega = (ONETHIRD*wheel_1 + ONETHIRD*wheel_2 + ONETHIRD*wheel_3)*MOTOR_TO_WHEEL/(this->Wheel_To_Center);
     }
     // 再转换一手得到自身解算的世界速度
     this->RoboTwist_To_WorldTwist(this->self_RoboSpeed,this->self_WorldSpeed);
}


/**
 * @brief 动力学逆解算
 * @update 2024-10-29 添加了在机器人坐标系下或世界坐标系下保持X轴或Y轴方向移动的功能，待测试
 * 
 */
void Omni_Chassis::Dynamics_Inverse_Resolution()
{
   float force_x = 0,force_y = 0,torque_omega = 0;
   static uint8_t cnt;

    if (this->Chassis_Status == CHASSIS_STOP)
    {
        this->ref_twist.linear_x = force_x;
        this->ref_twist.linear_y = force_y;
        this->ref_twist.omega = torque_omega;
        return;
    }
    // 改进：直接根据模式对对应的速度进行闭环，将锁角度的操作放到task中处理
    if(this->Chassis_Status == ROBOT_CHASSIS)
    {
        force_x = PID_Calculate(&this->Chassis_PID_X, this->RoboSpeed.linear_x, this->Ref_RoboSpeed.linear_x);
        force_y = PID_Calculate(&this->Chassis_PID_Y, this->RoboSpeed.linear_y, this->Ref_RoboSpeed.linear_y);
        torque_omega = PID_Calculate(&this->Chassis_PID_Omega, this->RoboSpeed.omega, this->Ref_RoboSpeed.omega);
    }
    else // 世界坐标系 
    {
        force_x = PID_Calculate(&this->Chassis_PID_X, this->WorldSpeed.linear_x, this->Ref_WorldSpeed.linear_x);
        force_y = PID_Calculate(&this->Chassis_PID_Y, this->WorldSpeed.linear_y, this->Ref_WorldSpeed.linear_y);
        torque_omega = PID_Calculate(&this->Chassis_PID_Omega, this->WorldSpeed.omega, this->Ref_WorldSpeed.omega);
    }

   this->ref_twist.linear_x = force_x;
   this->ref_twist.linear_y = force_y;
   this->ref_twist.omega = torque_omega;
}


void Omni_Chassis::Chassis_Parking_Control()
{
   this->dt = DWT_GetDeltaT(&this->DWT_CNT);
   // 10s没动车就设置为驻车模式
   if(this->dt>=10)
   {
      this->Chassis_Status = CHASSIS_STOP;
      this->dt = 0;
   }
}


void Omni_Chassis::Chassis_Reset_Output()
{
   PID_Reset(&this->Chassis_PID_X);
   PID_Reset(&this->Chassis_PID_Y);
   PID_Reset(&this->Chassis_PID_Omega);
   PID_Reset(&this->Chassis_Yaw_Adjust);
}
















