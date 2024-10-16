/**
 * @file air_joy.c
 * @author Keten (2863861004@qq.com)
 * @brief 航模遥控器数据处理，发布控制信息在话题"air_joy_pub"中，创建订阅者，订阅该话题即可获取控制信息
 * @version 0.1
 * @date 2024-10-14
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention :
 * @note :
 * @versioninfo :
 */
#include "air_joy.h"
#include "bsp_gpio.h"


static void update_trapezoidal_state(TrapezoidalState *state, float target_velocity);


// 留一个指针值，不管最终有无创建，也只占4字节的指针内存
Air_Joy_Instance_t *air_instance;


uint8_t Air_Joy_Init(GPIO_Instance_t *gpio_instance,Process_method_e method)
{
    if(gpio_instance == NULL)
    {
        LOGERROR("gpio_instance is not prepared!");
        return NULL;
    }
    air_instance = (Air_Joy_Instance_t*)pvPortMalloc(sizeof(Air_Joy_Instance_t));
    if(air_instance == NULL)
    {
        LOGERROR("create air_joy instance failed!");
        return NULL;
    }
    memset(air_instance,0,sizeof(Air_Joy_Instance_t));
    /* 设置控制模式 */
    air_instance->process_method = method;
    /* 进行gpio中断的设置 */
    air_instance->air_joy_gpio = gpio_instance;
    gpio_instance->exit_callback = Air_Update;
    gpio_instance->mode = GPIO_EXTI_MODE_RISING_FALLING;

    /* 用于更新的数据进行清零 */
    air_instance->SWA = 0;
    air_instance->SWB = 0;
    air_instance->SWC = 0;
    air_instance->SWD = 0;
    air_instance->LEFT_X = 0;
    air_instance->LEFT_Y = 0;
    air_instance->RIGHT_X = 0;
    air_instance->RIGHT_Y = 0;
    air_instance->last_ppm_time = 0;
    air_instance->now_ppm_time = 0;
    air_instance->ppm_ready = 0;
    air_instance->ppm_sample_cnt = 0;
    air_instance->ppm_update_flag = 0;
    air_instance->ppm_time_delta = 0;
    memset(air_instance->PPM_Buf,0,sizeof(air_instance->PPM_Buf));

    /* 注册发布者 */
    air_instance->air_joy_pub = register_pub("air_joy_pub");
}


uint8_t Air_Update(void *instance)
{
    /* 获取当前时间 */
    air_instance->last_ppm_time = air_instance->now_ppm_time;
    air_instance->now_ppm_time = DWT_GetTimeline_us();
    air_instance->ppm_time_delta = air_instance->now_ppm_time - air_instance->last_ppm_time;

    if(air_instance->ppm_ready == 1)
    {
        if(air_instance->ppm_time_delta >= 2100) // 帧头
        {
            air_instance->ppm_ready = 1;
            air_instance->ppm_sample_cnt = 0; // 对应的通道值
            air_instance->ppm_update_flag = 1;
        }
        else if(air_instance->ppm_time_delta >= 950 && air_instance->ppm_time_delta <= 2050)//单个PWM脉宽在1000-2000us，这里设定950-2050，提升容错
        {
            air_instance->PPM_Buf[air_instance->ppm_sample_cnt++] = air_instance->ppm_time_delta;// 对应通道写入缓冲区
            if(air_instance->ppm_sample_cnt >= 8) // 单次解析结束0-7表示8个通道。如果想要使用10通道，使用ibus协议(串口接收)
            {
                air_instance->LEFT_X = air_instance->PPM_Buf[0];
                air_instance->LEFT_Y = air_instance->PPM_Buf[1];
                air_instance->RIGHT_X = air_instance->PPM_Buf[3];
                air_instance->RIGHT_Y = air_instance->PPM_Buf[2];
                air_instance->SWA = air_instance->PPM_Buf[4];
                air_instance->SWB = air_instance->PPM_Buf[5]; 
                air_instance->SWC = air_instance->PPM_Buf[6]; 
                air_instance->SWD = air_instance->PPM_Buf[7];
            }
        }
        else
        {
            air_instance->ppm_ready = 0;
        }
    }
    else if(air_instance->ppm_time_delta >= 2100)//帧尾电平至少2ms=2000us
    {
        air_instance->ppm_ready = 1;
        air_instance->ppm_sample_cnt = 0;
        air_instance->ppm_update_flag = 0;
    }
}


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

uint8_t Air_Joy_Process()
{
    /* 在最前面提供拨杆处理，优先处理拨杆再处理摇杆 */
    if(air_instance->LEFT_X!=0||air_instance->LEFT_Y!=0||air_instance->RIGHT_X!=0||air_instance->RIGHT_Y!=0)            
    {
        if(air_instance->SWB > 950 && air_instance->SWB < 1050)
        {
            // 三档拨杆，最上状态
            air_instance->control_data.Status = 0;
        }
        if(air_instance->SWB > 1450 && air_instance->SWB < 1550)
        {
            // 三档拨杆，中间状态
            air_instance->control_data.Status = 1;
        }
        if(air_instance->SWB > 1950 && air_instance->SWB < 2050)
        {
            // 三档拨杆，最下状态
            air_instance->control_data.Status = 2;
        }
    }
    if(air_instance->LEFT_X > 1400 && air_instance->LEFT_X < 1600)
        air_instance->LEFT_X = 1500;
    if(air_instance->LEFT_Y > 1400 && air_instance->LEFT_Y < 1600)  
        air_instance->LEFT_Y = 1500;
    if(air_instance->RIGHT_X > 1400 && air_instance->RIGHT_X < 1600)
        air_instance->RIGHT_X = 1500;
    if(air_instance->RIGHT_Y > 1400 && air_instance->RIGHT_Y < 1600)
        air_instance->RIGHT_Y = 1500;
    switch(air_instance->process_method)
    {
        case NORMAL:
            air_instance->control_data.linear_x = (air_instance->LEFT_Y - 1500) / 500.0f * 3.0f;
            air_instance->control_data.linear_y = (air_instance->LEFT_X - 1500) / 500.0f * 3.0f;
            air_instance->control_data.Omega = (air_instance->RIGHT_X - 1500) / 500.0f * 4.0f;
            Air_Joy_Publish();
            break;
        case TRAPEZOIDAL:
            /* 初始化梯形规划状态量 */
            static TrapezoidalState left_y_state = {0.0f, 0.0f};
            static TrapezoidalState left_x_state = {0.0f, 0.0f};
            static TrapezoidalState right_x_state = {0.0f, 0.0f};
            float target_linear_x = (air_instance->LEFT_Y - 1500) / 500.0f * MAX_VELOCITY;
            float target_linear_y = (air_instance->LEFT_X - 1500) / 500.0f * MAX_VELOCITY;
            float target_omega = (air_instance->RIGHT_X - 1500) / 500.0f * MAX_VELOCITY;
            /* 更新梯形规划输出值 */
            update_trapezoidal_state(&left_x_state, target_linear_x);
            update_trapezoidal_state(&left_y_state, target_linear_y);
            update_trapezoidal_state(&right_x_state, target_omega);

            air_instance->control_data.linear_x = left_x_state.current_velocity;
            air_instance->control_data.linear_y = left_y_state.current_velocity;
            air_instance->control_data.Omega = right_x_state.current_velocity;
            Air_Joy_Publish();
            break;
    }
}


uint8_t Air_Joy_Publish()
{
    publish_data temp_data;
    temp_data.data = (uint8_t*)&air_instance->control_data;
    temp_data.len = sizeof(pub_Control_Data);
    air_instance->air_joy_pub->publish(air_instance->air_joy_pub,temp_data);
    return 1;
}



