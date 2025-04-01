/**
 * @file air_joy.c
 * @author Keten (2863861004@qq.com)
 * @brief 航模遥控器数据处理，发布控制信息在话题"air_joy_pub"中，创建订阅者，订阅该话题即可获取控制信息
 * @version 0.1
 * @date 2024-10-14
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention : 2025-1-21 发现当前航模遥控和底盘任务耦合太大了，不合适，需要解耦，直接把收到的航模遥控
 *                        数据发送出去，由app层的人自行编写处理航模数据的函数
 * @note :
 * @versioninfo :
 */
#include "air_joy.h"



// 留一个指针值，不管最终有无创建，也只占4字节的指针内存
Air_Joy_Instance_t *air_instance;


uint8_t Air_Joy_Init(GPIO_Instance_t *gpio_instance)
{
    if(gpio_instance == NULL)
    {
        LOGERROR("AIR_JOY gpio_instance is not prepared!");
        return 0;
    }
    air_instance = (Air_Joy_Instance_t*)pvPortMalloc(sizeof(Air_Joy_Instance_t));
    if(air_instance == NULL)
    {
        LOGERROR("create air_joy instance failed!");
        return 0;
    }
    memset(air_instance,0,sizeof(Air_Joy_Instance_t));
    /* 进行gpio中断的设置 */
    air_instance->air_joy_gpio = gpio_instance;
    gpio_instance->exit_callback = Air_Update;
    gpio_instance->mode = GPIO_EXTI_MODE_RISING_FALLING;

    /* 用于更新的数据进行清零 */
    air_instance->SWA = 1000;
    air_instance->SWB = 1000;
    air_instance->SWC = 1000;
    air_instance->SWD = 1000;
    air_instance->LEFT_X = 1500;
    air_instance->LEFT_Y = 1500;
    air_instance->RIGHT_X = 1500;
    air_instance->RIGHT_Y = 1500;
    air_instance->last_ppm_time = 0;
    air_instance->now_ppm_time = 0;
    air_instance->ppm_ready = 0;
    air_instance->ppm_sample_cnt = 0;
    air_instance->ppm_update_flag = 0;
    air_instance->ppm_time_delta = 0;
    memset(air_instance->PPM_Buf,0,sizeof(air_instance->PPM_Buf));

    /* 注册发布者 */
    air_instance->air_joy_pub = register_pub("air_joy_pub");

    air_instance->air_joy_data.LEFT_X = 0;air_instance->air_joy_data.LEFT_Y = 0;air_instance->air_joy_data.RIGHT_X = 0;air_instance->air_joy_data.RIGHT_Y = 0;
    air_instance->air_joy_data.SWA = 0;air_instance->air_joy_data.SWB = 0;air_instance->air_joy_data.SWC = 0;air_instance->air_joy_data.SWD = 0;

    return 1;
}


void Air_Update(void *instance)
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

    /* 发布控制信息 */
    Air_Joy_Publish();
}   


uint8_t Air_Joy_Publish()
{
    publish_data temp_data;
    temp_data.data = (uint8_t*)&air_instance->air_joy_data;
    temp_data.len = sizeof(pub_air_joy_data);
    air_instance->air_joy_pub->publish(air_instance->air_joy_pub,temp_data);
    /* 写一个 */
    return 1;
}


