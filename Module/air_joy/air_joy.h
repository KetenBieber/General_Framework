/**
 * @file air_joy.h
 * @author Keten (2863861004@qq.com)
 * @brief 
 * @version 0.1
 * @date 2024-10-14
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention :
 * @note :
 * @versioninfo :
 */
#ifndef AIR_JOY_H 
#define AIR_JOY_H 

#ifdef __cplusplus
extern "C"{
#endif

/*----------------------------------include-----------------------------------*/
#include "bsp_dwt.h"
#include "bsp_gpio.h"
#include "topics.h"
#include "data_type.h"
#include <math.h>
/*-----------------------------------macro------------------------------------*/
#define MAX_ACCELERATION 0.5f
#define MAX_VELOCITY 3.0f

/*----------------------------------typedef-----------------------------------*/

typedef enum
{
    NORMAL = 0,// 遥杆数据直驱
    TRAPEZOIDAL,// 遥杆数据梯形规划处理
}Process_method_e;

typedef struct 
{
    float current_velocity;
    float target_velocity;
}TrapezoidalState;

typedef struct
{
    GPIO_Instance_t *air_joy_gpio;
    uint16_t SWA,SWB,SWC,SWD;
    uint16_t LEFT_X,LEFT_Y,RIGHT_X,RIGHT_Y;

    uint32_t last_ppm_time,now_ppm_time;
    uint16_t PPM_Buf[10];
    uint8_t ppm_ready;
    uint8_t ppm_sample_cnt;
    uint8_t ppm_update_flag;
    uint16_t ppm_time_delta;
    /* 控制模式 */
    Process_method_e process_method;

    /* 发布控制信息，发布者注册 */
    Publisher *air_joy_pub;
    pub_Control_Data control_data;
}Air_Joy_Instance_t;

/*----------------------------------variable----------------------------------*/
extern Air_Joy_Instance_t *air_instance;


/*----------------------------------function----------------------------------*/

/**
 * @brief 
 * 
 * @param gpio_instance 
 * @param method abs_diff
 * @return uint8_t 
 */
uint8_t Air_Joy_Init(GPIO_Instance_t *gpio_instance,Process_method_e method);


/**
 * @brief 
 * 
 * @param instance 
 * @return uint8_t 
 */
uint8_t Air_Update(void *instance);


/**
 * @brief 
 * 
 * @return uint8_t 
 */
uint8_t Air_Joy_Process();


uint8_t Air_Joy_Publish();

/*------------------------------------test------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif	/* AIR_JOY_H */
