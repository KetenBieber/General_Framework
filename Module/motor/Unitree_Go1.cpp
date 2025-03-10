/**
 * @file Unitree_Go1.cpp
 * @author Keten (2863861004@qq.com)
 * @brief
 * @version 0.1
 * @date 2024-11-02
 *
 * @copyright Copyright (c) 2024
 *
 * @attention :
 * @note :
 * @versioninfo :
 */
#include "Unitree_Go1.h"

// 需要一个根据id来判断是哪个模块的函数，因为最多可能会有4个模块id，而且扩展帧拆开来那两位是模块id，应该象征性做好封装
uint8_t CAN_To_RS485_Module_ID_Callback(uint8_t module_id) {
  if (module_id == (CAN_To_RS485_Module_ID_0 >> 27)) // 如果是模块1
  {
    return 0;
  } else if (module_id == (CAN_To_RS485_Module_ID_1 >> 27)) {
    return 1;
  } else if (module_id == (CAN_To_RS485_Module_ID_2 >> 27)) {
    return 2;
  } else if (module_id == (CAN_To_RS485_Module_ID_3 >> 27)) {
    return 3;
  }

  return -1;
}

// 再需要一个函数来区分是一个模块上的哪个电机,这里传进数据帧，进行解析
// 只要返回值>=0，就说明是这个模块上的电机
// 目前先写这两个id，后续再加
uint8_t GO_Motor_ID_Callback(uint32_t motor_id) {
  // 保险起见，也算是把它的报文尽其用了
  if ((motor_id & 0x4000000) != GO1_Rec_From_Module) // 判断是不是发回来的报文
  {
    return -1;
  }
  uint8_t temp_motor_id = (motor_id & 0xF00);
  if (temp_motor_id == GO1_Motor_ID_0) {
    return 0;
  } else if (temp_motor_id == GO1_Motor_ID_1) {
    return 1;
  } else if (temp_motor_id == GO1_Motor_ID_2) {
    return 2;
  } else if (temp_motor_id == GO1_Motor_ID_3) {
    return 3;
  }
  return -2;
}

// 10 1 00 0000 01 0 00000000
// 标准的发送函数，就是发送一个Kpos 和 Kspd之后(控制模式11),后续使用控制模式10
uint8_t GO_M8010::GO_Motor_Standard_Ctrl(float ref_kpos, float ref_kspd,
                                         float ref_torque, float ref_speed,
                                         float ref_pos) {
  // 判断发送的数据有没有变化，如果有变化，就需要重新发送Kpos 和 Kspd
  if (ABS(this->real_ref_data.K_P - ref_kpos) > 0.0000001 ||
      ABS(this->real_ref_data.K_W - ref_kspd) > 0.0000001) {
    // 想要修改Kpos 和 Kspd
    this->Set_Ref_KParam(ref_kpos, ref_kspd);
    this->Set_Send_Extid(this->module_id << 27 | GO1_Send_To_Module |
                         GO1_Send_Data_Mode | GO1_Ctrl_Mode_2 | this->ID << 8 |
                         GO1_Default_Mode | GO1_Send_Null_1Bits |
                         GO1_Send_Null_1Byte);
  } else {
    this->Set_Ref_CtrlParam(ref_torque, ref_speed, ref_pos);
    // 说明Kpos 和
    // Kspd没有变化，不需要再使用控制模式11发送一次，直接使用控制模式10发送
    this->Set_Send_Extid(this->module_id << 27 | GO1_Send_To_Module |
                         GO1_Send_Data_Mode | GO1_Ctrl_Mode_1 | this->ID << 8 |
                         GO1_FOC_Closer_Mode | GO1_Send_Null_1Bits |
                         GO1_Send_Null_1Byte);
  }
  this->CANMsg_Process();

  if (this->can_tx_for_motor.can_handle == &hcan1)
    xQueueSend(CAN1_TxPort, &this->can_tx_for_motor, portMAX_DELAY);
  if (this->can_tx_for_motor.can_handle == &hcan2)
    xQueueSend(CAN2_TxPort, &this->can_tx_for_motor, portMAX_DELAY);
  //   COMMON_Motor_SendSingleMsg(*this);
  return 0;
}

// 节省资源的发送函数，就是只发送一次Kpos 和
// Kspd之后（控制模式11），后续使用控制模式13
uint8_t GO_M8010::GO_Motor_SaveResource_Ctrl(float ref_kpos, float ref_kspd,
                                             float ref_torque, float ref_speed,
                                             float ref_pos) {
  // 判断发送的数据有没有变化，如果有变化，就需要重新发送Kpos 和 Kspd
  if (ABS(this->real_ref_data.K_P - ref_kpos) > 0.0000001 ||
      ABS(this->real_ref_data.K_W - ref_kspd) > 0.0000001) {
    // 想要修改Kpos 和 Kspd
    this->Set_Ref_KParam(ref_kpos, ref_kspd);
    this->Set_Send_Extid(this->module_id << 27 | GO1_Send_To_Module |
                         GO1_Send_Data_Mode | GO1_Ctrl_Mode_2 | this->ID << 8 |
                         GO1_Default_Mode | GO1_Send_Null_1Bits |
                         GO1_Send_Null_1Byte);
  } else {
    // 说明Kpos 和
    // Kspd没有变化，不需要再使用控制模式11发送一次，直接使用控制模式13发送
    this->Set_Send_Extid(this->module_id << 27 | GO1_Send_To_Module |
                         GO1_Send_Data_Mode | GO1_Ctrl_Mode_4 | this->ID << 8 |
                         GO1_FOC_Closer_Mode | GO1_Send_Null_1Bits |
                         GO1_Send_Null_1Byte);
    this->Set_Ref_CtrlParam(ref_torque, ref_speed, ref_pos);
  }
  this->CANMsg_Process();

  if (this->can_tx_for_motor.can_handle == &hcan1)
    xQueueSend(CAN1_TxPort, &this->can_tx_for_motor, portMAX_DELAY);
  if (this->can_tx_for_motor.can_handle == &hcan2)
    xQueueSend(CAN2_TxPort, &this->can_tx_for_motor, portMAX_DELAY);
  return 0;
}

uint8_t GO_M8010::GO_Motor_Pos_Ctrl(float ref_pos, float K_p, float K_d) {
  if (ABS(this->real_ref_data.K_P - K_p) > 0.0000001 ||
      ABS(this->real_ref_data.K_W - K_d) > 0.0000001) {
    // 想要修改Kpos 和 Kspd
    this->Set_Ref_KParam(K_p, K_d);
    this->Set_Send_Extid(this->module_id << 27 | GO1_Send_To_Module |
                         GO1_Send_Data_Mode | GO1_Ctrl_Mode_2 | this->ID << 8 |
                         GO1_Default_Mode | GO1_Send_Null_1Bits |
                         GO1_Send_Null_1Byte);
  } else {
    // 说明Kpos 和 Kspd
    // 没有变化，不需要再使用控制模式11发送一次，直接使用控制模式1发送
    this->Set_Send_Extid(this->module_id << 27 | GO1_Send_To_Module |
                         GO1_Send_Data_Mode | GO1_Ctrl_Mode_1 | this->ID << 8 |
                         GO1_FOC_Closer_Mode | GO1_Send_Null_1Bits |
                         GO1_Send_Null_1Byte);
    this->Set_Ref_CtrlParam(0.0f, 0.0f, ref_pos);
  }
  this->CANMsg_Process();

  if (this->can_tx_for_motor.can_handle == &hcan1)
    xQueueSend(CAN1_TxPort, &this->can_tx_for_motor, portMAX_DELAY);
  if (this->can_tx_for_motor.can_handle == &hcan2)
    xQueueSend(CAN2_TxPort, &this->can_tx_for_motor, portMAX_DELAY);
  return 0;
}

uint8_t GO_M8010::GO_Motor_Speed_Ctrl(float refspeed, float K_p) {

  if (ABS(this->real_ref_data.K_W - K_p) >= 0.0000001) {
    // 想要修改Kspd,速度模式K_pos必须为0
    this->Set_Ref_KParam(0, K_p);
    this->Set_Send_Extid(this->module_id << 27 | GO1_Send_To_Module |
                         GO1_Send_Data_Mode | GO1_Ctrl_Mode_2 | this->ID << 8 |
                         GO1_Default_Mode | GO1_Send_Null_1Bits |
                         GO1_Send_Null_1Byte);
  } else {
    // 说明Kspd没有变化，不需要再使用控制模式11发送一次，直接使用控制模式1发送
    // 速度模式ref_T 必须为0
    this->Set_Send_Extid(this->module_id << 27 | GO1_Send_To_Module |
                         GO1_Send_Data_Mode | GO1_Ctrl_Mode_1 | this->ID << 8 |
                         GO1_FOC_Closer_Mode | GO1_Send_Null_1Bits |
                         GO1_Send_Null_1Byte);
    this->Set_Ref_CtrlParam(0.0f, refspeed, 0.0f);
  }

  this->CANMsg_Process();
  if (this->can_tx_for_motor.can_handle == &hcan1)
    xQueueSend(CAN1_TxPort, &this->can_tx_for_motor, portMAX_DELAY);
  if (this->can_tx_for_motor.can_handle == &hcan2)
    xQueueSend(CAN2_TxPort, &this->can_tx_for_motor, portMAX_DELAY);
  this->last_ref_kspd = K_p;
  return 0;
}

uint8_t GO_M8010::GO_Motor_Damping_Ctrl(float k_dampping) {
  if (ABS(this->real_ref_data.K_W - k_dampping) > 0.0000001) {
    this->Set_Ref_KParam(0, k_dampping);
    this->Set_Send_Extid(this->module_id << 27 | GO1_Send_To_Module |
                         GO1_Send_Data_Mode | GO1_Ctrl_Mode_2 | this->ID << 8 |
                         GO1_Default_Mode | GO1_Send_Null_1Bits |
                         GO1_Send_Null_1Byte);
  } else {
    /* 其他控制量必须置0 */
    this->Set_Ref_CtrlParam(0.0f, 0.0f, 0.0f);
    this->Set_Send_Extid(this->module_id << 27 | GO1_Send_To_Module |
                         GO1_Send_Data_Mode | GO1_Ctrl_Mode_1 | this->ID << 8 |
                         GO1_FOC_Closer_Mode | GO1_Send_Null_1Bits |
                         GO1_Send_Null_1Byte);
  }
  this->CANMsg_Process();

  if (this->can_tx_for_motor.can_handle == &hcan1)
    xQueueSend(CAN1_TxPort, &this->can_tx_for_motor, portMAX_DELAY);
  if (this->can_tx_for_motor.can_handle == &hcan2)
    xQueueSend(CAN2_TxPort, &this->can_tx_for_motor, portMAX_DELAY);
  return 0;
}

uint8_t GO_M8010::GO_Motor_No_Tarque_Ctrl() {
  // 判断当前的数据中K_pos和K_spd是不是0，不是的话就需要重新发送Kpos 和 Kspd
  if (ABS(this->real_ref_data.K_P - 0) > 0.0000001 ||
      ABS(this->real_ref_data.K_W - 0) > 0.0000001) {
    // 想要修改Kpos 和 Kspd
    this->Set_Ref_KParam(0.0f, 0.0f);
    this->Set_Send_Extid(this->module_id << 27 | GO1_Send_To_Module |
                         GO1_Send_Data_Mode | GO1_Ctrl_Mode_2 | this->ID << 8 |
                         GO1_Default_Mode | GO1_Send_Null_1Bits |
                         GO1_Send_Null_1Byte);
  } else {
    this->Set_Ref_CtrlParam(0.0f, 0.0f, 0.0f);
    // 说明Kpos 和
    // Kspd没有变化，不需要再使用控制模式11发送一次，直接使用控制模式10发送
    this->Set_Send_Extid(this->module_id << 27 | GO1_Send_To_Module |
                         GO1_Send_Data_Mode | GO1_Ctrl_Mode_1 | this->ID << 8 |
                         GO1_FOC_Closer_Mode | GO1_Send_Null_1Bits |
                         GO1_Send_Null_1Byte);
  }
  this->CANMsg_Process();

  if (this->can_tx_for_motor.can_handle == &hcan1)
    xQueueSend(CAN1_TxPort, &this->can_tx_for_motor, portMAX_DELAY);
  if (this->can_tx_for_motor.can_handle == &hcan2)
    xQueueSend(CAN2_TxPort, &this->can_tx_for_motor, portMAX_DELAY);

  return 0;
}

// 发送命令告诉电机，请回传Kpos 和 Kspd
uint8_t GO_M8010::GO_Motor_ReadBack_Ctrl() {
  // 发送控制模式是12
  this->Set_Send_Extid(this->module_id << 27 | GO1_Send_To_Module |
                       GO1_Send_Data_Mode | GO1_Ctrl_Mode_3 | this->ID << 8 |
                       GO1_FOC_Closer_Mode | GO1_Send_Null_1Bits |
                       GO1_Send_Null_1Byte);
  this->CANMsg_Process();
  if (this->can_tx_for_motor.can_handle == &hcan1)
    xQueueSend(CAN1_TxPort, &this->can_tx_for_motor, portMAX_DELAY);
  if (this->can_tx_for_motor.can_handle == &hcan2)
    xQueueSend(CAN2_TxPort, &this->can_tx_for_motor, portMAX_DELAY);
  return 0;
}

void GO_M8010::stop_the_motor() {

  if (ABS(this->real_ref_data.K_P - 0) > 0.0000001 ||
      ABS(this->real_ref_data.K_W - 0.005) > 0.0000001) {
    // 想要修改Kpos 和 Kspd
    this->Set_Ref_KParam(0, 0.005);
    this->Set_Send_Extid(this->module_id << 27 | GO1_Send_To_Module |
                         GO1_Send_Data_Mode | GO1_Ctrl_Mode_2 | this->ID << 8 |
                         GO1_Default_Mode | GO1_Send_Null_1Bits |
                         GO1_Send_Null_1Byte);
  } else {
    this->Set_Ref_CtrlParam(0.0f, 0.0f, this->real_cur_data.Pos);
    this->Set_Send_Extid(this->module_id << 27 | GO1_Send_To_Module |
                         GO1_Send_Data_Mode | GO1_Ctrl_Mode_1 | this->ID << 8 |
                         GO1_FOC_Closer_Mode | GO1_Send_Null_1Bits |
                         GO1_Send_Null_1Byte);
  }
  this->CANMsg_Process();
  if (this->can_tx_for_motor.can_handle == &hcan1)
    xQueueSend(CAN1_TxPort, &this->can_tx_for_motor, portMAX_DELAY);
  if (this->can_tx_for_motor.can_handle == &hcan2)
    xQueueSend(CAN2_TxPort, &this->can_tx_for_motor, portMAX_DELAY);
}

// 在这里面，先将剩余扩展帧中的数据拿出来，判断接收模式，然后再进行对应的数据段解析
void GO_M8010::update_Go1(uint8_t can_rx_data[], uint32_t data_id) {
  // 先根据低位1 来判断是什么接收数据模式
  if ((data_id & 0x3000000) == GO1_Rec_Data_Mode1) // 接收数据模式1
  {
    // 还需要根据最后一位是不是-128，来判断电机有无报错
    if ((uint16_t)(data_id & 0xFF) == -128) {
      // 电机发回来报错报文！
      this->errorType_Get((data_id & 0xFF0000) >>
                          16); // 进行错误代号解析，具体看错误数据类型
    } else {
      // 电机正常发回来报文！这时记得存储温度
      this->air_pressure_parameters_Get((uint16_t)((data_id & 0xFF0000) >> 16));
      this->temp_get((uint16_t)(data_id & 0xFF));
    }
    // 无论电机是否报错，都会发送回来数据段
    this->cur_rec_data.T = (int16_t)(can_rx_data[7] << 8 | can_rx_data[6]);
    this->cur_rec_data.W = (int16_t)(can_rx_data[5] << 8 | can_rx_data[4]);
    this->cur_rec_data.Pos =
        (int32_t)(can_rx_data[3] << 24 | can_rx_data[2] << 16 |
                  can_rx_data[1] << 8 | can_rx_data[0]);
  } else if ((data_id & 0x3000000) == GO1_Rec_Data_Mode2) // 接收数据模式2
  {
    // 对应指令下发中 模式12 ，读取Kpos 和 Kspd模式，因此只返回Kpos 和 Kspd
    this->cur_rec_data.K_P = (int16_t)(can_rx_data[3] << 8 | can_rx_data[2]);
    this->cur_rec_data.K_W = (int16_t)(can_rx_data[1] << 8 | can_rx_data[0]);
  }
  this->processRxMsg();
}
