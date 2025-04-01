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
 * @note
 * :此为底盘基类.h，请创建具体底盘类型的实例使用，其他类型的底盘均已提供对应的底盘解算以及动力学解算
 *        底盘进行机器人坐标系123和世界坐标系的灵活转换的前提是---获取了机器人的姿态，请确保机器人在posture层已经接入了姿态传感器的接口
 *        注意使用坐标系为
 *          前x、左y
 *        机器人使用pub-sub获取姿态数据，订阅chassis_imu_pub 获取 yaw角数据
 *                                     订阅chassis_pos_pub 获取位置数据（全局）
 * @versioninfo :
 *
 * @todo: 1.添加底盘驻车模式，就是底盘如果位于一段时间内没有控制的话，会进入驻车模式，这时底盘会锁0
 *        2.测试一下底盘极限速度
 *        3.将“获取当前速度”这个方法设计为
 * 函数指针，由使用者自己实现，这样可以适配不同的传感器
 *          设计出发点：因为不同的传感器获取速度的方法不同，所以这个方法应该由使用者自己实现，从而适配自己的底盘
 *        4.有些重复的东西应该设计成接口，而不是作为类的成员变量---比如什么机器人坐标系转换，待重构
 */
#pragma once

/*----------------------------------include-----------------------------------*/
#include "arm_math.h"
#include "motor_base.h"
#include "pid_controller.h"
#include "robot_def.h"
#include "topics.h"

#ifdef __cplusplus

/* 机器人坐标系结构体定义 */

typedef struct {
  /* 平动速度 */
  float linear_x;
  float linear_y;
  /* 转动速度 */
  float omega;
} Robot_Twist_t;

typedef enum {
  CHASSIS_STOP = 0,
  ROBOT_CHASSIS,
  WORLD_CHASSIS,
  PARKING_CHASSIS,
} Chassis_Status_e;

typedef enum {
  FREE = 0,      // 全向模式
  KEEP_X_MOVING, // 锁x轴方向移动
  KEEP_Y_MOVING, // 锁y轴方向移动
} Moving_Status_e;

typedef enum {
  HAND_CONTROL = 0, // 手动模式
  AUTO_CONTROL      // 自动模式
} Control_Status_e;

/* 底盘基类 */
class Chassis {
public:
  Chassis(size_t wheel_num, float wheel_radius)
      : Wheel_Num(wheel_num), Wheel_Radius(wheel_radius) {}
  virtual ~Chassis() = default;

  uint8_t Wheel_Num = 0;  // 车轮数量
  float Wheel_Radius = 0; // 车轮半径
  /* 底盘速度值PID */
  PID_t Chassis_PID_X;
  PID_t Chassis_PID_Y;
  PID_t Chassis_PID_Omega;
  PID_t Chassis_Yaw_Adjust;

  Chassis_Status_e Chassis_Status = CHASSIS_STOP; // 初始化为底盘失能状态
  Moving_Status_e Moving_Status = FREE;           // 初始化为全向移动状态
  Control_Status_e Control_Status = HAND_CONTROL; // 初始化为手动模式
  uint32_t DWT_CNT;
  float dt;

  /* 底盘所需pub-sub操作的初始化，必须调用 */
  uint8_t Chassis_Subscribe_Init() {
    this->chassis_imu_sub = register_sub("chassis_imu_pub", 1);
    this->chassis_pos_sub = register_sub("chassis_pos_pub", 1);
    this->chassis_spe_sub = register_sub("chassis_spe_pub", 1);

    this->imu_data = (pub_imu_yaw *)pvPortMalloc(sizeof(pub_imu_yaw));
    assert_param(this->imu_data != NULL);
    this->pos_data = (pub_Chassis_Pos *)pvPortMalloc(sizeof(pub_Chassis_Pos));
    assert_param(this->pos_data != NULL);
    this->spe_data = (pub_chassis_spe *)pvPortMalloc(sizeof(pub_chassis_spe));
    assert_param(this->spe_data != NULL);
    return 1;
  }

  /* 获取当前底盘姿态，必须获取imu数据之后才能进行机器人和世界坐标系的转换 */
  void Get_Current_Posture() {
    publish_data chassis_imu_data;
    chassis_imu_data = this->chassis_imu_sub->getdata(this->chassis_imu_sub);
    if (chassis_imu_data.len != -1) {
      this->imu_data = (pub_imu_yaw *)(chassis_imu_data.data);
    }
  }
  /* 获取当前底盘位置，如果有对应的ins系统可以调用此来更新底盘坐标 */
  void Get_Current_Position() {
    publish_data chassis_pos_data;
    chassis_pos_data = this->chassis_imu_sub->getdata(this->chassis_pos_sub);
    if (chassis_pos_data.len != -1) {
      pos_data = (pub_Chassis_Pos *)(chassis_pos_data.data);
    }
  }

  // 获取当前底盘速度
  void Get_Current_Velocity() {
    publish_data chassis_spe_data;
    chassis_spe_data = this->chassis_spe_sub->getdata(this->chassis_spe_sub);
    if (chassis_spe_data.len != -1) {
      spe_data = (pub_chassis_spe *)(chassis_spe_data.data);
      this->RoboSpeed.linear_x = spe_data->robo_vx;
      this->RoboSpeed.linear_y = spe_data->robo_vy;
      this->RoboSpeed.omega = spe_data->robo_omega;
      this->WorldSpeed.linear_x = spe_data->world_vx;
      this->WorldSpeed.linear_y = spe_data->world_vy;
      this->WorldSpeed.omega = spe_data->world_omega;
    }
  }

  // 世界坐标系下速度转机器人坐标系下速度
  void WorldTwist_To_RoboTwist(Robot_Twist_t &input_twist,
                               Robot_Twist_t &output_twist) {
    float COSANGLE =
        arm_cos_f32(this->imu_data->yaw * DEGREE_2_RAD); // cos90 = 0
    float SINANGLE =
        arm_sin_f32(this->imu_data->yaw * DEGREE_2_RAD); // sin90 = 1
    output_twist.linear_x =
        input_twist.linear_x * COSANGLE - input_twist.linear_y * SINANGLE;
    output_twist.linear_y =
        input_twist.linear_x * SINANGLE + input_twist.linear_y * COSANGLE;
    output_twist.omega = input_twist.omega;
  }
  // 机器人坐标系下速度转世界坐标系下速度
  void RoboTwist_To_WorldTwist(Robot_Twist_t &input_twist,
                               Robot_Twist_t &output_twist) {
    float COSANGLE = arm_cos_f32(this->imu_data->yaw * DEGREE_2_RAD);
    float SINANGLE = arm_sin_f32(this->imu_data->yaw * DEGREE_2_RAD);
    output_twist.linear_x =
        input_twist.linear_x * COSANGLE - input_twist.linear_y * SINANGLE;
    output_twist.linear_y =
        input_twist.linear_x * SINANGLE + input_twist.linear_y * COSANGLE;
    output_twist.omega = input_twist.omega;
  }

  /* 设置驻车速度 */
  void Set_Parking_Speed() {
    this->Ref_RoboSpeed = {0};
    this->Ref_WorldSpeed = {0};
  }

protected:
  /* 底盘速度pid跟踪器初始化,必须由派生类重写 */
  virtual void Chassis_TrackingController_Init() {};
  /* 运动学逆解算，由派生类重写 */
  virtual float Kinematics_Inverse_Resolution(size_t count,
                                              Robot_Twist_t ref_twist) {
    return 0;
  }
  /* 运动学正解算,由派生类重写 */
  virtual void Kinematics_forward_Resolution() {}
  /* 动力学，扭矩分配,由派生类重写 */
  virtual void Dynamics_Inverse_Resolution() {}
  /* 底盘驻车模式 */
  virtual void Chassis_Parking_Control() {}
  /* 底盘输出重置0 */
  virtual void Chassis_Reset_Output() {}

public:
  // robot_ins 解算得到
  Robot_Twist_t RoboSpeed = {0, 0, 0};  // 机器人坐标系下速度
  Robot_Twist_t WorldSpeed = {0, 0, 0}; // 世界坐标系下速度
  // 控制指令设置
  Robot_Twist_t Ref_RoboSpeed = {0, 0, 0};  // 期望机器人坐标系下速度
  Robot_Twist_t Ref_WorldSpeed = {0, 0, 0}; // 期望世界坐标系下速度

  // 自身正解算得到的机器人速度
  Robot_Twist_t self_RoboSpeed = {0, 0, 0}; // 运动学解算得到
  Robot_Twist_t self_WorldSpeed = {0, 0, 0};

  // 用于输入的计算变量
  Robot_Twist_t ref_twist; // 控制器输出量---> 设置电机的pid控制参考量

  float current_angle_to_keep = 0; // 保持某轴方向移动时储存的角度值

  /* 订阅imu话题的订阅者 */
  Subscriber *chassis_imu_sub;
  Subscriber *chassis_pos_sub;
  Subscriber *chassis_spe_sub;

  /* 订阅数据类型指针 */
  pub_imu_yaw *imu_data;     // 调用它来读yaw值
  pub_Chassis_Pos *pos_data; // 调用它来读位置值
  pub_chassis_spe *spe_data; // 调用它来读速度值
};

#ifdef __cplusplus
extern "C" {
#endif
/*-----------------------------------macro------------------------------------*/
#ifndef PI
#define PI 3.1415926535f // 圆周率
#endif

#define WHEEL_TO_MOTOR                                                         \
  RAD_2_DEGREE / (WHEEL_R) // 将车身速度逆解算得到的驱动轮速度m/s向（经过减速箱之后的）电机的转子的角速度，单位为
                           // 度/s
#define MOTOR_TO_WHEEL                                                         \
  (PI * WHEEL_R /                                                              \
   (180.0)) // 将电机轴（即转子经过减速箱之后）角速度换算到轮子实际线速度
#define SPEED_TO_RPM 60 / (2 * PI * WHEEL_R) // 将m/s转换为rpm/s
#define RPM_TO_SPEED 2 * PI *WHEEL_R / 60    // 将rpm/s转换为m/s

/*----------------------------------typedef-----------------------------------*/

#ifdef __cplusplus
}
#endif

#endif
