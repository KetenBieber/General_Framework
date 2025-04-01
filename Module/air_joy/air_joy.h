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
#include "FreeRTOS.h"
#include "task.h"

/* bsp层接口 */
#include "bsp_dwt.h"
#include "bsp_gpio.h"

/* module层接口 */
#include "topics.h"
#include "data_type.h"
#include <math.h>
/*-----------------------------------macro------------------------------------*/
/*----------------------------------typedef-----------------------------------*/

#pragma pack(1)
typedef struct
{
    uint16_t SWA,SWB,SWC,SWD;
    uint16_t LEFT_X,LEFT_Y,RIGHT_X,RIGHT_Y;
}pub_air_joy_data;

#pragma pack()

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

    /* 发布控制信息，发布者注册 */
    Publisher *air_joy_pub;
    pub_air_joy_data air_joy_data;

}Air_Joy_Instance_t;

/*----------------------------------function----------------------------------*/

/**
 * @brief 
 * 
 * @param gpio_instance 
 * @param method abs_diff
 * @return uint8_t 
 */
uint8_t Air_Joy_Init(GPIO_Instance_t *gpio_instance);


/**
 * @brief 
 * 
 * @param instance 
 * @return uint8_t 
 */
void Air_Update(void *instance);

uint8_t Air_Joy_Publish();

/*------------------------------------test------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif	/* AIR_JOY_H */