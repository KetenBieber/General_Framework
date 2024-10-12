/**
 * @file Omni_Chassis.cpp
 * @author Keten (2863861004@qq.com)
 * @brief 
 * @version 0.1
 * @date 2024-10-10
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention :
 * @note :
 * @versioninfo :
 */
#include "Omni_Chassis.h"


/**
 * @brief 运动学逆解算
 * 
 * @param ref_twist 
 */
void Omni_Chassis::Kinematics_Inverse_Resolution(Robot_Twist_t ref_twist)
{
     static float COS45 = arm_cos_f32(PI / 4.0f);
     static float SIN45 = arm_sin_f32(PI / 4.0f);
     static uint8_t count = 1;
     switch (count)
     {
     case 1:

      break;
     case 2:

      break;
     case 3:

      break;
     case 4:

      break;
     }

     if(this->Wheel_Num == 4)
     {
        this->APS_Wheel[0] = WHEEL_TO_MOTOR*(ref_twist.linear_x * (-COS45) + ref_twist.linear_y * SIN45 + ref_twist.omega * this->Wheel_To_Center);
        this->APS_Wheel[1] = WHEEL_TO_MOTOR*(ref_twist.linear_x * (-COS45) - ref_twist.linear_y * SIN45 + ref_twist.omega * this->Wheel_To_Center); 
        this->APS_Wheel[2] = WHEEL_TO_MOTOR*(ref_twist.linear_x * COS45 - ref_twist.linear_y * SIN45 + ref_twist.omega * this->Wheel_To_Center);
        this->APS_Wheel[3] = WHEEL_TO_MOTOR*(ref_twist.linear_x * COS45 + ref_twist.linear_y * SIN45 + ref_twist.omega * this->Wheel_To_Center);
     }  
     else 
     {
        this->APS_Wheel[0] = WHEEL_TO_MOTOR*(ref_twist.linear_x * (-COS45) + ref_twist.linear_y * SIN45 + ref_twist.omega * this->Wheel_To_Center);
        this->APS_Wheel[1] = WHEEL_TO_MOTOR*(ref_twist.linear_x * COS45 - ref_twist.linear_y * SIN45 + ref_twist.omega * this->Wheel_To_Center);
        this->APS_Wheel[2] = WHEEL_TO_MOTOR*(ref_twist.linear_x * 0 + ref_twist.linear_y * 1 + ref_twist.omega * this->Wheel_To_Center);
     }
}


Robot_Twist_t Omni_Chassis::Kinematics_forward_Resolution()
{
     static float COS45 = arm_cos_f32(PI / 4.0f);
     static float SIN45 = arm_sin_f32(PI / 4.0f);
     
}

Robot_Twist_t Omni_Chassis::RoboSpeed_To_WorldSpeed()
{
    
}


void Omni_Chassis::Dynamics_Inverse_Resolution()
{

}





