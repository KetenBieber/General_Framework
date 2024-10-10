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
}pub_Chassis_Pos;

/* 底盘计算所需yaw角数据 */
typedef struct 
{
    float yaw;
}pub_imu_yaw;

#pragma pack()
