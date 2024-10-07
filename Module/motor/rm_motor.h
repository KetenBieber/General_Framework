/**
 * @file rm_motor.h  --adapted from Yang JianYi
 * @author Keten (2863861004@qq.com)
 * @brief 
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
#include <stdint.h>
#include "motor_base.h"
#include "data_type.h"


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
              float max_current, uint8_t reduction_ratio,
                uint32_t rx_id,uint32_t s_id_high, uint32_t s_id_low) 
            : Motor(id,can_rx_instance,can_tx_instance,max_current,reduction_ratio),
                    receive_id(rx_id),send_id_high{s_id_high}, send_id_low{s_id_low}{}

    virtual ~RM_Common(){}
protected:
    /* can id 设置，需要根据不同电调协议进行设置，这里初始化选择了较多型号所采用的类型 */
     uint32_t receive_id =  0x200;// 设置can通讯初始化id
     uint32_t send_id_high = 0x1FF;// 设置发送高位id
     uint32_t send_id_low = 0x200;// 设置发送低位id
    /* 根据大疆电机can通讯协议重写对应的更新函数 */

    /* 更新电机角度函数 */
    virtual void update_angle(uint8_t can_rx_data[]) override
    {
        encoder = (uint16_t)(can_rx_data[0] << 8 | can_rx_data[1]);
        if(encoder_is_init)
        {   
            if(this->encoder - this->last_encoder < -4096)
                this->round_cnt++;
            else if(this->encoder - this->last_encoder > 4096)
                this->round_cnt--;
            else{}
        }
        else 
        {
            encoder_offset = encoder;
            encoder_is_init = true;
        }

        this->last_encoder = this->encoder;
        int32_t total_encoder = round_cnt*8192 + encoder - encoder_offset;
        angle = total_encoder / encoder_angle_ratio/motor_reduction_ratio;
    }

    /* 更新电机速度函数 */
    virtual void update_speed(uint8_t can_rx_data[]) override
    {
        speed = (int16_t)(can_rx_data[2] << 8 | can_rx_data[3]);
    } 
public:
    /* 打包can发送数据，大疆电机模块依赖 */
    void prepareCANMsg(CAN_Tx_Instance_t &CAN_TxMsg, int16_t current_out ) const
    {
        if(ID <= 4 && ID > 0)
        {
            CAN_TxMsg.tx_id = send_id_low;// 如果是编号0~4的电机，选择低位发送id
            CAN_TxMsg.tx_len = 8;
            CAN_TxMsg.tx_mailbox = can_tx_for_motor.tx_mailbox;
            CAN_TxMsg.isExTid = can_tx_for_motor.isExTid;
            CAN_TxMsg.can_handle = can_tx_for_motor.can_handle;
            CAN_TxMsg.can_tx_buff[ID * 2 - 2] = (uint8_t)(current_out >> 8) & 0xff;
            CAN_TxMsg.can_tx_buff[ID * 2 - 1] = (uint8_t)current_out & 0xff;
        }    
        else if(ID <= 8 && ID > 4)
        {
            CAN_TxMsg.tx_id = send_id_high;// 如果是编号4~8的电机，选择高位发送id
            CAN_TxMsg.tx_len = 8; 
            CAN_TxMsg.tx_mailbox = can_tx_for_motor.tx_mailbox;
            CAN_TxMsg.isExTid = can_tx_for_motor.isExTid;
            CAN_TxMsg.can_handle = can_tx_for_motor.can_handle;
            CAN_TxMsg.can_tx_buff[ID * 2 - 10] = (uint8_t)(current_out >> 8) & 0xff;
            CAN_TxMsg.can_tx_buff[ID * 2 - 9] = (uint8_t)current_out & 0xff;
        }
    }
    /* 电调回传数据处理函数 */
    virtual void CanMsg_Process(CAN_Tx_Instance_t &CAN_TxMsg) override
    {
        motor_constraint(&(this->Out), -max_current, max_current);
        prepareCANMsg(CAN_TxMsg, (int16_t)(this->Out));
    }

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
    Motor_C610(uint8_t id,const CAN_Rx_Instance_t& can_rx_instance,const CAN_Tx_Instance_t& can_tx_instance)
     :RM_Common(id,can_rx_instance,can_tx_instance,10000,36,0x200,0x1ff,0x200) {}
    virtual ~Motor_C610(){}
   
    virtual void update(uint8_t can_rx_data[]) override
    {
        update_angle(can_rx_data);
        update_speed(can_rx_data);
        this->tarque = (int16_t)(can_rx_data[4] << 8 | can_rx_data[5]);
    }
    /* 对C610电机参数读取 */
    inline uint16_t get_tarque() const { return this->tarque; }
    inline uint16_t get_temperature() const { return this->temperature; }
protected:

private:
     int16_t tarque = 0;
     uint8_t temperature = 0;
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
    Motor_C620(uint8_t id,const CAN_Rx_Instance_t& can_rx_instance,const CAN_Tx_Instance_t& can_tx_instance)
     :RM_Common(id,can_rx_instance,can_tx_instance,16384,19,0x200,0x1FF,0x200) {}
    virtual ~Motor_C620(){}

    /* 对C620电机参数读取 */
    inline int16_t get_tarque() const { return this->tarque; }
    inline uint8_t get_temperature() const { return this->temperature; }

    virtual void update(uint8_t can_rx_data[]) override
    {
        update_angle(can_rx_data);
        update_speed(can_rx_data);
        this->tarque = (int16_t)(can_rx_data[4] << 8 | can_rx_data[5]);
    }

protected:

private:
     int16_t tarque = 0;
     uint8_t temperature = 0;
};


/**
 * @brief GM6020内置电调 --- 控制GM6020电机
 * 
 */
class Motor_GM6020 :public RM_Common
{
public:
    Motor_GM6020(uint8_t id,const CAN_Rx_Instance_t& can_rx_instance,const CAN_Tx_Instance_t& can_tx_instance)
     :RM_Common(id,can_rx_instance,can_tx_instance,30000,1,0x204,0x2ff,0x1ff) {}
    virtual ~Motor_GM6020(){}

    inline uint16_t set_encoder_offset(uint16_t offset)
    {
        this->encoder_offset = offset;
        this->last_encoder = offset;
        this->encoder_is_init = true;
        return this->encoder;
    }
    virtual void update(uint8_t can_rx_data[]) override
    {
        this->update_angle(can_rx_data);
        this->update_speed(can_rx_data);
        this->tarque = (int16_t)(can_rx_data[4] << 8 | can_rx_data[5]);
        this->temperature = can_rx_data[6]; 
    }

    inline int16_t get_tarque() const { return this->tarque; }
    inline uint8_t get_temperature() const { return this->temperature; }
protected:

private:
     int16_t tarque = 0;
     uint8_t temperature = 0;

};


class VESC : public Motor
{
public:

protected:

private:
};

#endif // __cplusplus


