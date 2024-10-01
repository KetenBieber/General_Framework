/**
 * @file soft_reset.c
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
#include "soft_reset.h"

void Soft_Reset(void)
{
    __set_PRIMASK(1);
    HAL_NVIC_SystemReset();
}