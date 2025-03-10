/**
 * @file xbox.h
 * @author 冯大帅将军转自原作者刘洁宇
 * @brief
 * @version 0.1
 * @date 2025-1-13
 *
 * @copyright Copyright (c) 2025
 *
 * @attention :   手柄adc值如下：(没标完)
 *
 *  左扳机 trigLT     0~1023(按下)                  右扳机 trigRT 0~1023(按下)
 *                                     btnXbox
 *                ↑↓:joyLVert
 *                     0                                       btnY
 *    左摇杆     0   32768   65535  ←→:joyLHori            btnX      btnB
 *                  65535                                      btnA
 * 目前为止,没有加入DWT驻车模式,因为xbox按键较多,所以通过sub-pub传递的是各按键的具体数据
 * ,而且考虑到有些按键不需要,或是说分别控制上下层不同机构等等,具体向什么任务传递
 * 哪些按键就有使用者自己写
 * 具体需要在xbox.h中的XboxControllerData_t->pub_Xbox_Data
 * xbox_data;跳转到pub_Xbox_Data里
 * 添加按键,然后就是在本文件里Xbox_Process()函数里填写赋值
 *
 * @note :
 * @versioninfo :
 */
#include "xbox.h"

serial_frame_esp32_t rx_frame_esp32;
XBOX_Instance_t *XBOX_Instance = NULL;

bool bt = 0;
uint8_t xbox_rx_buffer[1] = {0};

uart_package_t xbox_uart_package = {
    .uart_handle = &huart2,
    .use_static_length_data = 0, // 使用不固定长度数据
    .rx_buffer = xbox_rx_buffer, // 接收缓冲区
    .rx_buffer_size = 1,
    .uart_callback = Xbox_Uart_Rx_Callback, // 接收回调函数
    .IT_CHOOSE = 1,                         // 选择普通中断
};

// 当前状态机状态
static rx_state_esp32_t rx_state_esp32 = WAITING_FOR_HEADER_0_ESP32;
// 数据索引
static uint8_t rx_index_esp32 = 0;
// 临时接收数据存储
static uint8_t rx_temp_data_esp32[MAX_DATA_LENGTH_ESP32 * 4];

uint8_t Xbox_Init(Uart_Instance_t *xbox_uart_instance) {

  XBOX_Instance = (XBOX_Instance_t *)pvPortMalloc(sizeof(XBOX_Instance_t));

  if (XBOX_Instance == NULL) {
    LOGERROR("create xbox instance failed!");
    return 0;
  }
  memset(XBOX_Instance, 0, sizeof(XBOX_Instance_t));

  XBOX_Instance->xbox_msgs =
      (XboxControllerData_t *)pvPortMalloc(sizeof(XboxControllerData_t));

  XBOX_Instance->xbox_uart_instance = xbox_uart_instance;

  XBOX_Instance->xbox_msgs->btnY = 0;
  XBOX_Instance->xbox_msgs->btnY_last = 0;
  XBOX_Instance->xbox_msgs->btnB = 0;
  XBOX_Instance->xbox_msgs->btnB_last = 0;
  XBOX_Instance->xbox_msgs->btnA = 0;
  XBOX_Instance->xbox_msgs->btnA_last = 0;
  XBOX_Instance->xbox_msgs->btnX = 0;
  XBOX_Instance->xbox_msgs->btnX_last = 0;
  XBOX_Instance->xbox_msgs->btnShare = 0;
  XBOX_Instance->xbox_msgs->btnShare_last = 0;
  XBOX_Instance->xbox_msgs->btnSelect = 0;
  XBOX_Instance->xbox_msgs->btnSelect_last = 0;
  XBOX_Instance->xbox_msgs->btnXbox = 0;
  XBOX_Instance->xbox_msgs->btnXbox_last = 0;
  XBOX_Instance->xbox_msgs->btnLB = 0;
  XBOX_Instance->xbox_msgs->btnLB_last = 0;
  XBOX_Instance->xbox_msgs->btnRB = 0;
  XBOX_Instance->xbox_msgs->btnRB_last = 0;
  XBOX_Instance->xbox_msgs->btnLS = 0;
  XBOX_Instance->xbox_msgs->btnLS_last = 0;
  XBOX_Instance->xbox_msgs->btnRS = 0;
  XBOX_Instance->xbox_msgs->btnRS_last = 0;
  XBOX_Instance->xbox_msgs->btnDirUp = 0;
  XBOX_Instance->xbox_msgs->btnDirUp_last = 0;
  XBOX_Instance->xbox_msgs->btnDirLeft = 0;
  XBOX_Instance->xbox_msgs->btnDirLeft_last = 0;
  XBOX_Instance->xbox_msgs->btnDirRight = 0;
  XBOX_Instance->xbox_msgs->btnDirRight_last = 0;
  XBOX_Instance->xbox_msgs->btnDirDown = 0;
  XBOX_Instance->xbox_msgs->btnDirDown_last = 0;
  XBOX_Instance->xbox_msgs->joyLHori = 32768;
  XBOX_Instance->xbox_msgs->joyLVert = 32768;
  XBOX_Instance->xbox_msgs->joyRHori = 32768;
  XBOX_Instance->xbox_msgs->joyRVert = 32768;
  XBOX_Instance->xbox_msgs->trigLT = 0;
  XBOX_Instance->xbox_msgs->trigRT = 0;
  XBOX_Instance->xbox_msgs->joyLHori_map = 0;
  XBOX_Instance->xbox_msgs->joyLVert_map = 0;
  XBOX_Instance->xbox_msgs->joyRHori_map = 0;
  XBOX_Instance->xbox_msgs->joyRVert_map = 0;
  XBOX_Instance->xbox_msgs->trigLT_map = 0;
  XBOX_Instance->xbox_msgs->trigRT_map = 0;

  XBOX_Instance->xbox_pub = register_pub("xbox");

  XBOX_Instance->xbox_data.btnY = 0;
  XBOX_Instance->xbox_data.btnA = 0;
  XBOX_Instance->xbox_data.btnLB = 0;
  XBOX_Instance->xbox_data.trigLT = 0;
  XBOX_Instance->xbox_data.btnDirUp = 0;
  XBOX_Instance->xbox_data.btnDirDown = 0;
  XBOX_Instance->xbox_data.btnB = 0;
  
  return 1;
}

// 数据处理函数，使用状态机实现接收解包
uint8_t Xbox_Process(uint8_t byte) {
  switch (rx_state_esp32) {
  case WAITING_FOR_HEADER_0_ESP32:
    if (byte == FRAME_HEAD_0_ESP32) {
      rx_frame_esp32.frame_head[0] = byte;
      rx_state_esp32 = WAITING_FOR_HEADER_1_ESP32;
    }
    break;

  case WAITING_FOR_HEADER_1_ESP32:
    if (byte == FRAME_HEAD_1_ESP32) {
      rx_frame_esp32.frame_head[1] = byte;
      rx_state_esp32 = WAITING_FOR_ID_ESP32;
    } else {
      rx_state_esp32 = WAITING_FOR_HEADER_0_ESP32;
    }
    break;

  case WAITING_FOR_ID_ESP32:
    rx_frame_esp32.frame_id = byte;
    rx_state_esp32 = WAITING_FOR_LENGTH_ESP32;
    break;

  case WAITING_FOR_LENGTH_ESP32:
    rx_frame_esp32.data_length = byte;
    rx_index_esp32 = 0;
    rx_state_esp32 = WAITING_FOR_DATA_ESP32;
    break;

  case WAITING_FOR_DATA_ESP32:
    rx_temp_data_esp32[rx_index_esp32++] = byte;
    if (rx_index_esp32 >= rx_frame_esp32.data_length) {
      rx_state_esp32 = WAITING_FOR_CRC_0_ESP32;
    }
    break;

  case WAITING_FOR_CRC_0_ESP32:
    rx_frame_esp32.check_code_e.crc_buff[0] = byte;
    rx_state_esp32 = WAITING_FOR_CRC_1_ESP32;
    break;

  case WAITING_FOR_CRC_1_ESP32:
    rx_frame_esp32.check_code_e.crc_buff[1] = byte;
    rx_state_esp32 = WAITING_FOR_END_0_ESP32;
    break;

  case WAITING_FOR_END_0_ESP32:
    if (byte == FRAME_END_0_ESP32) {
      rx_frame_esp32.frame_end[0] = byte;
      rx_state_esp32 = WAITING_FOR_END_1_ESP32;
    } else {
      rx_state_esp32 = WAITING_FOR_HEADER_0_ESP32;
    }
    break;

  case WAITING_FOR_END_1_ESP32:
    if (byte == FRAME_END_1_ESP32) {
      rx_frame_esp32.frame_end[1] = byte;
      uint16_t received_crc = rx_frame_esp32.check_code_e.crc_code;
      // uint16_t calculated_crc = CRC16_Table(rx_temp_data_esp32,
      // rx_frame_esp32.data_length); rx_frame_esp32.crc_calculated =
      // calculated_crc;
      if (1) {
        for (uint8_t i = 0; i < rx_frame_esp32.data_length; i++) {
          rx_frame_esp32.data_buff.buff_msg[i] = rx_temp_data_esp32[i];
        }
        Xbox_Get_Data(rx_frame_esp32.data_buff.buff_msg, XBOX_Instance);
        rx_state_esp32 = WAITING_FOR_HEADER_0_ESP32;
        return rx_frame_esp32.frame_id;
      }
    }
    rx_state_esp32 = WAITING_FOR_HEADER_0_ESP32;
    break;

  default:
    rx_state_esp32 = WAITING_FOR_HEADER_0_ESP32;
    break;
  }
  Xbox_Publish(); // 发布数据
  return 0;
}

void Xbox_Get_Data(uint8_t *xbox_datas, XBOX_Instance_t *Instance) {
  // 解析按键数据 (bool 值)
  Instance->xbox_msgs->btnY = xbox_datas[0];
  Instance->xbox_msgs->btnB = xbox_datas[1];
  Instance->xbox_msgs->btnA = xbox_datas[2];
  Instance->xbox_msgs->btnX = xbox_datas[3];
  Instance->xbox_msgs->btnShare = xbox_datas[4];
  Instance->xbox_msgs->btnStart = xbox_datas[5];
  Instance->xbox_msgs->btnSelect = xbox_datas[6];
  Instance->xbox_msgs->btnXbox = xbox_datas[7];
  Instance->xbox_msgs->btnLB = xbox_datas[8];
  Instance->xbox_msgs->btnRB = xbox_datas[9];
  Instance->xbox_msgs->btnLS = xbox_datas[10];
  Instance->xbox_msgs->btnRS = xbox_datas[11];
  Instance->xbox_msgs->btnDirUp = xbox_datas[12];
  Instance->xbox_msgs->btnDirLeft = xbox_datas[13];
  Instance->xbox_msgs->btnDirRight = xbox_datas[14];
  Instance->xbox_msgs->btnDirDown = xbox_datas[15];

  // 解析霍尔传感器值（16位数据，高8位和低8位拼接）
  Instance->xbox_msgs->joyLHori =
      ((uint16_t)xbox_datas[16] << 8) | xbox_datas[17];
  Instance->xbox_msgs->joyLVert =
      ((uint16_t)xbox_datas[18] << 8) | xbox_datas[19];
  Instance->xbox_msgs->joyRHori =
      ((uint16_t)xbox_datas[20] << 8) | xbox_datas[21];
  Instance->xbox_msgs->joyRVert =
      ((uint16_t)xbox_datas[22] << 8) | xbox_datas[23];
  Instance->xbox_msgs->trigLT =
      ((uint16_t)xbox_datas[24] << 8) | xbox_datas[25];
  Instance->xbox_msgs->trigRT =
      ((uint16_t)xbox_datas[26] << 8) | xbox_datas[27];

  // 这里对要传递的xbox数据赋值
  Instance->xbox_data.btnY = Instance->xbox_msgs->btnY;
  Instance->xbox_data.btnA = Instance->xbox_msgs->btnA;
  Instance->xbox_data.btnLB = Instance->xbox_msgs->btnLB;
  Instance->xbox_data.trigLT = Instance->xbox_msgs->trigLT;
  Instance->xbox_data.btnDirUp = Instance->xbox_msgs->btnDirUp;
  Instance->xbox_data.btnDirDown = Instance->xbox_msgs->btnDirDown; 
  Instance->xbox_data.btnB = Instance->xbox_msgs->btnB;
  //..........
}

bool Button_Continue(bool currentBtnState, bool *lastBtnState) {

  if (currentBtnState && !(*lastBtnState)) { // 检测到上升沿
    bt = 1;
  } else if (!currentBtnState && (*lastBtnState)) { // 检测到下降沿
    bt = 0;
  }
  *lastBtnState = currentBtnState;
  return bt;
}

bool Button_Switch(bool currentBtnState, bool *lastBtnState) {
  if (currentBtnState && !(*lastBtnState)) { // 检测到上升沿
    *lastBtnState = currentBtnState;
    return true;
  }
  *lastBtnState = currentBtnState;
  return false;
}

uint8_t Xbox_Publish() {
  publish_data temp_data;
  temp_data.data = (uint8_t *)&XBOX_Instance->xbox_data;
  temp_data.len = sizeof(pub_Xbox_Data);
  XBOX_Instance->xbox_pub->publish(XBOX_Instance->xbox_pub, temp_data);
  return 1;
}

uint8_t Xbox_Uart_Rx_Callback(Uart_Instance_t *uart_instance,
                              uint16_t data_len) {

  if (uart_instance == NULL) {
    LOGERROR("xbox uart_instance is NULL!");
    return 0;
  }
  // Uart_Instance_t *temp_uart_instance = (Uart_Instance_t*)uart_instance;
  // memcpy(aaa, temp_uart_instance->uart_package.rx_buffer,
  // sizeof(temp_uart_instance->uart_package.rx_buffer)); for (uint16_t i = 0; i
  // < data_len; i++)
  // {

  Xbox_Process(uart_instance->uart_package.rx_buffer[0]);
  // }
  return 0;
}
