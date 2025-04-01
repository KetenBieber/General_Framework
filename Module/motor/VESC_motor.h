/**
 * @file VESC_motor.h  --adapted from Yang JianYi
 * @author PY 
 * @brief 
 * @version 0.1
 * @date 2024-10-08
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention :
 * @note :
 * @versioninfo :
 */
#ifndef VESC_MOTOR_H 
#define VESC_MOTOR_H 
/*----------------------------------include-----------------------------------*/
#include "motor_base.h"
#include "motor_interface.h"
#include "FreeRTOS.h"
#include "queue.h"
#ifdef __cplusplus
extern "C"{
#endif
/*-----------------------------------macro------------------------------------*/
#define CAN_PACKET_STATUS 9 //电流&转速 接收数据包标识符
/*----------------------------------typedef-----------------------------------*/

/*----------------------------------variable----------------------------------*/

/*-------------------------------------os-------------------------------------*/

/*----------------------------------function----------------------------------*/
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

/**
 * @brief VESC电调类
 * reduction_ratio: U8和5065电机无减速箱 设置为1
 * max_current: 电流限制
 * ctrl_config: 控制模式 可不用设置 直接调用对应模式的控制函数即可
 */
class VESC : public Motor
{
public:
    VESC(uint8_t id,
              const CAN_Rx_Instance_t& can_rx_instance,const CAN_Tx_Instance_t& can_tx_instance,
              const Motor_Control_Setting_t& ctrl_config,
              int16_t max_current, float reduction_ratio)
            : Motor(id,can_rx_instance,can_tx_instance,ctrl_config,max_current,reduction_ratio){}
    virtual ~VESC() = default;
    virtual void set_motor_ref(float ref) override;
    virtual void stop_the_motor() override;
    virtual void enable_the_motor() override;
    virtual void pid_control_to_motor() override;
    void Rpm_Control(float rpm); //电机转速控制 rpm
    void Cur_Control(float current);//电机电流控制 mA

public:
    inline virtual void update(uint8_t can_rx_data[8]) override
    {
        speed = (int32_t)(((uint32_t)can_rx_data[0]) << 24     |
                    ((uint32_t)can_rx_data[1]) << 16  |
                    ((uint32_t)can_rx_data[2]) << 8   |
                    ((uint32_t)can_rx_data[3]));
        
        motor_current = (int16_t)((can_rx_data[4] << 8) | can_rx_data[5]);

        motor_current=(float)motor_current*0.01f;// A
        speed = (float)speed/((float)motor_polse);
    }

    virtual void CanMsg_Process(CAN_Tx_Instance_t& can_txmsg)override
    {
        can_txmsg=can_tx_for_motor;
        //发送转速
        if(can_tx_for_motor.tx_id==(CAN_CMD_SET_ERPM << 8) | ID)
        {
            this->target_rpm = Out*if_reduction*motor_polse;
            if (target_rpm != 0)
            {
                can_txmsg.can_tx_buff[0]=((int32_t)target_rpm >> 24) & 0xFF;
                can_txmsg.can_tx_buff[1]=((int32_t)target_rpm >> 16) & 0xFF;
                can_txmsg.can_tx_buff[2]=((int32_t)target_rpm >> 8) & 0xFF;
                can_txmsg.can_tx_buff[3]=(int32_t)target_rpm & 0xFF;
            }
            else
            {
                can_txmsg.can_tx_buff[0]=((int32_t)brake_current >> 24) & 0xFF;
                can_txmsg.can_tx_buff[1]=((int32_t)brake_current >> 16) & 0xFF;
                can_txmsg.can_tx_buff[2]=((int32_t)brake_current >> 8) & 0xFF;
                can_txmsg.can_tx_buff[3]=(int32_t)brake_current & 0xFF;
                can_txmsg.tx_id=(CAN_CMD_SET_BRAKE << 8) | ID;
            }
        }
        //发送电流
        else if(can_tx_for_motor.tx_id==(CAN_CMD_SET_CURRENT << 8) | ID)
        {
            if(Out != 0)
            {
                can_txmsg.can_tx_buff[0]=((int32_t)Out >> 24) & 0xFF;
                can_txmsg.can_tx_buff[1]=((int32_t)Out >> 16) & 0xFF;
                can_txmsg.can_tx_buff[2]=((int32_t)Out >> 8) & 0xFF;
                can_txmsg.can_tx_buff[3]=(int32_t)Out & 0xFF;
            }
            else
            {
                can_txmsg.can_tx_buff[0]=((int32_t)brake_current >> 24) & 0xFF;
                can_txmsg.can_tx_buff[1]=((int32_t)brake_current >> 16) & 0xFF;
                can_txmsg.can_tx_buff[2]=((int32_t)brake_current >> 8) & 0xFF;
                can_txmsg.can_tx_buff[3]=(int32_t)brake_current & 0xFF;
                can_txmsg.tx_id=(CAN_CMD_SET_BRAKE << 8) | ID;
            }
        }
    }
    
protected:

    float target_rpm; //目标转速
    uint32_t brake_current=6000;//刹车电流
    uint8_t motor_polse = 7;//极对数

    // VESC控制id指令枚举
    typedef enum {
    CAN_CMD_SET_CURRENT = 0x01,
    CAN_CMD_SET_ERPM    = 0x03,
    CAN_CMD_SET_BRAKE   = 0x02,
    }CAN_PACKET_ID;

};
/*------------------------------------test------------------------------------*/
#endif

#endif /* VESC_MOTOR_H */
