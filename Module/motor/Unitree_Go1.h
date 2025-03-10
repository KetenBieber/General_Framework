/**
 * @file Unitree_Go1.h
 * @author Keten (2863861004@qq.com)
 * @brief
 * @version 0.1
 * @date 2024-11-02
 *
 * @copyright Copyright (c) 2024
 *
 * @attention :注意：使用接口时直接对输出轴进行控制，不需要考虑减速箱的影响
 *             其实对于连贯的控制还没具体写过，因为没有应用场景（
 *             所以只是对各种控制模式做接口，实际上用到的时候应该怎么连贯控制还需重新考虑，可以提要求给我来设计
 *
 * @note :
 * 目前打算将go1系列电机统一用一条总线做处理，因为它的数据解析程序比较特别！
 *
 * @versioninfo :
 */
#ifndef UNITREE_GO1_H
#define UNITREE_GO1_H

/*----------------------------------include-----------------------------------*/
#include "FreeRTOS.h"
#include "com_config.h"
#include "motor_base.h"
#include "motor_interface.h"
#include "queue.h"

#ifdef __cplusplus

extern "C" {
#endif
/*----------------------------------typedef-----------------------------------*/

typedef enum {
  NORMAL_WORKING = 0,
  OVER_HEATED,   // 过热
  OVER_CURRENT,  // 过流
  OVER_VOLTAGE,  // 过压
  WRONG_ENCODER, // 编码器故障
} GO_M8010_Error_Type_e;

/**
 * @brief 格式化数据结构体
 *
 */
typedef struct {
  int16_t T = 0;   // 期望关节输出扭矩 unit: N.m
  int16_t W = 0;   // 期望关节输出速度 unit: rad/s
  int32_t Pos = 0; // 期望关节输出位置 unit: rad
  int16_t K_P = 0; // 期望关节刚度系数
  int16_t K_W = 0; // 期望关节阻尼系数
} motor_format_ref_data;

typedef struct {
  float T;   // 期望关节的输出力矩 （电机本身的力矩）unit：N.m
  float W;   // 期望关节速度（电机本身的速度）（rad/s）
  float Pos; // 期望关节位置（rad）
  float K_P; // 关节刚度系数
  float K_W; // 关节速度系数

} motor_real_ref_data;

/*-----------------------------------macro------------------------------------*/
/*
    扩展帧组成（从高位到低位）
    指令下发：模块id | 下发指示位 | 数据内容指示位 | 控制模式位 | 目标电机id |
   控制模式 | 预留位 | 预留位

    模块上传：
 */

/* 高位2bits作为id标识符 */
/* 模块id 2bits 故最多有3个CAN转RS485模块 */
#define CAN_To_RS485_Module_ID_0 (0 << 27)
#define CAN_To_RS485_Module_ID_1 (1 << 27)
#define CAN_To_RS485_Module_ID_2 (2 << 27)
#define CAN_To_RS485_Module_ID_3 (3 << 27)

/* 指令下发为0 模块上传为1,占1bit */
#define GO1_Send_To_Module (0 << 26)
#define GO1_Rec_From_Module (1 << 26)

/* 数据内容指示位,占2bits */
#define GO1_Send_Data_Mode (0 << 24)
#define GO1_Rec_Data_Mode1 (1 << 24)
#define GO1_Rec_Data_Mode2 (2 << 24)

/* ---------------------低位3--------------------- */
/* 控制模式位 占1Byte */
#define GO1_Ctrl_Mode_1 (10 << 16)
#define GO1_Ctrl_Mode_2 (11 << 16)
#define GO1_Ctrl_Mode_3 (12 << 16)
#define GO1_Ctrl_Mode_4 (13 << 16)

/* ---------------------低位2--------------------- */
/* 预留位 */
#define GO1_Send_Null_1Bits (0 << 15) // 1bit的保留位

/* 电机控制模式 占3bits */
#define GO1_Default_Mode (0 << 12)        // 锁定
#define GO1_FOC_Closer_Mode (1 << 12)     // FOC闭环
#define GO1_Encoder_calibration (2 << 12) // 编码器校准模式
// 剩下0x3~0x7 全为预留位

/* 目标电机id 总线上最多挂载15个GO1电机（理论上）占4bits */
#define GO1_Motor_ID_0 (0 << 8)
#define GO1_Motor_ID_1 (1 << 8)
#define GO1_Motor_ID_2 (2 << 8)
#define GO1_Motor_ID_3 (3 << 8)
#define GO1_Motor_ID_4 (4 << 8)
#define GO1_Motor_ID_5 (5 << 8)
#define GO1_Motor_ID_6 (6 << 8)
#define GO1_Motor_ID_7 (7 << 8)
#define GO1_Motor_ID_8 (8 << 8)
#define GO1_Motor_ID_9 (9 << 8)
#define GO1_Motor_ID_10 (10 << 8)
#define GO1_Motor_ID_11 (11 << 8)
#define GO1_Motor_ID_12 (12 << 8)
#define GO1_Motor_ID_13 (13 << 8)
#define GO1_Motor_ID_14 (14 << 8)
#define GO1_Motor_ID_15 (15 << 8)

/* ---------------------低位1--------------------- */
/* 发送预留位,占1Byte */
#define GO1_Send_Null_1Byte (0 << 0) // 1Byte的保留位

/*----------------------------------function----------------------------------*/

uint8_t CAN_To_RS485_Module_ID_Callback(uint8_t module_id);

uint8_t GO_Motor_ID_Callback(uint32_t motor_id);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

/**
 * @brief GO_M8010电机类
 *  reduction_ratio 不用设置的话就设置为 -1
 *
 */
class GO_M8010 : public Motor {
public:
  GO_M8010(uint8_t id, const CAN_Rx_Instance_t &can_rx_instance,
           const CAN_Tx_Instance_t &can_tx_instance,
           const Motor_Control_Setting_t &ctrl_config, int16_t max_current,
           float reduction_ratio, uint32_t module_id)
      : Motor(id, can_rx_instance, can_tx_instance, ctrl_config, max_current,
              reduction_ratio),
        module_id(module_id) {
    if (this->if_reduction == 1) {
      this->motor_reduction_ratio = 1; // 不使用减速箱
    } else if (this->if_reduction == -1) {
      this->motor_reduction_ratio = 6.33; // 使用默认减速比 6.33:1
    } else {
      this->motor_reduction_ratio = reduction_ratio; // 使用自制减速箱的减速比
    }
  }
  virtual ~GO_M8010() = default;

  /**
   * @brief GO1电机更新数据接口
   *
   * @param can_rx_data
   * @param data_id
   */
  void update_Go1(uint8_t can_rx_data[], uint32_t data_id);

  /**
   * @brief 力位混合控制接口
   *        每控制一次，就会返回一次数据，需要持续调用才能获取电机数据
   * @attention:注意，这里的位置控制和位置模式不同，电机会根据当前位置作为0点（此时电机pos不一定是0度），然后去做转动！
   * @param ref_kpos
   * @param ref_kspd
   * @param ref_torque
   * @param ref_speed
   * @param ref_pos
   * @return uint8_t
   */
  uint8_t GO_Motor_Standard_Ctrl(float ref_kpos, float ref_kspd,
                                 float ref_torque, float ref_speed,
                                 float ref_pos);

  /**
   * @brief 力位混合控制接口---省资源版
   *        不接收电机数据，如果总线挂载电机数量较多，可以选取这个函数
   *
   * @param ref_kpos
   * @param ref_kspd
   * @param ref_torque
   * @param ref_speed
   * @param ref_pos
   * @return uint8_t
   */
  uint8_t GO_Motor_SaveResource_Ctrl(float ref_kpos, float ref_kspd,
                                     float ref_torque, float ref_speed,
                                     float ref_pos);

  /**
   * @brief 位置模式接口（绝对编码器位置）
   *        输入期望位置，可以直接转到期望位置
   * @attention:
   * pd控制器，所以如果目标位置变化较大，那么电机产生的力矩也会很大！
   *             效果：电机会转到期望位置（即编码器的位置）
   *             注意GO1内置的是单圈绝对值编码器，掉电会将圈数重置
   * @param ref_pos
   * @param K_p
   * @param K_d
   * @return uint8_t
   */
  uint8_t GO_Motor_Pos_Ctrl(float ref_pos, float K_p, float K_d);

  /**
   * @brief 速度模式接口
   *       输入期望速度，电机的输出轴将会稳定在一个固定的速度
   *       此时是一个对速度的p控制器
   *
   * @param ref_speed
   * @param K_p
   * @return uint8_t
   */
  uint8_t GO_Motor_Speed_Ctrl(float refspeed, float K_p);

  /**
   * @brief 阻尼模式接口
   *        控制效果：有一个阻尼一样的效果，但是电机不会锁位置，有阻力但是会动
   *
   * @param k_dampping
   * @return uint8_t
   */
  uint8_t GO_Motor_Damping_Ctrl(float k_dampping);

  /**
   * @brief 零力矩模式接口
   *        电机的转动阻力会明显小于上电之前（感觉没什么用
   *        或许可以充当失能电机模式、电机重置模式
   *        因为这个接口的本质是将所有控制量置0
   *
   * @return uint8_t
   */
  uint8_t GO_Motor_No_Tarque_Ctrl();

  /* K_p K_w 数据回传接口 */
  /**
   * @brief  读取电机回传的Kpos 和 Kspd
   *         发送特定的命令，让电机可以回读设置的Kpos 和 Kspd
   *
   * @return uint8_t
   */
  uint8_t GO_Motor_ReadBack_Ctrl();

  /**
   * @brief 电机停转
   *
   */
  virtual void stop_the_motor() override;

public:
  /* 设置期望值,Kpos & Kspd */
  void Set_Ref_KParam(float ref_kpos, float ref_kspd) {
    this->Data_PreProcess_for_KParam(ref_kpos, ref_kspd);
  }
  /* 设置控制量参考值 */
  void Set_Ref_CtrlParam(float ref_torque, float ref_speed, float ref_pos) {
    this->Data_PreProcess_for_CtrlParam(ref_torque, ref_speed, ref_pos);
  }

  /* 发送数据预处理 */
  void Data_PreProcess_for_KParam(float ref_kpos, float ref_kspd) {
    /* 对期望真实值进行限幅 */
    motor_constraint(&(ref_kpos), static_cast<float>(0.0f),
                     static_cast<float>(25.599f)); // 对期望关节刚度系数限幅
    motor_constraint(&(ref_kspd), static_cast<float>(0.0f),
                     static_cast<float>(25.599f)); // 对期望关节阻尼系数限幅
    this->real_ref_data.K_P = ref_kpos;
    this->real_ref_data.K_W = ref_kspd;

    this->ref_send_data.K_P =
        (uint16_t)(ref_kpos * 1280.0f); // 1280 = 32768/256
    this->ref_send_data.K_W =
        (uint16_t)(ref_kspd * 1280.0f); // 1280 = 32768/256
  }

  void Data_PreProcess_for_CtrlParam(float ref_torque, float ref_speed,
                                     float ref_pos) {
    /* 对期望值进行减速比设置 --- 将关节速度映射到电机转子速度再发送 */
    this->real_ref_data.T = ref_torque * this->motor_reduction_ratio;
    this->real_ref_data.W = ref_speed * this->motor_reduction_ratio;
    this->real_ref_data.Pos = ref_pos * this->motor_reduction_ratio;

    motor_constraint(&(this->real_ref_data.T), static_cast<float>(-127.99f),
                     static_cast<float>(127.99f));
    motor_constraint(&(this->real_ref_data.W), static_cast<float>(-804.00f),
                     static_cast<float>(804.00f));
    motor_constraint(&(this->real_ref_data.Pos), static_cast<float>(-411774.0f),
                     static_cast<float>(411774.0f));

    this->ref_send_data.T =
        (uint16_t)(this->real_ref_data.T *
                   256.0f); // 乘上256，将浮点数用较小的带宽传输
    this->ref_send_data.W =
        (uint16_t)(this->real_ref_data.W * 256.0f / (2 * PI));
    this->ref_send_data.Pos =
        (uint32_t)(this->real_ref_data.Pos * 32768 / (2 * PI));
  }

  /* can帧发送数据打包 */
  virtual void CANMsg_Process() {
    if ((can_tx_for_motor.tx_id & 0xFF0000) == GO1_Ctrl_Mode_1) {
      // 控制模式位为10
      // @todo:"这个模式下，每发一次就会收到一次电机返回数据"这个怎么实现？
      can_tx_for_motor.can_tx_buff[7] = (this->ref_send_data.T >> 8) & 0xFF;
      can_tx_for_motor.can_tx_buff[6] = this->ref_send_data.T & 0xFF;
      can_tx_for_motor.can_tx_buff[5] = (this->ref_send_data.W >> 8) & 0xFF;
      can_tx_for_motor.can_tx_buff[4] = this->ref_send_data.W & 0xFF;
      can_tx_for_motor.can_tx_buff[3] = (this->ref_send_data.Pos >> 24) & 0xFF;
      can_tx_for_motor.can_tx_buff[2] = (this->ref_send_data.Pos >> 16) & 0xFF;
      can_tx_for_motor.can_tx_buff[1] = (this->ref_send_data.Pos >> 8) & 0xFF;
      can_tx_for_motor.can_tx_buff[0] = this->ref_send_data.Pos & 0xFF;
    } else if ((can_tx_for_motor.tx_id & 0xFF0000) == GO1_Ctrl_Mode_2) {
      // 控制模式位为11 设置 Kpos 和 Kspd
      can_tx_for_motor.can_tx_buff[7] = 0;
      can_tx_for_motor.can_tx_buff[6] = 0;
      can_tx_for_motor.can_tx_buff[5] = 0;
      can_tx_for_motor.can_tx_buff[4] = 0;
      can_tx_for_motor.can_tx_buff[3] = (this->ref_send_data.K_P >> 8) & 0xFF;
      can_tx_for_motor.can_tx_buff[2] = this->ref_send_data.K_P & 0xFF;
      can_tx_for_motor.can_tx_buff[1] = (this->ref_send_data.K_W >> 8) & 0xFF;
      can_tx_for_motor.can_tx_buff[0] = this->ref_send_data.K_W & 0xFF;
    } else if ((can_tx_for_motor.tx_id & 0xFF0000) == GO1_Ctrl_Mode_3) {
      // 控制模式位为12
      memset(can_tx_for_motor.can_tx_buff, 0,
             sizeof(can_tx_for_motor.can_tx_buff)); // 直接清零发送
    } else if ((can_tx_for_motor.tx_id & 0xFF0000) == GO1_Ctrl_Mode_4) {
      // 控制模式位为13
      // @todo:这个模式下，发送控制信息除非在电机报错，否则不会有数据返回
      can_tx_for_motor.can_tx_buff[7] = (this->ref_send_data.T >> 8) & 0xFF;
      can_tx_for_motor.can_tx_buff[6] = this->ref_send_data.T & 0xFF;
      can_tx_for_motor.can_tx_buff[5] = (this->ref_send_data.W >> 8) & 0xFF;
      can_tx_for_motor.can_tx_buff[4] = this->ref_send_data.W & 0xFF;
      can_tx_for_motor.can_tx_buff[3] = (this->ref_send_data.Pos >> 24) & 0xFF;
      can_tx_for_motor.can_tx_buff[2] = (this->ref_send_data.Pos >> 16) & 0xFF;
      can_tx_for_motor.can_tx_buff[1] = (this->ref_send_data.Pos >> 8) & 0xFF;
      can_tx_for_motor.can_tx_buff[0] = this->ref_send_data.Pos & 0xFF;
    }
  }
  /* 错误代号解析 */
  void errorType_Get(uint8_t error_id) {
    switch (error_id) {
    case 0:
      this->error_type = NORMAL_WORKING;
      break;
    case 1:
      this->error_type = OVER_HEATED;
      break;
    case 2:
      this->error_type = OVER_CURRENT;
      break;
    case 3:
      this->error_type = OVER_VOLTAGE;
      break;
    case 4:
      this->error_type = WRONG_ENCODER;
      break;
    }
  }
  void air_pressure_parameters_Get(uint16_t data) {
    this->air_pressure_parameters = (float)data;
  }
  void temp_get(uint16_t data) { this->temp = (float)data; }

  /* 设置发送的扩展帧id */
  void Set_Send_Extid(uint32_t extid) {
    this->can_tx_for_motor.isExTid = true;
    this->can_tx_for_motor.tx_id = extid;
  }

  void processRxMsg() {
    // 处理接收到的数据
    this->real_cur_data.T =
        ((float)this->cur_rec_data.T) / 256.0f /
        this->motor_reduction_ratio; // 除以256，将整形再转化为浮点
    this->real_cur_data.W = (((float)this->cur_rec_data.W) / 256.0f) * 2 * PI /
                            this->motor_reduction_ratio;
    this->real_cur_data.Pos = ((float)this->cur_rec_data.Pos) * 2 * PI / 32768 /
                              this->motor_reduction_ratio;
  }

public:
  /* 期望和当前的真实速度 */
  motor_real_ref_data real_ref_data = {0};
  motor_real_ref_data real_cur_data = {0};
  /* 期望和当前被格式化后的数据 */
  motor_format_ref_data ref_send_data = {0};
  motor_format_ref_data cur_rec_data = {0};

  GO_M8010_Error_Type_e error_type =
      NORMAL_WORKING; // 电机错误标识: 0.正常 1.过热 2.过流 3.过压 4.编码器故障
                      // 5-7.保留
  float air_pressure_parameters = 0;
  int temp;
  uint32_t module_id = 0;

private:
  float last_ref_kpos = 0;
  float last_ref_kspd = 0;

public:
  /* 这里是不适合原父类中虚函数的实现，不写会报错，暂时没想到什么优雅的实现方法，反正这下面的不用管就是了
   */
  virtual void update(uint8_t can_rx_data[]) {}
  virtual void CanMsg_Process(CAN_Tx_Instance_t &can_txmsg) {}
  virtual void set_motor_ref(float ref) {};
  virtual void enable_the_motor() {};
  virtual void pid_control_to_motor() {};
};

/*----------------------------------variable----------------------------------*/

#endif

#endif /* UNITREE_GO1_H */
