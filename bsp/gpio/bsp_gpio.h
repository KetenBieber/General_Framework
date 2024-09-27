/**
 * @file bsp_gpio.h
 * @author Keten (2863861004@qq.com)
 * @brief 
 * @version 0.1
 * @date 2024-09-26
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention : gpio设计需要什么功能  gpio口的基本功能以及外部中断的注册功能接口，这些都需要做好封装
 *              如果是需要快速响应的io口，建议使用位带操作！
 *              （位带操作本来就是为了省事，不在这里进行封装了！
 * @note :
 * @versioninfo :
 */
#ifndef BSP_GPIO_H 
#define BSP_GPIO_H 

#ifdef __cplusplus
extern "C"{
#endif

/*----------------------------------include-----------------------------------*/
#include "gpio.h"
#include <string.h>
#include "bsp_log.h"

/*-----------------------------------macro------------------------------------*/
#define GPIO_MX_DEVICE_NUM 10
/*----------------------------------typedef-----------------------------------*/
/* 前向声明 */
typedef struct GPIO_Instance_t GPIO_Instance_t;

typedef enum
{
    GPIO_WITHOUT_EXIT,// 无中断配置,普通io口
    GPIO_EXTI_MODE_RISING,
    GPIO_EXTI_MODE_FALLING,
    GPIO_EXTI_MODE_RISING_FALLING,
    GPIO_EXTI_MODE_NONE,
}exit_mode_e;

typedef uint8_t (*exit_callback_fun) (GPIO_Instance_t *gpio_instance);

struct GPIO_Instance_t
{
    GPIO_TypeDef *GPIOx;        // GPIOA,GPIOB,GPIOC...
    uint16_t gpio_pin;          // io口序号
    GPIO_PinState pin_state;    // io口状态
    exit_mode_e mode;           // 中断模式
    uint8_t (*read_io)(GPIO_TypeDef* goiox, uint16_t pin);// 读io口状态
    uint8_t (*write_io)(GPIO_TypeDef* goiox, uint16_t pin, GPIO_PinState state);// 写io口状态
    uint8_t (*toogle_io)(GPIO_TypeDef* goiox, uint16_t pin);// 翻转io口状态
    exit_callback_fun exit_callback;// 中断回调函数接口，需要用户自己实现
};

/*----------------------------------function----------------------------------*/
/**
 * @brief gpio口实例 注册函数
 * 
 * @param gpiox         io 口总线 GPIOA、GPIOB、GPIOC...
 * @param gpio_pin      io 口序号
 * @param state         io 口电平状态
 * @param mode          io 口中断模式
 * @param exit_callback io 口外部中断函数指针
 * @return GPIO_Instance_t* 
 */
GPIO_Instance_t* GPIO_Pin_Register(GPIO_TypeDef* gpiox, uint16_t gpio_pin, GPIO_PinState state, exit_mode_e mode, exit_callback_fun exit_callback);


/**
 * @brief io口中断管理函数，放在it.c中
 * 
 * @param gpio_pin 
 * @param gpio_device 
 * @return uint8_t 
 */
uint8_t Exit_GPIO_Handler(uint16_t gpio_pin,GPIO_Instance_t* gpio_device);



#ifdef __cplusplus
}
#endif

#endif	/* BSP_GPIO_H */
