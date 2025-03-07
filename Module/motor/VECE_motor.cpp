/**
 * @file VECS_motor.cpp
 * @author PY 
 * @brief VESC电调控制实现
 * 
 * **************************************************************************
 * 使用说明：
 *  实例化VESC对象后，输入对应所需的设置参数
 *  有两种控制模式--电流控制和转速控制 ，调用对应的控制函数即可，不用特别设置控制模式
 *  根据电机实际 级对数 和所需 刹车电流 更改类中的数据
 *  机器人学院新版f407开发板（带虚拟串口） can口左边为CAN_L，右边为CAN_H
 * **************************************************************************
 * 电调上位机配置（请使用3.0以上版本）：
 * 
 * Motor Setting General 配置要点（U8+磁编）
 * --Motor Type-- 选择“FOC”
 * --Sensor Port Mode-- 选择“ABI Encoder”
 * --ABI Encoder Counts-- 选择4000
 * 
 * “General” 校准
 * “Encoder” 校准
 * 
 * App Setting General 配置要点 其他可参考https://blog.csdn.net/Piamen/article/details/103206573?ops_request_misc=&request_id=&biz_id=102&utm_term=%E6%9C%AC%E6%9D%B0%E6%98%8E%E7%94%B5%E8%B0%83vesc%20can&utm_medium=distribute.pc_search_result.none-task-blog-2~all~sobaiduweb~default-8-103206573.142^v101^pc_search_result_base5&spm=1018.2226.3001.4187
 * --APP to Use-- 电调装车后没有完全确认配置，尽量设置为“UART”，避免出现只能使用USB更改配置的情况，确认后可设置为NO APP
 *  --VESC ID-- 设置电调ID 1、2、3、4
 *  --Can Status Message Mode-- 选择“ CAN_STATUS_1 ”
 *  --Can Status Rate-- 选择50Hz
 *  -- CAN Baund Rate-- 选择1M
 *  --CAN Mode-- 选择VESC
 * ****************************************************************************
 * @version 0.1
 * @date 2024-1-12
 * 
 * @copyright Copyright (c) 2025
 * 
 * @attention :
 * @note :
 * 
 */

#include "VESC_motor.h"
#include "motor_interface.h"

/**
 * @brief 转速控制函数
 */
void VESC::Rpm_Control(float rpm)
{
    this->Out=rpm;
    can_tx_for_motor.tx_id= (CAN_CMD_SET_ERPM << 8) | ID;
    return;
}

/**
 * @brief 电流控制函数
 */
void VESC::Cur_Control(float current)
{
    this->Out=current;
    can_tx_for_motor.tx_id= (CAN_CMD_SET_CURRENT << 8) | ID;
    return;
}

void VESC::set_motor_ref(float ref)
{
   return;
}

void VESC::stop_the_motor()
{
    return;
}

void VESC::enable_the_motor()
{
    return;
}

void VESC::pid_control_to_motor()
{
    return;
}
