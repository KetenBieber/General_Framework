/**
 * @file soft_reset.h
 * @author Keten (2863861004@qq.com)
 * @brief stm32软复位,软件复位的实现
 * @version 0.1
 * @date 2024-09-29
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention :
 * @note :
 * @versioninfo :
 */
#ifndef SOFT_RESET_H 
#define SOFT_RESET_H 

#ifdef __cplusplus
extern "C"{
#endif

/*----------------------------------include-----------------------------------*/

#include "stm32f4xx_hal.h"



/*----------------------------------function----------------------------------*/
void Soft_Reset(void);


#ifdef __cplusplus
}
#endif

#endif	/* SOFT_RESET_H */
