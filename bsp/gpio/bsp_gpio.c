/**
 * @file bsp_gpio.c
 * @author Keten (2863861004@qq.com)
 * @brief 
 * @version 0.1
 * @date 2024-09-26
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention :
 * @note :
 * @versioninfo :
 */
#include "bsp_gpio.h"

static uint8_t idx = 0;
static GPIO_Instance_t *gpio_device[GPIO_MX_DEVICE_NUM] = {NULL};

static uint8_t Hal_Read_io(GPIO_TypeDef* goiox, uint16_t pin);
static uint8_t Hal_Write_io(GPIO_TypeDef* goiox, uint16_t pin, GPIO_PinState state);
static uint8_t Hal_Toogle_io(GPIO_TypeDef* goiox, uint16_t pin);


GPIO_Instance_t* GPIO_Pin_Register(GPIO_TypeDef* gpiox, uint16_t gpio_pin, GPIO_PinState state, exit_mode_e mode, exit_callback_fun exit_callback)
{
    GPIO_Instance_t *new_gpio_device = (GPIO_Instance_t*)malloc(sizeof(GPIO_Instance_t));
    if(new_gpio_device == NULL)
    {
        LOGERROR("create gpio instance failed!");
        return NULL;
    }
    memset(new_gpio_device,0,sizeof(GPIO_Instance_t));// 初始化gpio设备实例
    /* 挂载用户传入io口参数 */
    new_gpio_device->GPIOx = gpiox;
    new_gpio_device->gpio_pin = gpio_pin;
    new_gpio_device->pin_state =  state;
    new_gpio_device->mode = mode;
    new_gpio_device->exit_callback = exit_callback;

    new_gpio_device->read_io = Hal_Read_io;
    new_gpio_device->write_io = Hal_Write_io;
    new_gpio_device->toogle_io = Hal_Toogle_io;

    gpio_device[idx++] = new_gpio_device;
    LOGINFO("new gpio device create successfully!");
    return new_gpio_device;
}


uint8_t Exit_GPIO_Handler(uint16_t gpio_pin,GPIO_Instance_t* gpio_device)
{
    if(gpio_device == NULL)
    {
        LOGERROR("gpio exit failed!");
        return 0;
    }
    /* 先清除标志位 */
    __HAL_GPIO_EXTI_CLEAR_FLAG(gpio_pin);
    if(gpio_device->exit_callback != NULL && gpio_pin == gpio_device->gpio_pin)
    {
        /* 如果传入io口句柄被设置为外部中断模式，则调用对应回调函数 */
        gpio_device->exit_callback(gpio_device);
        return 1;
    }
    return 0;
}


/**
 * @brief 封装hal库的 读取io口电平信号
 * 
 * @param goiox 
 * @param pin 
 * @return uint8_t 
 */
static uint8_t Hal_Read_io(GPIO_TypeDef* goiox, uint16_t pin)
{
    HAL_GPIO_ReadPin(goiox, pin);
    return 1;
}


/**
 * @brief 封装hal库的 向io口写入信号
 * 
 * @param goiox 
 * @param pin 
 * @param state 
 * @return uint8_t 
 */
static uint8_t Hal_Write_io(GPIO_TypeDef* goiox, uint16_t pin, GPIO_PinState state)
{
    HAL_GPIO_WritePin(goiox, pin, state);
    return 1;
}


/**
 * @brief 封装hal库的 翻转io口信号
 * 
 * @param goiox 
 * @param pin 
 * @return uint8_t 
 */
static uint8_t Hal_Toogle_io(GPIO_TypeDef* goiox, uint16_t pin)
{
    HAL_GPIO_TogglePin(goiox, pin);
    return 1;
}

