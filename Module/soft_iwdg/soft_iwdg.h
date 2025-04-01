/**
 * @file soft_iwdg.h
 * @author Keten (2863861004@qq.com)
 * @brief 
 * @version 0.1
 * @date 2024-09-29
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention :
 * @note :
 * @versioninfo :
 */
#ifndef SOFT_IWDG_H 
#define SOFT_IWDG_H 

#ifdef __cplusplus
extern "C"{
#endif


/*----------------------------------include-----------------------------------*/
/* rtos层接口 */
#include "FreeRTOS.h"
#include "task.h"
/* bsp层接口 */
#include "bsp_dwt.h"
#include "bsp_log.h"
/*-----------------------------------macro------------------------------------*/

#define IWDG_CNT    128      // 限制被注册的看门狗数量
/*----------------------------------typedef-----------------------------------*/
/* 向前声明 */
typedef struct IWDG_Instance_t IWDG_Instance_t;

/* 看门狗饥饿咬人函数 */
typedef uint8_t (*if_dog_offline_callback)(void* owner_id);
/* 时间接口 */
typedef struct 
{
#ifdef USE_RTOS
    void (*iwdg_sleep)(const TickType_t xTicksToSleep);// 使用多任务操作系统,在初始化的时候必须要阻塞,将cpu资源释放
#else
    void (*iwdg_sleep)(float s);
#endif
}time_interface_for_iwdg_t;

typedef struct
{
    uint16_t reload_count;     // 看门狗重载值
    uint16_t init_count;       // 一些需要睡一觉再起来的看门狗的睡眠时间
    if_dog_offline_callback callback;     // 看门狗异常处理函数
}iwdg_config_t;

/* 看门狗监控线程实例 */
struct IWDG_Instance_t
{
    time_interface_for_iwdg_t* sleep_func_interface;// 睡眠函数接口(delay函数接口)
    iwdg_config_t* dog_config;// 看门狗配置
    uint16_t temp_count;       // 看门狗当前值  
    uint8_t is_sleep;           // 看门狗睡觉标志位 置0 为在睡觉; 置1 为唤醒
    uint8_t (*fall_asleep)(IWDG_Instance_t* dog_instance);// 看门狗睡觉的函数
    uint8_t (*feed_dog) (IWDG_Instance_t* dog_instance);// 喂看门狗
    uint8_t (*is_dog_online)(IWDG_Instance_t* dog_instance);// 查看狗是不是还在
    uint8_t (*iwdg_Deinit)(void*);// 看门狗注销函数
    void *owner_id; // 看门狗的拥有者id
};
/*----------------------------------function----------------------------------*/

/**
 * @brief 看门狗监护实例注册函数
 * 
 * @param iwdg_config 
 * @return IWDG_Instance_t* 
 */
IWDG_Instance_t* IWDG_Register(iwdg_config_t* iwdg_config);


/**
 * @brief 看门狗工作前睡眠函数
 * 
 * @param dog_instance 
 * @return uint8_t 
 */
uint8_t IWDG_Fall_Asleep(IWDG_Instance_t *dog_instance);


/**
 * @brief 
 * 
 * @param dog_instance 
 * @return uint8_t 
 */
uint8_t IWDG_Feed_Dog(IWDG_Instance_t* dog_instance);


/**
 * @brief 
 * 
 * @param dog_instance 
 * @return uint8_t 
 */
uint8_t IWDG_Is_Dog_Online(IWDG_Instance_t* dog_instance);


/**
 * @brief 监护线程函数,必须注册一个看门狗监护线程,在线程中循环调用这个函数
 * 
 */
void IWDG_Task(void);


/**
 * @brief 
 * 
 * @param dog_instance 
 * @return uint8_t 
 */
uint8_t IWDG_UnRegister(void *dog_instance);  

/*------------------------------------test------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif	/* SOFT_IWDG_H */
