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

 */
#pragma once
#include <stdint.h>
#include "motor_base.h"
#include "data_type.h"
#include "filter.h"

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
              int16_t max_current, uint8_t reduction_ratio,
                uint32_t rx_id,uint32_t s_id_high, uint32_t s_id_low) 
            : Motor(id,can_rx_instance,can_tx_instance,ctrl_config,max_current,reduction_ratio),
                    receive_id(rx_id),send_id_high{s_id_high}, send_id_low{s_id_low}{}

    virtual ~RM_Common() = default;
    virtual void set_motor_ref(float ref) override;
    virtual void stop_the_motor() override;
    virtual void enable_the_motor() override;
    virtual void pid_control_to_motor() override;
protected:
    /* can id 设置，需要根据不同电调协议进行设置，这里初始化选择了较多型号所采用的类型 */
     uint32_t receive_id =  0x200;// 设置can通讯初始化id
     uint32_t send_id_high = 0x1FF;// 设置发送高位id
     uint32_t send_id_low = 0x200;// 设置发送低位id
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
            encoder_offset = encoder;
            encoder_is_init = true;
        }

        // 更新上一次的编码器值
        last_encoder = encoder;
    }

    /* 更新电机转子速度函数 */
    inline virtual void update_speed(uint8_t can_rx_data[]) override
    {
        speed = (int16_t)(can_rx_data[2] << 8 | can_rx_data[3]);
    } 
    inline virtual void update_speed_aps() override
    {
        ExponentialFilter<float> aps_filter(0.85f);// 指数平滑系数，填1则无滤波
        aps_filter.update(RPM_PER_MIN_2_ANGLE_PER_SEC*(float)(this->speed));
        speed_aps = aps_filter.getFilteredValue();
    }
    /* 更新电机实际电流值 */
    inline void update_current(uint8_t can_rx_data[]) override
    {
        motor_current = (int16_t)(can_rx_data[4]<<8 | can_rx_data[5]);        
    }
    /* 更新电机温度函数 */
    inline void updata_temperature(uint8_t can_rx_data[]) override
    {
        motor_temperature = (int8_t)can_rx_data[6];
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
    /* 电调回传数据处理函数 */
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
 *      reduction_ratio:36
 * 减速比
 */
class Motor_C610 : public RM_Common
{
public:
    Motor_C610(uint8_t id,const CAN_Rx_Instance_t& can_rx_instance,const CAN_Tx_Instance_t& can_tx_instance,
               const Motor_Control_Setting_t& ctrl_config)
     :RM_Common(id,can_rx_instance,can_tx_instance,ctrl_config,10000,36,0x200,0x1ff,0x200) {}
    virtual ~Motor_C610() = default;
   
    virtual void update(uint8_t can_rx_data[]) override
    {
        update_angle(can_rx_data);
        update_speed(can_rx_data);
        update_speed_aps();
        update_current(can_rx_data);
        updata_temperature(can_rx_data);
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
 *    reduction_ratio:  19:1
 *    
 */
class Motor_C620 :public RM_Common
{
public:
    Motor_C620(uint8_t id,const CAN_Rx_Instance_t& can_rx_instance,const CAN_Tx_Instance_t& can_tx_instance,
                const Motor_Control_Setting_t& ctrl_config)
     :RM_Common(id,can_rx_instance,can_tx_instance,ctrl_config,16384,19,0x200,0x1FF,0x200) {}
    virtual ~Motor_C620() = default;

    virtual void update(uint8_t can_rx_data[]) override
    {
        update_angle(can_rx_data);
        update_speed(can_rx_data);
        update_speed_aps();
        update_current(can_rx_data);
        updata_temperature(can_rx_data);        
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
 * 
 */
class Motor_GM6020 :public RM_Common
{
public:
    Motor_GM6020(uint8_t id,const CAN_Rx_Instance_t& can_rx_instance,const CAN_Tx_Instance_t& can_tx_instance,
                 const Motor_Control_Setting_t& ctrl_config)
     :RM_Common(id,can_rx_instance,can_tx_instance,ctrl_config,30000,1,0x204,0x2ff,0x1ff) {}
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
        updata_temperature(can_rx_data);
    }

    /* 查阅电调手册：发送控制信息和电调实际控制信息存在映射关系 */
    virtual int16_t aps_to_current(float &input_ref) override
    {
        return 0;
    }
protected:

};

#endif



