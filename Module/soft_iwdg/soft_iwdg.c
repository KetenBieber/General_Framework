/**
 * @file soft_iwdg.c
 * @author Keten (2863861004@qq.com)
 * @brief 软件看门狗,可以用于监测任务是否正常运行
 * @version 0.1
 * @date 2024-09-29
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention :
 * @note :
 * @versioninfo :
 */
#include "soft_iwdg.h"

static time_interface_for_iwdg_t dog_time_interface = {
#ifdef USE_RTOS    
    .iwdg_sleep = vTaskDelay// 挂载上rtos中的delay接口
#else  
    .iwdg_sleep = DWT_Delay
#endif    
};
static uint8_t Time_Interface_Init(IWDG_Instance_t *dog_instance);

/* IWDG实例数组(狗圈),所有注册了IWDG的模块信息都会被保存在这里 */
static uint8_t idx_iwdg = 0;
static IWDG_Instance_t *IWDG_Device[IWDG_CNT] = {NULL};
static uint8_t IWDG_Deinit(IWDG_Instance_t **dog_instance);


IWDG_Instance_t* IWDG_Register(iwdg_config_t* iwdg_config)
{
    if(iwdg_config == NULL)
    {
        LOGERROR("iwdg_config is not valid!");
        return NULL;
    }
    if(idx_iwdg>=IWDG_CNT)
    {
        LOGERROR("iwdg exceed max instance count!");
        return NULL;
    }
    IWDG_Instance_t *temp_dog_instance = (IWDG_Instance_t*)pvPortMalloc(sizeof(IWDG_Instance_t));
    if(temp_dog_instance == NULL)
    {
        LOGERROR("dog_instance pvPortMalloc failed!");
        return NULL;
    }
    if(Time_Interface_Init(temp_dog_instance) != 1)
    {
        /* 创建失败,清除内存,返回NULL */
        free(temp_dog_instance);
        temp_dog_instance = NULL;
        return NULL;
    }
    /* 挂载iwdg_config参数 */
    temp_dog_instance->dog_config = iwdg_config;
    temp_dog_instance->is_sleep = 0;// 设置为睡觉状态
    temp_dog_instance->fall_asleep = IWDG_Fall_Asleep;// 挂载睡觉函数
    temp_dog_instance->feed_dog = IWDG_Feed_Dog;// 挂载喂狗函数
    temp_dog_instance->is_dog_online = IWDG_Is_Dog_Online;// 挂载查看狗是否在线函数
    temp_dog_instance->iwdg_Deinit = IWDG_UnRegister;// 挂载注销函数

    IWDG_Device[idx_iwdg++] = temp_dog_instance;
    LOGINFO("IWDG_Instance create successfully!");
    return temp_dog_instance;

}



static uint8_t Time_Interface_Init(IWDG_Instance_t *dog_instance)
{
    if(dog_instance == NULL)
    {
        LOGERROR("dog_instance get time_interface failed!");
        return 0;
    }
    dog_instance->sleep_func_interface = &dog_time_interface;
    LOGINFO("dog_instance get time_interface success!");
    return 1;
}

uint8_t IWDG_Fall_Asleep(IWDG_Instance_t *dog_instance)
{
    dog_instance->is_sleep = 0;// 将看门狗设置为睡眠状态
    if(dog_instance == NULL)
    {
        return 0;
    }
    /* 调用看门狗初始化睡眠函数,将初始化所需要的时间传入 */
    dog_instance->sleep_func_interface->iwdg_sleep(dog_instance->dog_config->init_count);
    dog_instance->is_sleep = 1;// 将看门狗设置为唤醒状态
    /* 这个时候,cpu资源会被让出 */
    return 1;
}



uint8_t IWDG_Feed_Dog(IWDG_Instance_t* dog_instance)
{
    /* 重装载当前值 */
    dog_instance->temp_count = dog_instance->dog_config->reload_count;
    return 1;
}



uint8_t IWDG_Is_Dog_Online(IWDG_Instance_t* dog_instance)
{
    return dog_instance->temp_count > 0;
}


void IWDG_Task(void)
{
    IWDG_Instance_t *temp_dog_instance;
    for(uint8_t i = 0; i<idx_iwdg; i++)
    {
        temp_dog_instance = IWDG_Device[i];
        if(temp_dog_instance->temp_count >0)
            temp_dog_instance->temp_count--;
        else if(temp_dog_instance->dog_config->callback && (temp_dog_instance->is_sleep == 1))// 调用回调函数,如果有注册回调函数并且此时任务并非睡眠状态
        {
            temp_dog_instance->dog_config->callback(temp_dog_instance->owner_id);
        }
    }
}


uint8_t IWDG_UnRegister(void *dog_instance)  
{
    if(dog_instance == NULL)
    {
        LOGERROR("dog_instance is not valid!");
        return 0;
    }
    IWDG_Instance_t *temp_dog_instance = (IWDG_Instance_t*)dog_instance;
    for(int i = 0; i < idx_iwdg; i++)
    {
        if(temp_dog_instance == IWDG_Device[i])
        {
            /* 调用 Deinit 函数来清除实例和父类实例 */
            IWDG_Deinit(&temp_dog_instance);
            /* 从数组中移除指定项 */
            for(int j = i; j < idx_iwdg - 1; j++)
            {
                IWDG_Device[j] = IWDG_Device[j + 1];
            }
            IWDG_Device[idx_iwdg - 1] = NULL;
            idx_iwdg--;

            return 1;
        }
    }

    LOGINFO("dog_instance not found in list!");
    return 0;
}

static uint8_t IWDG_Deinit(IWDG_Instance_t **dog_instance)
{
    if(dog_instance == NULL || *dog_instance == NULL)
    {
        LOGERROR("In Deinit dog_instance is not valid!");
        return 0;
    }
    IWDG_Instance_t *temp_iwdg_instance = *dog_instance;
    /* 清空iwdg_config配置 */
    /* 规定dog_instance->dog_config必须将其生命域锁在任务函数中，所以不能对这块内存进行释放，需要操作的就只有将函数置空 */
    temp_iwdg_instance->dog_config->callback = NULL;

    /* 清空时间函数接口 */
    temp_iwdg_instance->sleep_func_interface->iwdg_sleep = NULL; 
    
    /* 清空函数接口 */
    temp_iwdg_instance->fall_asleep = NULL;
    temp_iwdg_instance->feed_dog = NULL;
    temp_iwdg_instance->is_dog_online = NULL;
    temp_iwdg_instance->iwdg_Deinit = NULL;

    /* 清除实例 */
    vPortFree(temp_iwdg_instance);
    *dog_instance = NULL;
    return 1;
}



