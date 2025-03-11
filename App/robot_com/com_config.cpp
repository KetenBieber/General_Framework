/**
 * @file com_config.cpp
 * @author Keten (2863861004@qq.com)
 * @brief 
 * @version 0.1
 * @date 2024-10-05
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention :
 * @note :      2024-11-03 修改CAN2接收回调函数，使其适配宇树go1 电机的数据解析
 *                         移除了TEST_SYSTEM_M2006的测试
 * 
 *              2025-1-17  当前ros通讯协议 stm32 -> ros
 *                         数据位1：指令位
 *                         数据位2：twist.x
 *                         数据位3：twist.y
 *                         数据位4：twist.z
 *                         数据为5：NULL
 *                         数据位6：NULL
 *                         发布twist话题：ros_serial_pub
 * 
 *              2025-1-22  对自动的接入维护：当控制命令输入“自动”时，需要检查是否开启上位机数据接收
 *                         如果设置为自动，但是没有接入上位机数据，则不执行自动控制
 * @versioninfo :
 */
#include "com_config.h"
#include "topics.h"

extern Motor_C610 m2006[1];

extern GO_M8010 go1_motor[1];

osThreadId_t CAN1_Send_TaskHandle;
osThreadId_t CAN2_Send_TaskHandle;
osThreadId_t ROSCOM_TaskHandle;

QueueHandle_t CAN1_TxPort;
QueueHandle_t CAN2_TxPort;


extern Motor_GM6020 gm6020[1];

#ifdef USE_OMNI_CHASSIS
// 全向底盘驱动电机
extern Motor_C620 chassis_motor[4];
#endif

// 舵向电机实例
extern Motor_GM6020 rubber_motor[4];

uint8_t Common_Service_Init()
{
    CAN1_TxPort = xQueueCreate(16,sizeof(CAN_Tx_Instance_t));
    CAN2_TxPort = xQueueCreate(16,sizeof(CAN_Tx_Instance_t));
    SubPub_Init();// 话题订阅机制开启

    return 1;
}

#ifdef TEST_SYSTEM_GM6020
float motor_acceleration = 0;
float angle = 0;
float motor_current = 0;
#endif



static void ros_serial_fsm(uint8_t _flag);


void CAN1_Rx_Callback(CAN_Rx_Instance_t *can_instance)
{
    if(can_instance->RxHeader.IDE == CAN_ID_STD)
    {
        switch(can_instance->RxHeader.StdId)
        {
#ifdef USE_OMNI_CHASSIS
            case 0x201:
            {                
                chassis_motor[0].update(can_instance->can_rx_buff);
                break;
            }
            case 0x202:
            {
                chassis_motor[1].update(can_instance->can_rx_buff);
                break;
            }
            case 0x203:
            {
                chassis_motor[2].update(can_instance->can_rx_buff);
                break;
            }
            case 0x204:
            {
                chassis_motor[3].update(can_instance->can_rx_buff);
                break;
            }
#endif

#ifdef TEST_SYSTEM_GM6020
            case 0x205:
            {
                gm6020[0].update(can_instance->can_rx_buff);
                angle = gm6020[0].angle;
                break;
            }
#endif

#ifdef USE_SWERVE_CHASSIS// 舵向电机
            case 0x205:
            {
                rubber_motor[0].update(can_instance->can_rx_buff);
                break;
            }
            case 0x206:
            {
                rubber_motor[1].update(can_instance->can_rx_buff);
                break;
            }
            case 0x207:
            {
                rubber_motor[2].update(can_instance->can_rx_buff);
                break;
            }
            case 0x208:
            {
                rubber_motor[3].update(can_instance->can_rx_buff);
                break;
            }
#endif
        }
    }
    if(can_instance->RxHeader.IDE == CAN_ID_EXT)
    {
        switch(can_instance->RxHeader.ExtId)
        {
            case 0x201:
            {
                break;
            }
        }
    }
}

// 现在打算can2专控go1电机，当然也可以换上别的电机，只是暂时先用
void CAN2_Rx_Callback(CAN_Rx_Instance_t *can_instance)
{
    uint32_t data_of_id = (uint32_t)can_instance->RxHeader.ExtId & 0x07FFFFFF;
    uint8_t temp_module_id = CAN_To_RS485_Module_ID_Callback((uint8_t)(can_instance->RxHeader.ExtId >> 27) & 0x03);// 解析出标识符（模块id）
    uint8_t temp_motor_id = GO_Motor_ID_Callback(data_of_id);// 解析出扩展帧中的数据部分
#ifdef DEBUG_GO1_MOTOR
    if(temp_motor_id < 0)
    {
        return;
    }
    switch(temp_module_id)
    {
        case 0:
            break;
        case 1:
            break;
        case 2:
            break;
        case 3:// 模块出厂id为3
            go1_motor[temp_motor_id].update_Go1(can_instance->can_rx_buff,data_of_id);
            break;
    }
#endif
    // 如果不是go1的协议，为大疆电机的协议，则会进入这个分支
     if(can_instance->RxHeader.IDE == CAN_ID_STD)
    {
        switch(can_instance->RxHeader.StdId)
        {
#ifdef USE_SWERVE_CHASSIS       
            // 轮向电机     
            case 0x205:
            {
                break;
            }
            case 0x206:
            {
                break;
            }
            case 0x207:
            {
                break;
            }
            case 0x208:
            {
                break;
            }
#endif
            default:
            // 处理未知的标准ID
            break;
        }
    }
    else
    {
        switch(can_instance->RxHeader.ExtId)
        {
            case 0x201:
            {
                break;
            }
        }
    }

}


__attribute((noreturn)) void CAN1_Send_Task(void *argument)
{
    CAN_Tx_Instance_t temp_can_txmsg;
    uint8_t free_can_mailbox;
    for(;;)
    {
        if(xQueueReceive(CAN1_TxPort,&temp_can_txmsg,0) == pdTRUE)
        {
            do{
                free_can_mailbox = HAL_CAN_GetTxMailboxesFreeLevel(&hcan1);
            }while(free_can_mailbox == 0);
            if(temp_can_txmsg.isExTid == 1)// 发送扩展帧
                CAN_Transmit_ExtId(&temp_can_txmsg);
            else    // 发送标准帧
                CAN_Transmit_StdID(&temp_can_txmsg);
        }
        osDelay(1);
    }
}


__attribute((noreturn)) void CAN2_Send_Task(void *argument)
{
    CAN_Tx_Instance_t temp_can_txmsg;
    uint8_t free_can_mailbox;
    for(;;)
    {
        if(xQueueReceive(CAN2_TxPort,&temp_can_txmsg,0) == pdTRUE)
        {
            do{
                free_can_mailbox = HAL_CAN_GetTxMailboxesFreeLevel(&hcan2);
            }while(free_can_mailbox == 0);
            if(temp_can_txmsg.isExTid == 1)     // 发送扩展帧
                CAN_Transmit_ExtId(&temp_can_txmsg);
            else    // 发送标准帧
                CAN_Transmit_StdID(&temp_can_txmsg);
        }
        osDelay(1);
    }
}


extern ROS_Com_Instance_t *ros_instance;

__attribute((noreturn)) void ROSCOM_Task(void *argument)
{
    uint8_t task_flag = 0;
    if(ROS_Communication_Init() != 1)
    {
        LOGERROR("ros com failed!");// 初始化失败，直接自杀
        vTaskDelete(NULL);
    }
    /* ros接收到信息向其他任务发布信息的发布者初始化 */
    publish_data temp_data;
    pub_Control_Data ros_twist;
    Publisher *twist_pub = register_pub("ros_serial_pub");

    Subscriber *start_rev_cmd;
    start_rev_cmd = register_sub("ctrl_pub",1);
    publish_data cmd_data;
    pub_Control_Data start_rev_data;
    for(;;)
    {
        cmd_data = start_rev_cmd->getdata(start_rev_cmd);
        if(cmd_data.len != -1)
        {
            start_rev_data = *(pub_Control_Data*)cmd_data.data;
            task_flag = start_rev_data.ctrl;
        }
        ros_serial_fsm(task_flag);
        if(ROSCOM_Task_Function(ros_instance))// 解包得到数据
        {
            ros_twist.linear_x = ros_instance->data_get[0];
            ros_twist.linear_y = ros_instance->data_get[1];
            ros_twist.Omega = ros_instance->data_get[2];
            temp_data.data = (uint8_t*)&ros_twist;
            temp_data.len = sizeof(pub_Control_Data);
            twist_pub->publish(twist_pub,temp_data);// 发布自动控制速度指令
        }
        osDelay(1);
    }
}

enum
{
    ASK_FOR_REQUEST = 0,
    START_TALKING = 1,
    STOP_TALKING = 2,
};


static void ros_serial_fsm(uint8_t _flag)
{
    float send[6] = {0};
    // ros通讯设备状态机
    switch(_flag)
    {
        case ASK_FOR_REQUEST:
        {
            /* 发送请求指令 */
            send[0] = 1;
            ROSCom_SendData(send);
            osDelay(1000);// 1s发一次，等待回复
            break;
        }
        case START_TALKING:
        {
            /* 可能需要将速度上传回电脑进行速度闭环，目前先不这么做 */
            send[0] = 1;
            break;
        }
        case STOP_TALKING:
        {
            send[0] = 2;// 发送该指令，小电脑停止发送数据
            break;
        }
    }
}


