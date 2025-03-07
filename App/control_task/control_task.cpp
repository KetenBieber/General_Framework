/**
 * @file control_task.cpp
 * @author Keten (2863861004@qq.com)
 * @brief 
 * @version 0.1
 * @date 2024-10-04
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention :
 * @note :  2025-1-22 对之前控制数据和手柄数据任务进行解耦，在这里订阅原始的手柄数据，进行解析后，发布话题出去到所需的app层
 *                    对航模遥控手柄的数据订阅话题：air_joy_pub
 *                    对xbox手柄的数据订阅话题 ：xbox_joy_pub
 * 
 *                     发布控制信息的唯一接口：ctrl_pub
 * @todo : 测试完之后，将所有全局变量的订阅者发布者移植任务函数中，使其成为局部变量
 * @versioninfo :
 */
#include "control_task.h"


osThreadId_t Control_TaskHandle;

/* 控制信息发布 */
Publisher *ctrl_pub;
pub_Control_Data ctrl_data;

/* 订阅遥控信息 */
Subscriber *air_joy_sub;
pub_air_joy_data air_joy_data;

/* 是否使用梯形曲线处理遥杆数据 */
uint8_t if_use_trapezoidal = 0;

static void update_trapezoidal_state(TrapezoidalState *state, float target_velocity) 
{
    state->target_velocity = target_velocity;
    if (fabs(state->current_velocity - state->target_velocity) < MAX_ACCELERATION) {
        state->current_velocity = state->target_velocity;
    } else if (state->current_velocity < state->target_velocity) {
        state->current_velocity += MAX_ACCELERATION;
    } else {
        state->current_velocity -= MAX_ACCELERATION;
    }
}


void Air_Joy_Process()
{
    /* 在最前面提供拨杆处理，优先处理拨杆再处理摇杆 */
    if(air_joy_data.SWA > 950 && air_joy_data.SWA < 1050)
        ctrl_data.if_rev_ros = 0;// 两档拨杆，最上状态
    if(air_joy_data.SWA > 1950 && air_joy_data.SWA < 2050)
        ctrl_data.if_rev_ros = 1;// 两档拨杆，最下状态
    if(air_joy_data.SWB > 950 && air_joy_data.SWB < 1050)
        ctrl_data.Status = 0;// 三档拨杆，最上状态
    if(air_joy_data.SWB > 1450 && air_joy_data.SWB < 1550)
        ctrl_data.Status = 1;// 三档拨杆，中间状态
    if(air_joy_data.SWB > 1950 && air_joy_data.SWB < 2050)
        ctrl_data.Status = 2;// 三档拨杆，最下状态
    if(air_joy_data.SWC > 950 && air_joy_data.SWC < 1050)
        ctrl_data.Move = 0;// 三档拨杆，最上状态
    if(air_joy_data.SWC > 1450 && air_joy_data.SWC < 1550)
        ctrl_data.Move = 1;// 三档拨杆，中间状态
    if(air_joy_data.SWC > 1950 && air_joy_data.SWC < 2050)
        ctrl_data.Move = 2;// 三档拨杆，最下状态
    if(air_joy_data.SWD > 950 && air_joy_data.SWD < 1050)
        ctrl_data.ctrl = 0;// 两档拨杆，最上状态
    if(air_joy_data.SWD > 1950 && air_joy_data.SWD < 2050)
        ctrl_data.ctrl = 1;// 两档拨杆，最下状态

    /* 遥杆数据浅滤一下 */
    if(air_joy_data.LEFT_X > 1400 && air_joy_data.LEFT_X < 1600)
        air_joy_data.LEFT_X = 1500;
    if(air_joy_data.LEFT_Y > 1400 && air_joy_data.LEFT_Y < 1600)  
        air_joy_data.LEFT_Y = 1500;
    if(air_joy_data.RIGHT_X > 1400 && air_joy_data.RIGHT_X < 1600)
        air_joy_data.RIGHT_X = 1500;
    if(air_joy_data.RIGHT_Y > 1400 && air_joy_data.RIGHT_Y < 1600)
        air_joy_data.RIGHT_Y = 1500;

    switch(if_use_trapezoidal)
    {
        case NORMAL:
            ctrl_data.linear_x = (air_joy_data.LEFT_Y - 1500) / 500.0f * MAX_VELOCITY;
            ctrl_data.linear_y = -(air_joy_data.LEFT_X - 1500) / 500.0f * MAX_VELOCITY;
            ctrl_data.Omega = (air_joy_data.RIGHT_X - 1500) / 500.0f * MAX_VELOCITY;
            Air_Joy_Publish();
            break;
        case TRAPEZOIDAL:
            /* 初始化梯形规划状态量 */
            static TrapezoidalState left_y_state = {0.0f, 0.0f};
            static TrapezoidalState left_x_state = {0.0f, 0.0f};
            static TrapezoidalState right_x_state = {0.0f, 0.0f};
            float target_linear_x = (air_joy_data.LEFT_Y - 1500) / 500.0f * MAX_VELOCITY;
            float target_linear_y = (air_joy_data.LEFT_X - 1500) / 500.0f * MAX_VELOCITY;
            float target_omega = (air_joy_data.RIGHT_X - 1500) / 500.0f * MAX_VELOCITY;
            /* 更新梯形规划输出值 */
            update_trapezoidal_state(&left_x_state, target_linear_x);
            update_trapezoidal_state(&left_y_state, target_linear_y);
            update_trapezoidal_state(&right_x_state, target_omega);

            ctrl_data.linear_x = left_x_state.current_velocity;
            ctrl_data.linear_y = left_y_state.current_velocity;
            ctrl_data.Omega = right_x_state.current_velocity;
            Air_Joy_Publish();
            break;
    }
}





__attribute((noreturn)) void Control_Task(void *argument)
{
    /* 机器人控制接口，这里选用航模遥控 */
    /* 航模遥控 */
    GPIO_Instance_t *gpio_instance = GPIO_Pin_Register(GPIOA, GPIO_PIN_1);
    if(gpio_instance == NULL)
    {
        LOGERROR("gpio instance create failed!");
        vTaskDelete(NULL);
    }
    Air_Joy_Init(gpio_instance);
    if_use_trapezoidal = NORMAL;

    /* 接收遥控数据订阅者初始化 */
    air_joy_sub = register_sub("air_joy_pub",1);
    publish_data temp_air_data;

    /* 发布控制数据 */
    ctrl_pub = register_pub("ctrl_pub");
    publish_data temp_ctrl_data;
    for(;;)
    {
        temp_air_data = air_joy_sub->getdata(air_joy_sub);
        if(temp_air_data.len != -1)
        {
            air_joy_data = *(pub_air_joy_data*)temp_air_data.data;
            Air_Joy_Process();
            temp_ctrl_data.data = (uint8_t*)&ctrl_data;
            temp_ctrl_data.len = sizeof(pub_Control_Data);
            ctrl_pub->publish(ctrl_pub,temp_ctrl_data);
        }
        osDelay(2);
    }
}


