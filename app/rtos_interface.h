/**
 * @file rtos_interface.h
 * @author Keten (2863861004@qq.com)
 * @brief 在这里封装了一些简单的rtos宏接口，用更加直观的命名，方便直接在app层调用，减少代码上阅读的难度
 * @version 0.1
 * @date 2024-09-24
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention :
 * @note :
 * @versioninfo :
 */

#include "freertos.h"
#include "queue.h"

/**
 * @brief 通过调用这个宏，可以间接实现 队列接收判断 ，这个宏的值应该代表 1 为接收到了； 0 为未接收到
 *        建议使用阻塞，降低cpu占用
 */
#define CHECK_QUEUE_RECEIVE(queue, msg, ticks)  (xQueueReceive((queue),(msg),(ticks)) == pdPASS)

#define DELETE_MYSELF_THREAD()                   vTaskDelete(NULL) 


