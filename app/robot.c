/**
 * @file robot.c
 * @author Keten (2863861004@qq.com)
 * @brief 机器人的主要功能，负责初始化以及特色功能任务的编写
 * @version 0.1
 * @date 2024-09-14
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention :
 * @note :
 * @versioninfo :
 */
#include "bsp_init.h"
#include "robot.h"
#include "robot_def.h"
#include "robot_task.h"

#ifdef USE_SYSTEMVIEW
/* 添加systemview支持项 */
#include "SEGGER_SYSVIEW.h"
#endif

/* 更多特色功能可以在这里通过宏开关来决定是否使用 */

void Robot_Init()
{
    /* 关闭中断，防止在初始化过程中发生中断 */
    __disable_irq();

    osTaskInit();
    Bsp_Init();
    
  /* 使用systemview需要在系统调度前初始化 */
#ifdef USE_SYSTEMVIEW
  SEGGER_SYSVIEW_Conf();
#endif    

    /*初始化完成，开启中断 */
    __enable_irq();
}


