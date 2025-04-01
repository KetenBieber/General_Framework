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

/**
 * @brief 将各字符串相加
 *        通过连接字符串的方式实现字符串的拼接，尽量别用，是动态分配，可能造成内存管理报错
 * @param num 
 * @param string 
 * @return char* 
 */
char* str_sum(int num,...) {
    va_list args;
    va_start(args, num);
    int total_length = 0;
    // 计算总长度
    for (int i = 0; i < num; i++) {
        char* str = va_arg(args, char*);
        total_length += strlen(str);
    }
    va_end(args);

    // 为结果字符串分配内存，加 1 是为了存储字符串结束符 '\0'
    char* result = (char*)malloc(total_length + 1);
    result[0] = '\0';
    va_start(args, num);
    // 拼接字符串
    for (int i = 0; i < num; i++) {
        char* str = va_arg(args, char*);
        // 使用 strcat 函数，确保不会溢出
        strncat(result, str, strlen(str));
    }
    va_end(args);
    return result;
}