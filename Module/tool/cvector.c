// C语言实现动态数组 
// 使用size_t 实现平台无关性
#include "cvector.h"

/**
 * @brief 创建动态数组
 *        分配内存并初始化数组的基本属性
 * @param size 数组成员的大小
 * @return cvector* 
 */
cvector* cvector_create(const size_t size) {
    cvector* cv = (cvector*)malloc(sizeof(struct _cvector));

    if (!cv) return NULL;

    cv->cv_pdata = malloc(MIN_LEN * size);

    if (!cv->cv_pdata) {
        free(cv);
        return NULL;
    }

    cv->cv_size = size;
    cv->cv_tot_len = MIN_LEN;
    cv->cv_len = 0;

    return cv;
}


/**
 * @brief 动态数组的摧毁
 * 
 * @param cv 
 */
void cvector_destroy(cvector* cv) {
    free(cv->cv_pdata);
    free(cv);
    return;
}


/**
 * @brief 获取当前cvector的长度
 * 
 * @param cv 
 * @return size_t 
 */
size_t cvector_length(cvector* cv) { return cv->cv_len; }


/**
 * @brief 往动态数组中添加新元素
 *        
 * 
 * @param cv 
 * @param memb 
 * @return void* 
 */
void* cvector_pushback(cvector* cv, void* memb) {
    if (cv->cv_len >= cv->cv_tot_len) {
        // void* pd_sav = cv->cv_pdata;
        // 以cv_tot_len为最小单位进行扩张，避免反复realloc
        cv->cv_tot_len <<= EXPANED_VAL;
        cv->cv_pdata = realloc(cv->cv_pdata, cv->cv_tot_len * cv->cv_size);
    }

    memcpy((char*)cv->cv_pdata + cv->cv_len * cv->cv_size, memb, cv->cv_size);
    cv->cv_len++;

    return cv->cv_pdata + (cv->cv_len - 1) * cv->cv_size;
}


/**
 * @brief 访问cvctor中的元素
 *        通过计算元素的内存偏移量来实现对特定索引元素的访问
 * @param cv 
 * @param index 
 * @return void* 
 */
void* cvector_val_at(cvector* cv, size_t index) {
    return cv->cv_pdata + index * cv->cv_size;
}
