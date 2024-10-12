/**
 * @file chassis_task.cpp
 * @author Keten (2863861004@qq.com)
 * @brief 底盘任务 在这里完成底盘所有机构的初始化 对于每一个机器人，在这里进行底盘的选型以及参数的设置
 *        注意大疆电机作为一款同时具有发送和接收的can设备，较其他can设备来说更加特别的在于，发送电机控制帧时
 *        可以在一帧can中装载4个电机的控制量，也就是说控制4个电机无需发送4帧can，只需一帧就可以完成
 *        VESC电调的can发送帧一帧是只能包括一个电机的控制量，所以需要发送4帧才能控制4个电机
 * 
 *        注意这里存的是底盘的运动控制，不细分到电机！电机的控制将会集成在电机module中
 *        也就是说app层操作的是电机的外环
 *        电机内部电流环、速度环or位置环 封装在电机内部
 * 
 * 
 * @version 0.1
 * @date 2024-10-04
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention :
 * @note :
 * @versioninfo :
 * @todo :使用扭矩进行底盘控制（并且加入扭矩前馈），将坐标系换成更加通用的 上z前x左y
 *        丰富底盘种类，目前打算加入：三全向轮、四全向轮、四舵轮、三舵轮  ---同时包括运动学正解算以及逆解算
 */
#include "chassis_task.h"
#include "bsp_can.h"
#include "rm_motor.h"
#include "Chassis.h"
#include "Omni_Chassis.h"


/* 即使是大疆电机，也最好做好定义,统一can设备使用方式 */

/* m3508电机作lf电机 */
CAN_Tx_Instance_t m3508_lf_tx_instance = {
        .can_handle = &hcan1,
        .isExTid = 0,
        .tx_mailbox = 0,
        .tx_id = 0x200,// 1~4 的电机为 0x200
        .tx_len = 8,// 一个电机发送控制帧只需占2个数据位
        .can_tx_buff = {0},
};

CAN_Rx_Instance_t m3508_lf_rx_instance = {
        .can_handle = &hcan1,
        .RxHeader = {0},
        .rx_len = 8,
        .rx_id = 0x201,
        .object_para = CanFilter_0|CanFifo_0|Can_STDID|Can_DataType,
        .mask_id = 0,
        .can_rx_buff = {0},
};

Motor_Control_Setting_t m3508_lf_control_instance = {
    .motor_controller_setting = {
        .speed_PID = {
            .Kp = 1.0,
            .Ki = 0.0,
            .Kd = 0.0,
            .MaxOut = 10000,
            .IntegralLimit = 1000,
            .Improve = Trapezoid_Intergral | Integral_Limit | Derivative_On_Measurement
        },
        .pid_ref = 0,
    },
    .outer_loop_type = SPEED_LOOP,
    .inner_loop_type = SPEED_LOOP,
    .motor_is_reverse_flag = MOTOR_DIRECTION_NORMAL,
    .motor_working_status = MOTOR_ENABLED,
};

/* m3508电机作rf电机 */
CAN_Tx_Instance_t m3508_rf_tx_instance = {
        .can_handle = &hcan1,
        .isExTid = 0,
        .tx_mailbox = 0,
        .tx_id = 0x200,// 1~4 的电机为 0x200
        .tx_len = 8,// 一个电机发送控制帧只需占2个数据位
        .can_tx_buff = {0},
};
CAN_Rx_Instance_t m3508_rf_rx_instance = {
        .can_handle = &hcan1,
        .RxHeader = {0},
        .rx_len = 8,
        .rx_id = 0x202,
        .object_para = CanFilter_0|CanFifo_0|Can_STDID|Can_DataType,
        .mask_id = 0,
        .can_rx_buff = {0},
};

Motor_Control_Setting_t m3508_rf_control_instance = {
    .motor_controller_setting = {
        .speed_PID = {
            .Kp = 1.0,
            .Ki = 0.0,
            .Kd = 0.0,
            .MaxOut = 10000,
            .IntegralLimit = 1000,
            .Improve = Trapezoid_Intergral | Integral_Limit | Derivative_On_Measurement
        },
        .pid_ref = 0,
    },
    .outer_loop_type = SPEED_LOOP,
    .inner_loop_type = SPEED_LOOP,
    .motor_is_reverse_flag = MOTOR_DIRECTION_NORMAL,
    .motor_working_status = MOTOR_ENABLED,
};


/* m3508电机作rb电机 */
CAN_Tx_Instance_t m3508_rb_tx_instance = {
        .can_handle = &hcan1,
        .isExTid = 0,
        .tx_mailbox = 0,
        .tx_id = 0x200,// 1~4 的电机为 0x200
        .tx_len = 8,// 一个电机发送控制帧只需占2个数据位
        .can_tx_buff = {0},
};
CAN_Rx_Instance_t m3508_rb_rx_instance = {
        .can_handle = &hcan1,
        .RxHeader = {0},
        .rx_len = 8,
        .rx_id = 0x203,
        .object_para = CanFilter_1|CanFifo_1|Can_STDID|Can_DataType,
        .mask_id = 0,
        .can_rx_buff = {0},
};

Motor_Control_Setting_t m3508_rb_control_instance = {
    .motor_controller_setting = {
        .speed_PID = {
            .Kp = 1.0,
            .Ki = 0.0,
            .Kd = 0.0,
            .MaxOut = 10000,
            .IntegralLimit = 1000,
            .Improve = Trapezoid_Intergral | Integral_Limit | Derivative_On_Measurement
        },
        .pid_ref = 0,
    },
    .outer_loop_type = SPEED_LOOP,
    .inner_loop_type = SPEED_LOOP,
    .motor_is_reverse_flag = MOTOR_DIRECTION_NORMAL,
    .motor_working_status = MOTOR_ENABLED,
};

/* m3508电机作lb电机 */
CAN_Tx_Instance_t m3508_lb_tx_instance = {
        .can_handle = &hcan1,
        .isExTid = 0,
        .tx_mailbox = 0,
        .tx_id = 0x200,// 1~4 的电机为 0x200
        .tx_len = 8,// 一个电机发送控制帧只需占2个数据位
        .can_tx_buff = {0},
};
CAN_Rx_Instance_t m3508_lb_rx_instance = {
        .can_handle = &hcan1,
        .RxHeader = {0},
        .rx_len = 8,
        .rx_id = 0x204,
        .object_para = CanFilter_1|CanFifo_1|Can_STDID|Can_DataType,
        .mask_id = 0,
        .can_rx_buff = {0},
};

Motor_Control_Setting_t m3508_lb_control_instance = {
    .motor_controller_setting = {
        .speed_PID = {
            .Kp = 1.0,
            .Ki = 0.0,
            .Kd = 0.0,
            .MaxOut = 10000,
            .IntegralLimit = 1000,
            .Improve = Trapezoid_Intergral | Integral_Limit | Derivative_On_Measurement
        },
        .pid_ref = 0,
    },
    .outer_loop_type = SPEED_LOOP,
    .inner_loop_type = SPEED_LOOP,
    .motor_is_reverse_flag = MOTOR_DIRECTION_NORMAL,
    .motor_working_status = MOTOR_ENABLED,
};


/* 实例化电机，存进读取速度更快的CCMRAM */

CCMRAM Motor_C620 chassis_motor[4] = {Motor_C620(1,m3508_lf_rx_instance,m3508_lf_tx_instance,m3508_lf_control_instance), 
                                      Motor_C620(2,m3508_rf_rx_instance,m3508_rf_tx_instance,m3508_rf_control_instance),
                                      Motor_C620(3,m3508_rb_rx_instance,m3508_rb_tx_instance,m3508_rb_control_instance), 
                                      Motor_C620(4,m3508_lb_rx_instance,m3508_lb_tx_instance,m3508_lb_control_instance)};

uint8_t Chassis_Init()
{
    /* 创建底盘实例 */
    Omni_Chassis User_Chassis(4,WHEEL_R,CHASSIS_R);
    User_Chassis.Chassis_PID_X = {
        .Kp = 1.0,
        .Ki = 0.0,
        .Kd = 0.0,
    };
    User_Chassis.Chassis_PID_Y = {

    };
    User_Chassis.Chassis_PID_Omega = {

    };

    User_Chassis.Chassis_Subscribe_Init();
    User_Chassis.Get_Current_Position();
    User_Chassis.Get_Current_Posture();
    return 1;
}


uint8_t Chassis_Control()
{

#if USE_FOUR_SWERVE_CHASSIS 

#elif USE_THREE_SWERVE_CHASSIS

#elif USE_FOUR_OMNI_CHASSIS

#elif USE_THREE_OMNI_CHASSIS

#else
    LOGERROR("choose at least one kind of chassis!");

#endif
    return 1;
}



