/**
 * @file bsp_topics.h
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
#ifndef BSP_TOPICS_H 
#define BSP_TOPICS_H 

#ifdef __cplusplus
extern "C"{
#endif

/*----------------------------------include-----------------------------------*/
#include "cvector.h"
#include "circular_queue.h"
#include "stdint.h"
/*----------------------------------typedef-----------------------------------*/
#pragma pack(1)
struct internal_topic;
typedef struct publish_data_t {
    uint8_t* data;
    int len;
} publish_data;

typedef struct publisher_t {
    const char* pub_topic;
    struct internal_topic* topic;
    void (*publish)(struct publisher_t* pub, publish_data data);
} Publisher;

typedef struct subscriber_t{
    const char *sub_topic;
    circular_queue* queue;
    publish_data (*getdata)(struct subscriber_t* sub);
} Subscriber;
#pragma pack()

/*----------------------------------function----------------------------------*/
void SubPub_Init();
Publisher* register_pub(const char* topic);
Subscriber* register_sub(const char* topic, uint32_t buffer_len);

#ifdef __cplusplus
}
#endif

#endif	/* BSP_TOPICS_H */
