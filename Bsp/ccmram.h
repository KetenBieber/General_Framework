/**
 * @file ccmram.h
 * @author Keten (2863861004@qq.com)
 * @brief CCMRAM 预编译指令前缀 定义为 宏函数，方便调用
 * @version 0.1
 * @date 2024-10-01
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention :对于f4芯片，只可以使用其来进行数据的存储，不可以进行代码的存储，使用的话会进入fault
 *             查询资料，现在只有f3系列芯片可以使用这一特性，移植该bsp包需记得更改
 *             存储数据需求应是“频繁被读取、提高执行速度”
 *             2024-10-10 经测试，应该是不能将CCMRAM的东西放进创建类实例初始化列表中，这样实例的初始化会失败
 * 
 * @note :
 * @versioninfo :
 */
#pragma once 

/**
 * @brief 将数据存到CCMRAM中，提高数据的读取速度
 * 
 */
#define CCMRAM __attribute__((section(".ccmram"))) 


#define PACKED __attribute__((__packed__))