/**
 * @file DM_motor.h
 * @author 冯大帅将军 
 * @brief 
 * @version 0.1
 * @date 2025-4-01
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention :
 * @note :
 * @versioninfo :
 */
#ifndef DM_MOTOR_H
#define DM_MOTOR_H
/*----------------------------------include-----------------------------------*/
#include "motor_base.h"
#include "motor_interface.h"
#include "FreeRTOS.h"
#include "queue.h"
#ifdef __cplusplus
extern "C"{
#endif 
/*-----------------------------------macro------------------------------------*/
#define _POS_with_SPEED_CONTROL 0x01//位置速度控制
#define _SPEED_CONTROL         0x02//速度控制
#define _MIT_CONTROL           0x03//MIT控制

//此时是设置电机模式的，当然也可以用枚举
#define CMD_MOTOR_MODE      0x01
#define CMD_RESET_MODE      0x02
#define CMD_ZERO_POSITION   0x03

//根据相关参数进行限幅
#define P_MIN   -12.5f//position电机位置
#define P_MAX	 12.5f
#define V_MIN	-500.0f//电机速度
#define V_MAX	 500.0f
#define T_MIN 	-18.0f//电机扭矩
#define T_MAX 	 18.0f
#define Kp_MIN 	 0//Kp范围
#define Kp_MAX   500.0f
#define Kd_MIN 	 0//Kd范围
#define Kd_MAX   5.0f
#define I_MIN  	 0
#define I_MAX  	 18.0f

/*函数AK80_motion_control用**/
#define PITCH_MAX		90.0f
#define PITCH_MIN		-90.0f

//此参数的作用暂时未知
#define MIT_P_MIN						-12.5f
#define MIT_P_MAX						 12.5f
#define MIT_V_MIN						-500.0f
#define MIT_V_MAX						 500.0f
/*----------------------------------typedef-----------------------------------*/

/*----------------------------------variable----------------------------------*/

/*-------------------------------------os-------------------------------------*/

/*----------------------------------function----------------------------------*/
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
/**
 * @brief DM电调类
 * reduction_ratio: DM-J4310-2EC减速比为10 设置为？
 * max_current: 最大电流为41A,最好不超过32A,上位机设置，这里不用管
 * ctrl_config: 控制模式
 * （分为速度位置控制和速度模式，还有MIT控制（MIT控制式有三种控制方法：速度、位置、力矩）） 
 * 可不用设置 直接调用对应模式的控制函数即可
 */
class DM_motor : public Motor
{
public:
    DM_motor(uint8_t id,
        const CAN_Rx_Instance_t& can_rx_instance,const CAN_Tx_Instance_t& can_tx_instance,
        const Motor_Control_Setting_t& ctrl_config,
        uint8_t ctrl_mode_,
        int16_t max_current, float reduction_ratio)
      : ctrl_mode(ctrl_mode_),
        Motor(id,can_rx_instance,can_tx_instance,ctrl_config,max_current,reduction_ratio){
        DM_MOTOR_ENABLE();
      }
    virtual ~DM_motor() = default;
    virtual void set_motor_ref(float ref) override;
    virtual void stop_the_motor() override;
    virtual void enable_the_motor() override;
    virtual void pid_control_to_motor() override;
    void POS_with_SPEED_CONTROL(float pos, float speed);//位置速度控制
    void SPEED_CONTROL(float speed);//速度控制
    void MIT_CONTROL(float speed, float pos, float torque,float Kp, float Kd);//MIT控制
    
    /***浮点型转整形***
    入口参数：浮点数据、该数据最小值、该数据最大值、位数
    *****************/
    int float_to_uint(float x1,float x1_min,float x1_max,int bits)
    {
	    float span = x1_max-x1_min;
	    float offset = x1_min;
	    return (int)((x1-offset)*((float)((1<<bits)-1))/span);
    }

    //整型转浮点型
    //根据给定的范围和位数，将无符号整数转换为浮点
    float uint_to_float(int x1_int,float x1_min,float x1_max,int bits)
    {
	    float span=x1_max-x1_min;
	    float offset=x1_min;
	    return ((float)x1_int)*span/((float)((1<<bits)-1)) + offset;
    }
public:
    inline virtual void update(uint8_t can_rx_data[8]) override
    {
        POS=(can_rx_data[1]<<8)|can_rx_data[2];                           // 电机位置数据
        SPEED = (can_rx_data[3]<<4)|(can_rx_data[4]>>4);                  // 电机速度数据
        TORQUE = ((can_rx_data[4]&0xF)<<8)|can_rx_data[5];                // 电机扭矩数据
        real_angle = uint_to_float(POS, MIT_P_MIN, MIT_P_MAX, 16);        // 转子机械角度
        real_speed = uint_to_float(SPEED, MIT_V_MIN, MIT_V_MAX, 12);      // 实际转子转速
        motor_current = uint_to_float(TORQUE, -I_MAX, I_MAX, 12);		  // 实际转矩电流         
    }
    virtual void CanMsg_Process(CAN_Tx_Instance_t& CAN_TxMsg)override
    {
        
        CAN_TxMsg.tx_mailbox = can_tx_for_motor.tx_mailbox;
        CAN_TxMsg.isExTid = can_tx_for_motor.isExTid;
        CAN_TxMsg.can_handle = can_tx_for_motor.can_handle;
        if(MOTOR_MODE ==0){
        if(ctrl_mode == _POS_with_SPEED_CONTROL)
        {
            CAN_TxMsg.tx_id = can_tx_for_motor.tx_id+0x100;
            CAN_TxMsg.tx_len = 8;
            uint8_t *pbuf,*vbuf;
            pbuf = (uint8_t *)&pos;
            vbuf = (uint8_t *)&speed;
            CAN_TxMsg.can_tx_buff[0] = *pbuf;
            CAN_TxMsg.can_tx_buff[1] = *(pbuf+1);
            CAN_TxMsg.can_tx_buff[2] = *(pbuf+2);
            CAN_TxMsg.can_tx_buff[3] = *(pbuf+3);
            CAN_TxMsg.can_tx_buff[4] = *vbuf;
            CAN_TxMsg.can_tx_buff[5] = *(vbuf+1);
            CAN_TxMsg.can_tx_buff[6] = *(vbuf+2);
            CAN_TxMsg.can_tx_buff[7] = *(vbuf+3);
        }
        else if(ctrl_mode == _SPEED_CONTROL)
        {
            CAN_TxMsg.tx_id = can_tx_for_motor.tx_id+0x200;
            CAN_TxMsg.tx_len = 4;
            uint8_t *vbuf;
            vbuf = (uint8_t *)&this->speed;
            CAN_TxMsg.can_tx_buff[0] = *vbuf;
            CAN_TxMsg.can_tx_buff[1] = *(vbuf+1);
            CAN_TxMsg.can_tx_buff[2] = *(vbuf+2);
            CAN_TxMsg.can_tx_buff[3] = *(vbuf+3);
        }
        else if(ctrl_mode == _MIT_CONTROL)
        {
            CAN_TxMsg.tx_id = can_tx_for_motor.tx_id;
            CAN_TxMsg.tx_len = 8;
            uint16_t pos_tmp,vel_tmp,kp_tmp,kd_tmp,tor_tmp;
            pos_tmp = float_to_uint(this->pos,MIT_P_MIN,MIT_P_MAX,16);
            vel_tmp = float_to_uint(this->speed,MIT_V_MIN,MIT_V_MAX,12);
            kp_tmp = float_to_uint(this->Kp,Kp_MIN,Kp_MAX,12);
            kd_tmp = float_to_uint(this->Kd,Kd_MIN,Kd_MAX,12);
            tor_tmp = float_to_uint(this->torque,I_MIN,I_MAX,12);
            CAN_TxMsg.can_tx_buff[0] = pos_tmp>>8;
            CAN_TxMsg.can_tx_buff[1] = pos_tmp;
            CAN_TxMsg.can_tx_buff[2] = vel_tmp>>4;
            CAN_TxMsg.can_tx_buff[3] = (vel_tmp<<4)|(kp_tmp>>8);
            CAN_TxMsg.can_tx_buff[4] = kp_tmp;
            CAN_TxMsg.can_tx_buff[5] = kd_tmp>>4;
            CAN_TxMsg.can_tx_buff[6] = (kd_tmp<<4)|(tor_tmp>>8);
            CAN_TxMsg.can_tx_buff[7] = tor_tmp;
        }}
        else if(MOTOR_MODE == CMD_MOTOR_MODE)
        {
          CAN_TxMsg.tx_id = can_tx_for_motor.tx_id+0x100;
          CAN_TxMsg.tx_len = 8;
          CAN_TxMsg.can_tx_buff[0] = (uint8_t)(0xFF);
          CAN_TxMsg.can_tx_buff[1] = (uint8_t)(0xFF);
          CAN_TxMsg.can_tx_buff[2] = (uint8_t)(0xFF);
          CAN_TxMsg.can_tx_buff[3] = (uint8_t)(0xFF);
          CAN_TxMsg.can_tx_buff[4] = (uint8_t)(0xFF);
          CAN_TxMsg.can_tx_buff[5] = (uint8_t)(0xFF);
          CAN_TxMsg.can_tx_buff[6] = (uint8_t)(0xFF);
          CAN_TxMsg.can_tx_buff[7] = (uint8_t)(0xFC);
          MOTOR_MODE = 0;
        }
        else if(MOTOR_MODE == CMD_RESET_MODE)
        {
          CAN_TxMsg.tx_id = can_tx_for_motor.tx_id+0x100;
          CAN_TxMsg.tx_len = 8;
          CAN_TxMsg.can_tx_buff[0] = (uint8_t)(0xFF);
          CAN_TxMsg.can_tx_buff[1] = (uint8_t)(0xFF);
          CAN_TxMsg.can_tx_buff[2] = (uint8_t)(0xFF);
          CAN_TxMsg.can_tx_buff[3] = (uint8_t)(0xFF);
          CAN_TxMsg.can_tx_buff[4] = (uint8_t)(0xFF);
          CAN_TxMsg.can_tx_buff[5] = (uint8_t)(0xFF);
          CAN_TxMsg.can_tx_buff[6] = (uint8_t)(0xFF);
          CAN_TxMsg.can_tx_buff[7] = (uint8_t)(0xFD);
          MOTOR_MODE = 0;
        }
        else if(MOTOR_MODE == CMD_ZERO_POSITION)
        {
          CAN_TxMsg.tx_id = can_tx_for_motor.tx_id+0x100;
          CAN_TxMsg.tx_len = 8;
          CAN_TxMsg.can_tx_buff[0] = (uint8_t)(0xFF);
          CAN_TxMsg.can_tx_buff[1] = (uint8_t)(0xFF);
          CAN_TxMsg.can_tx_buff[2] = (uint8_t)(0xFF);
          CAN_TxMsg.can_tx_buff[3] = (uint8_t)(0xFF);
          CAN_TxMsg.can_tx_buff[4] = (uint8_t)(0xFF);
          CAN_TxMsg.can_tx_buff[5] = (uint8_t)(0xFF);
          CAN_TxMsg.can_tx_buff[6] = (uint8_t)(0xFF);
          CAN_TxMsg.can_tx_buff[7] = (uint8_t)(0xFE);
          MOTOR_MODE = 0;
        }
    }
    void DM_MOTOR_ENABLE(void)
    {
        MOTOR_MODE = CMD_MOTOR_MODE;
    }
    void DM_MOTOR_DISABLE(void)
    {
        MOTOR_MODE = CMD_RESET_MODE;
    }
    void DM_MOTOR_ZERO_POSITION(void)
    {
        MOTOR_MODE = CMD_ZERO_POSITION;
    }
protected:
    int16_t POS; //电机位置数据
    int16_t SPEED; //电机速度数据
    int16_t TORQUE; //电机扭矩数据
    float real_angle; //转子机械角度
    float real_speed; //实际转子转速
    float motor_current; //实际转矩电流
    float pos; //目标位置
    float speed; //目标速度
    float torque; //目标转矩
    float Kp; //MIT比例系数
    float Kd; //MIT微分系数
    uint8_t ctrl_mode; //控制模式
    uint8_t MOTOR_MODE=0;

};
    



#endif 
#endif /* __DM_MOTOR_H__ */