/**
 * @file robot_def.h
 * @author Keten (2863861004@qq.com)
 * @brief 机器人的参数配置文件及线程间通信数据类型定义
 * @version 0.1
 * @date 2024-09-14
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention :
 * @note :
 * @versioninfo :
 */
#pragma once 

/* 是否启用多板通讯 */
#define ONE_BOARD 1                     // 单板控制整车

/* 底盘类型选择 */
#define USE_FOUR_SWERVE_CHASSIS 0           // 使用四舵轮底盘
#define USE_THREE_SWERVE_CHASSIS 0          // 使用三舵轮底盘
#define USE_FOUR_OMNI_CHASSIS 1             // 使用四全向轮
#define USE_THREE_OMNI_CHASSIS 0            // 使用三全向轮


/* 机器人重要参数定义 */



/* 通讯所用数据结构体定义 */
// 记得取消字节对齐



