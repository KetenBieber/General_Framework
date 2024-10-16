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
 *        底盘调外环速度环，比较量是机器人实际速度和期望速度，然后将输出量作为电机的输入量
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
#include "rm_motor.h"
#include "Chassis.h"
#include "Omni_Chassis.h"
#include "motor_interface.h"

/* freertos任务 */
osThreadId_t Chassis_TaskHandle;


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
            .Kp = 8.0,
            .Ki = 0.0,
            .Kd = 0.0,
            .MaxOut = 10000,
            .IntegralLimit = 1000,
            .Output_LPF_RC = 0.2,
            .Derivative_LPF_RC = 0.5,
            .OLS_Order = 1,
            .Improve = OutputFilter|Trapezoid_Intergral | Integral_Limit | Derivative_On_Measurement
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
            .Kp = 8.0,
            .Ki = 0.0,
            .Kd = 0.0,
            .MaxOut = 10000,
            .IntegralLimit = 1000,
            .Output_LPF_RC = 0.2,
            .Derivative_LPF_RC = 0.5,
            .OLS_Order = 1,
            .Improve = OutputFilter|Trapezoid_Intergral | Integral_Limit | Derivative_On_Measurement
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
            .Kp = 8.0,
            .Ki = 0.0,
            .Kd = 0.0,
            .MaxOut = 10000,
            .IntegralLimit = 1000,
            .Output_LPF_RC = 0.2,
            .Derivative_LPF_RC = 0.5,
            .OLS_Order = 1,
            .Improve = OutputFilter|Trapezoid_Intergral | Integral_Limit | Derivative_On_Measurement
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
            .Kp = 8.5,
            .Ki = 0.0,
            .Kd = 0.0,
            .MaxOut = 10000,
            .IntegralLimit = 1000,
            .Output_LPF_RC = 0.2,
            .Derivative_LPF_RC = 0.5,
            .OLS_Order = 1,
            .Improve = OutputFilter|Trapezoid_Intergral | Integral_Limit | Derivative_On_Measurement
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

 /* 创建底盘实例 */
Omni_Chassis User_Chassis(4,WHEEL_R,CHASSIS_R);

#ifdef VOFA_TO_DEBUG
/* vofa调试pid使用 */
Subscriber *motor_pid_sub;
pub_vofa_pid pid_data;
#endif

#ifdef CHASSIS_TO_DEBUG
/* 接收航模遥控控制信息 */
Subscriber *air_data_sub;
pub_Control_Data twist;  
#endif

uint8_t Chassis_Init()
{
    /* 底盘扭矩控制pid */
    User_Chassis.Chassis_PID_X = {
        .Kp = 30.0,
        .Ki = 0.5,
        .Kd = 0.0,
        .MaxOut = 70.0,
        .IntegralLimit = 70.0,
        .Output_LPF_RC = 0.2,
        .Improve = Integral_Limit|OutputFilter,
    };
    User_Chassis.Chassis_PID_Y = {
        .Kp = 30.0,
        .Ki = 0.5,
        .Kd = 0.0,
        .MaxOut = 70.0,
        .IntegralLimit = 70.0,
        .Output_LPF_RC = 0.2,
        .Improve = Integral_Limit|OutputFilter,
    };
    User_Chassis.Chassis_PID_Omega = {
        .Kp = 30.0,
        .Ki = 0.5,
        .Kd = 0.0,
        .MaxOut = 70.0,
        .IntegralLimit = 70.0,
        .Output_LPF_RC = 0.2,
        .Improve = Integral_Limit|OutputFilter,
    };

    User_Chassis.Chassis_Subscribe_Init();

#ifdef CHASSIS_TO_DEBUG
    /* air_joy 订阅者准备 */
    air_data_sub = register_sub("air_joy_pub",1);
#endif

#ifdef VOFA_TO_DEBUG
    /* vofa订阅者准备 */
    motor_pid_sub = register_sub("vofa_pub",1);
#endif
    return 1;
}




__attribute((noreturn)) void Chassis_Task(void *argument)
{
    Chassis_Init();
#ifdef VOFA_TO_DEBUG
    publish_data temp_pid_data;
#endif
#ifdef CHASSIS_TO_DEBUG
    publish_data temp_data;
    float temp = 0;
#endif
    for(;;)
    {
#ifdef VOFA_TO_DEBUG
        /* vofa调试pid使用 */
        temp_pid_data = motor_pid_sub->getdata(motor_pid_sub);
        if(temp_pid_data.len != -1)
        {
            pid_data = *(pub_vofa_pid*)temp_pid_data.data;
            chassis_motor[0].ctrl_motor_config.motor_controller_setting.speed_PID = {
                .Kp = pid_data.Kp,
                .Ki = pid_data.Ki,
                .Kd = pid_data.Kd,
            };
        }
#endif

#ifdef CHASSIS_TO_DEBUG
        /* 接收航模遥控数据 */
        temp_data = air_data_sub->getdata(air_data_sub);
        if(temp_data.len != -1)
        {
            twist = *(pub_Control_Data*)temp_data.data;
            switch (User_Chassis.Chassis_Status)
            {
                case ROBOT_CHASSIS:
                {
                    User_Chassis.Ref_RoboSpeed.linear_x = twist.linear_x;
                    User_Chassis.Ref_RoboSpeed.linear_y = twist.linear_y;
                    User_Chassis.Ref_RoboSpeed.omega = twist.Omega;
                    break;
                }
                case WORLD_CHASSIS:
                {
                    User_Chassis.Ref_WorldSpeed.linear_x = twist.linear_x;
                    User_Chassis.Ref_WorldSpeed.linear_y = twist.linear_y;
                    User_Chassis.Ref_WorldSpeed.omega = twist.Omega;
                    break;
                }
                default:
                    break;
            }
            User_Chassis.Chassis_Status = (Chassis_Status_e)twist.Status;
            Chassis(User_Chassis);       
        }
#endif
        osDelay(2);
    }
}


uint8_t Chassis(Omni_Chassis &user_chassis)
{
   float temp = 0;
   switch(user_chassis.Chassis_Status)
   {
      case CHASSIS_STOP:
         /* 底盘停止 */
         for(size_t i = 0;i < user_chassis.Wheel_Num; i++)
         {
             chassis_motor[i].Out = 0;
         }
         break;
      case ROBOT_CHASSIS:
         /* 机器人坐标系下控制底盘 */

         // 正解算得到底盘速度
         user_chassis.Kinematics_forward_Resolution(chassis_motor[0].speed_aps,chassis_motor[1].speed_aps,chassis_motor[2].speed_aps,chassis_motor[3].speed_aps);      
         /* 外环速度环 */
         user_chassis.Dynamics_Inverse_Resolution();
         for(size_t i = 0;i < user_chassis.Wheel_Num; i++)
         {
            temp = user_chassis.Kinematics_Inverse_Resolution(i,user_chassis.ref_twist);
            chassis_motor[i].Out = chassis_motor[i].aps_to_current(temp);
         }
         break;
      case WORLD_CHASSIS:
         /* 世界坐标系下控制底盘 */
         /* 获取ins系统的机器人姿态数据 */
         user_chassis.Get_Current_Posture();
         // 正解算得到底盘速度
         user_chassis.Kinematics_forward_Resolution(chassis_motor[0].speed_aps,chassis_motor[1].speed_aps,chassis_motor[2].speed_aps,chassis_motor[3].speed_aps);      
         /* 获得当前世界坐标系下速度 */
         user_chassis.RoboSpeed_To_WorldSpeed();
         /* 外环速度环 */
         user_chassis.Dynamics_Inverse_Resolution();
         /* 电机输出赋值 */
         for(size_t i = 0;i < user_chassis.Wheel_Num; i++)
         {
            temp = user_chassis.Kinematics_Inverse_Resolution(i,user_chassis.ref_twist);
            chassis_motor[i].Out = chassis_motor[i].aps_to_current(temp);
         }
         break;
    }
    Motor_SendMsgs(chassis_motor); // 发送电机can帧
    return 1;
}  









