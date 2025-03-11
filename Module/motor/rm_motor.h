/**
 * @file rm_motor.h  --adapted from Yang JianYi
 * @author Keten (2863861004@qq.com)
 * @brief 大疆电机分类封装，对获取角速度进行封装
 * @version 0.1
 * @date 2024-10-03
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention :
 * @note :
 * @versioninfo :
 * @todo： 1.GM6020电机的测试
 *         2.2024 10-22 还是将默认减速箱加入了，电机速度闭环控制现在默认采用减速过后的轴速,而非使用转子实际速度
 *                     现在加入一个初始化列表变量,用来决定是否使用默认减速箱配置
 *                     if_use_default_reduction_ratio 
 *                          1 --- 去除减速箱 配置
 *                         -1 --- 使用默认减速箱
 *                          x --- 使用外部减速箱,减速比为x
 *                          

 */
#pragma once
#include <stdint.h>
#include "motor_base.h"
#include "data_type.h"
#include "bsp_dwt.h"

#ifdef __cplusplus

/**
 * @brief rm电机 协议层接口
 * 
 */
class RM_Common : public Motor
{
public:
    RM_Common(uint8_t id,
              const CAN_Rx_Instance_t& can_rx_instance,const CAN_Tx_Instance_t& can_tx_instance,
              const Motor_Control_Setting_t& ctrl_config,
              int16_t max_current, float reduction_ratio) 
            : Motor(id,can_rx_instance,can_tx_instance,ctrl_config,max_current,reduction_ratio){}

    virtual ~RM_Common() = default;
    virtual void set_motor_ref(float ref) override;
    virtual void stop_the_motor() override;
    virtual void enable_the_motor() override;
    virtual void pid_control_to_motor() override;
    void Motor_Ctrl(float ref);
protected:
    /* 根据大疆电机can通讯协议重写对应的更新函数 */

    /* 更新电机角度函数 */
    inline virtual void update_angle(uint8_t can_rx_data[]) override
    {
        // 读取当前编码器值
        encoder = (uint16_t)(can_rx_data[0] << 8 | can_rx_data[1]);
        if(encoder_is_init)
        {   
            int16_t delta_encoder = encoder - last_encoder;

            // 处理编码器值的溢出情况
            if(delta_encoder < -4096)
                round_cnt++;
            else if(delta_encoder > 4096)
                round_cnt--;

            // 更新总编码器值
            int32_t total_encoder = round_cnt * 8192 + encoder - encoder_offset;
            angle = static_cast<float>(total_encoder) / encoder_angle_ratio / motor_reduction_ratio;
        }
        else 
        {
            this->encoder_offset = this->encoder;
            encoder_is_init = true;
        }

        // 更新上一次的编码器值
        this->last_encoder = this->encoder;
    }

    /* 更新电机转子速度函数 */
    inline virtual void update_speed(uint8_t can_rx_data[]) override
    {
        if(this->if_reduction == -1)        // 使用默认减速箱
        {
            this->speed = (int16_t)(can_rx_data[2] << 8 | can_rx_data[3])/motor_reduction_ratio;// 单位:rpm
        }
        else if(this->if_reduction == 1)        // 只剩屁股(无减速箱)
        {
            this->speed = (int16_t)(can_rx_data[2] << 8 | can_rx_data[3]);// 单位:rpm
        }
        else    // 外接减速箱
        {
            this->speed = (int16_t)(can_rx_data[2] << 8 | can_rx_data[3])/motor_reduction_ratio;// 单位:rpm
        }
    } 
    inline virtual void update_speed_aps() override
    {
        this->last_speed_aps = this->speed_aps;
        this->speed_aps = this->alpha * this->speed* RPM_PER_MIN_2_ANGLE_PER_SEC + (1 - this->alpha) * this->temp_last_speed_aps;
        this->temp_last_speed_aps =  this->speed_aps;
    }
    /* 更新电机实际电流值 */
    inline void update_current(uint8_t can_rx_data[]) override
    {
        this->motor_current = (int16_t)(can_rx_data[4]<<8 | can_rx_data[5]);        
    }
    /* 更新电机温度函数 */
    inline void update_temperature(uint8_t can_rx_data[]) override
    {
        this->motor_temperature = (int8_t)can_rx_data[6];
    }
    /* 更新电机加速度函数 */
    inline void update_motor_acceleration()
    {
        this->motor_acceleration = ((this->speed_aps - this->last_speed_aps)*DEGREE_2_RAD / this->dt)*this->alpha + (1 - this->alpha)*this->temp_last_motor_acceleration;
        this->temp_last_motor_acceleration = this->motor_acceleration;
    }
public:
    /* 打包can发送数据，大疆电机模块依赖 */
    void prepareCANMsg(CAN_Tx_Instance_t &CAN_TxMsg, int16_t current_out ) const
    {
        CAN_TxMsg.tx_id = can_tx_for_motor.tx_id;                               // 根据设置id进行发送
        CAN_TxMsg.tx_len = 8;                                                   // 大疆电机这里统一设置成8
        CAN_TxMsg.tx_mailbox = can_tx_for_motor.tx_mailbox;                     // 设置邮箱号
        CAN_TxMsg.isExTid = can_tx_for_motor.isExTid;                           // 设置发送帧的种类，大疆电机默认为发送标准帧
        CAN_TxMsg.can_handle = can_tx_for_motor.can_handle;                     // 设置发送can句柄
        CAN_TxMsg.can_tx_buff[ID * 2 - 2] = (uint8_t)(current_out >> 8) & 0xff; // 装填电流值高8位
        CAN_TxMsg.can_tx_buff[ID * 2 - 1] = (uint8_t)current_out & 0xff;        // 装填电流值低8位
    }
    /* 电调控制数据处理函数 */
    virtual void CanMsg_Process(CAN_Tx_Instance_t &CAN_TxMsg) override
    {
        motor_constraint(&(this->Out), static_cast<int16_t>(-max_current),static_cast<int16_t>(max_current));
        prepareCANMsg(CAN_TxMsg, (this->Out));
    }
    /* 电调发送单位转换，提供转子速度向电流的转换，需由各型号电调各自实现 */
    virtual int16_t aps_to_current(float &input_ref){ return 0; }
};


/**
 * @brief C610 电调 --- 控制m2006电机
 * 构造函数参数：
 *      id：
 *      max_current:10000
 *    reduction_ratio:  -1 --- 使用默认36
 *                       1  --- 不接减速箱
 *                       x --- 自制减速箱减速比
 */
class Motor_C610 : public RM_Common
{
public:
    Motor_C610(uint8_t id,const CAN_Rx_Instance_t& can_rx_instance,const CAN_Tx_Instance_t& can_tx_instance,
               const Motor_Control_Setting_t& ctrl_config,float reduction_ratio)
     :RM_Common(id,can_rx_instance,can_tx_instance,ctrl_config,10000,reduction_ratio)
     {
        if(this->if_reduction == 1)
        {
            this->motor_reduction_ratio = 1;
        }
        else if(this->if_reduction == -1)
        {
            this->motor_reduction_ratio = 36;
        }
        else
        {
            this->motor_reduction_ratio = reduction_ratio;
        }        
    }
    virtual ~Motor_C610() = default;
   
    virtual void update(uint8_t can_rx_data[]) override
    {
        this->dt = DWT_GetDeltaT(&this->DWT_CNT);
        update_angle(can_rx_data);
        update_speed(can_rx_data);
        update_speed_aps();
        update_motor_acceleration();
        update_current(can_rx_data);
        update_temperature(can_rx_data);
    }
    /* 查阅电调手册：发送控制信息和电调实际控制信息存在映射关系，C610控制电流范围 -10000~10000 ，发送控制电流范围 -10000~10000*/
    virtual int16_t aps_to_current(float &input_ref) override
    {
        return (int16_t)input_ref * (10000.0f/10000.0f);
    }
protected:

};


/**
 * @brief C620 电调 --- 控制m3508电机
 * 构造函数参数：
 *     id：
 *    max_current:16384
 *    reduction_ratio:  -1 --- 使用默认19
 *                       1 --- 不接减速箱
 *                       x --- 自制减速箱减速比
 */                     

class Motor_C620 :public RM_Common
{
public:
    Motor_C620(uint8_t id,const CAN_Rx_Instance_t& can_rx_instance,const CAN_Tx_Instance_t& can_tx_instance,
                const Motor_Control_Setting_t& ctrl_config,float reduction_ratio)
     :RM_Common(id,can_rx_instance,can_tx_instance,ctrl_config,16384,reduction_ratio)
     {
        if(this->if_reduction == 1)
        {
            this->motor_reduction_ratio = 1;
        }
        else if(this->if_reduction == -1)
        {
            this->motor_reduction_ratio = 19;
        }
        else
        {
            this->motor_reduction_ratio = reduction_ratio;
        }        
    }
    virtual ~Motor_C620() = default;

    virtual void update(uint8_t can_rx_data[]) override
    {
        update_angle(can_rx_data);
        update_speed(can_rx_data);
        update_speed_aps();
        update_current(can_rx_data);
        update_temperature(can_rx_data);        
    }

    /* 查阅电调手册：发送控制信息和电调实际控制信息存在映射关系，C620控制电流范围-20000~20000 ，发送控制电流范围-16384~16384 */
    virtual int16_t aps_to_current(float &input_ref) override
    {
        return (int16_t)input_ref * (16384.0f/20000.0f);
    }

protected:

};


/**
 * @brief GM6020内置电调 --- 控制GM6020电机
 *    reduction_ratio:  -1 --- 使用默认1
 *                       1  --- 不接减速箱
 *                       x --- 自制减速箱减速比
 *    最大空载转速 320rpm
 *    额定扭矩 1.2 N*m
 *    一般采用位置控制
 */
class Motor_GM6020 :public RM_Common
{
public:
    Motor_GM6020(uint8_t id,const CAN_Rx_Instance_t& can_rx_instance,const CAN_Tx_Instance_t& can_tx_instance,
                 const Motor_Control_Setting_t& ctrl_config,float reduction_ratio)
     :RM_Common(id,can_rx_instance,can_tx_instance,ctrl_config,30000,reduction_ratio)
     {
        if(this->if_reduction == 1)
        {
            this->motor_reduction_ratio = 1;
        }
        else if(this->if_reduction == -1)
        {
            this->motor_reduction_ratio = 1;
        }
        else
        {
            this->motor_reduction_ratio = reduction_ratio;
        }        
    }
    virtual ~Motor_GM6020() = default;

    inline uint16_t set_encoder_offset(uint16_t offset)
    {
        this->encoder_offset = offset;
        this->last_encoder = offset;
        this->encoder_is_init = true;
        return this->encoder;
    }
    virtual void update(uint8_t can_rx_data[]) override
    {
        update_angle(can_rx_data);
        update_speed(can_rx_data);
        update_speed_aps();
        update_current(can_rx_data);
        update_temperature(can_rx_data);
    }

    /* 查阅电调手册：发送控制信息和电调实际控制信息存在映射关系 */
    virtual int16_t aps_to_current(float &input_ref) override
    {
        return (int16_t)input_ref * (16384.0f/30000.0f);
    }

protected:

};

#endif



