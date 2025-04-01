/**
 * @file data_pool.h
 * @author Keten (2863861004@qq.com)
 * @brief 机器人所用到的数据容器定义,最后上车机器人所用的数据包，都需要在这里被定义！
 *        用于Topics之间通讯的都必须使用1字节对齐，否则会出现数据错乱
 * @version 0.1
 * @date 2024-10-03
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention :
 * @note :
 * @versioninfo :
 */
#pragma once 

#include "data_type.h"

#pragma pack(1)

/* 机器人姿态发布底盘当前位置以及航向角姿态，由姿态任务发布 */
typedef struct 
{
    float x;
    float y;
    float yaw;
    float omega;// 角速度
}pub_Chassis_Pos;

/* 底盘计算所需yaw角数据 */
typedef struct 
{
    float yaw;
}pub_imu_yaw;

typedef struct
{
    float robo_vx;
    float robo_vy;
    float robo_omega;

    float world_vx;
    float world_vy;
    float world_omega;
}pub_chassis_spe;

typedef struct
{
    uint8_t control_way;// 控制方式，遥控OR自动
    uint8_t STOP;// 置0为移动状态，置1停止速度输出
}pub_chassis_status;


typedef struct
{
    float linear_x;// x方向速度 m/s
    float linear_y;// y方向速度 m/s
    float Omega;   // 转动速度 rad/s
    uint8_t Status;// 底盘状态
    uint8_t Move;// 底盘运动方式
    uint8_t ctrl;// 底盘控制模式
    uint8_t if_rev_ros;// 是否接收ros上位机数据
}pub_Control_Data;

#pragma pack()
