/**
 * @file bsp_topics.c
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
#include "topics.h"

#include "stdlib.h"
#include "string.h"

struct internal_topic {
    Publisher* pub;
    cvector* subs;
    const char* topic_str;
};

cvector* topics;

void SubPub_Init() { topics = cvector_create(sizeof(struct internal_topic*)); }
static void pub_commit(struct publisher_t* pub, publish_data data);
static publish_data sub_get(struct subscriber_t* sub);
static struct internal_topic* register_topic(const char* topic);



/**
 * @brief 发布函数  
 *        将数据推送到每个订阅者的队列中，如果队列已满，则先弹出最旧的数据
 * 
 * @param pub 
 * @param data 
 */
static void pub_commit(struct publisher_t* pub, publish_data data) {
    for (uint32_t i = 0; i < pub->topic->subs->cv_len; ++i) {
        Subscriber* now = *(Subscriber**)cvector_val_at(pub->topic->subs, i);
        if (now->queue->cq_len == now->queue->cq_max_len) circular_queue_pop(now->queue);
        circular_queue_push(now->queue, &data);
    }
}


/**
 * @brief 订阅函数
 *        如果队列中有数据，则弹出并返回，否则返回一个空数据结构
 * 
 * @param sub 
 * @return publish_data 
 */
static publish_data sub_get(struct subscriber_t* sub) {
    publish_data now;
    now.data = NULL;
    now.len = -1;
    if (sub->queue->cq_len) {
        now = *(publish_data*)circular_queue_pop(sub->queue);
    }
    return now;
}


/**
 * @brief 注册话题函数
 *        如果话题已存在，则返回现有的主题结构体，否则创建一个新的主题结构体并添加到topics动态数组中
 * 
 * @param topic 
 * @return struct internal_topic* 
 */
static struct internal_topic* register_topic(const char* topic) {
    for (uint32_t i = 0; i < topics->cv_len; ++i) {
        struct internal_topic* now = *(struct internal_topic**)cvector_val_at(topics, i);
        if (!strcmp(now->topic_str, topic)) {
            return now;
        }
    }
    struct internal_topic* now = malloc(sizeof(struct internal_topic));
    now->pub = NULL;
    now->subs = cvector_create(sizeof(Subscriber*));
    now->topic_str = topic;
    cvector_pushback(topics,&now);
    return now;
}


Publisher* register_pub(const char* topic) {
    struct internal_topic* now_topic = register_topic(topic);
    if (now_topic->pub != NULL) return now_topic->pub;
    Publisher* obj = malloc(sizeof(Publisher));
    obj->pub_topic = topic;
    obj->topic = now_topic;
    obj->publish = pub_commit;
    now_topic->pub = obj;
    return obj;
}


Subscriber* register_sub(const char* topic, uint32_t buffer_len) {
    struct internal_topic* now_topic = register_topic(topic);
    Subscriber* obj = malloc(sizeof(Subscriber));
    obj->queue = create_circular_queue(sizeof(publish_data), buffer_len);
    cvector_pushback(now_topic->subs, &obj);
    obj->sub_topic = topic;
    obj->getdata = sub_get;
    return obj;
}