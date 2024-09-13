/**
 * @file logtest.h
 * @author Keten (2863861004@qq.com)
 * @brief
 * @version 0.1
 * @date 2024-09-13
 *
 * @copyright Copyright (c) 2024
 *
 * @attention :
 * @note :
 * @versioninfo :
 */

#ifndef LOGTEST_H
#define LOGTEST_H

#ifdef __cplusplus
extern "C"
{
#endif

/*----------------------------------include-----------------------------------*/
#include "bsp_log.h"
/*-----------------------------------macro------------------------------------*/
#define LOG_TEST_TASK_STACK_SIZE 1024
#define LOG_TEST_TASK_PRIORITY 5
    /*----------------------------------typedef-----------------------------------*/
    typedef int (*log_test_func)(void);
    typedef int (*log_test_func1)(void);
    typedef int (*log_test_func2)(void);
    typedef int (*log_test_func666)(void);
    typedef int (*log_test_func777)(void);
    /*----------------------------------variable----------------------------------*/

    /*-------------------------------------os-------------------------------------*/

    /*----------------------------------function----------------------------------*/

    /*------------------------------------test------------------------------------*/
    int a = 1;

    int b = 2;

    int c = 3;

#ifdef __cplusplus
}
#endif

#endif /* LOGTEST_H */
