/**
 * @file bsp_bitband.h
 * @author Keten (2863861004@qq.com)
 * @brief  位带操作宏函数头文件，适合高速的io口操作，但是不利于代码阅读
 *         而且只有cortex - m3和cortex - m4支持位带操作
 * @version 0.1
 * @date 2024-09-27
 * 
 * @copyright Copyright (c) 2024
 * 
 * @attention :
 * @note :
 * @versioninfo :
 */
#pragma once

/* 位带操作原理:将每个位映射到一个独立的32位的字地址，通过访问这个字地址可以直接操作对应的位 */

/* 
    外设位带别名区地址:
    AliasAddr = BITBAND_BASE + (A - PERIPH_BASE)*8*4 + n*4
        A:片上外设位带区的某个比特，所在字节的地址为A,位序号为n（0<=n<=32）
        BITBAND_BASE:外设位带别名区基地址 0x42000000
        PERIPH_BASE：外设位带区基地址 0x40000000
        A - PERIPH_BASE:表示该比特前面有多少个字节，得到字节偏移量
            * 8:一个字节有8位
            * 4:书接上回，位带操作原理是把每个位映射到一个独立的字地址（8个位映射到32位地址），即一个位对应外设位带别名区的4个字节
        n*4:将位序号转换为 外设位带别名区中的字节偏移量，每个位在位带别名区中占用4个字节，所以乘以4
 */
/**
 * @brief "位带地址+位序号"转换"别名区地址"
 */
#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x02000000+((addr & 0x000FFFFF)<<5)+(bitnum<<2))

/**
 * @brief 将地址转换为指针
 * 
 */
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr))


/**
 * @brief 把位带别名区地址转换成指针
 * 
 */
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum))

// GPIO ODR 和 IDR 寄存器地址映射
#define GPIOA_ODR_Addr    (GPIOA_BASE+20)
#define GPIOB_ODR_Addr    (GPIOB_BASE+20)
#define GPIOC_ODR_Addr    (GPIOC_BASE+20)
#define GPIOD_ODR_Addr    (GPIOD_BASE+20)
#define GPIOE_ODR_Addr    (GPIOE_BASE+20)
#define GPIOF_ODR_Addr    (GPIOF_BASE+20)
#define GPIOG_ODR_Addr    (GPIOG_BASE+20)
#define GPIOH_ODR_Addr    (GPIOH_BASE+20)


#define GPIOA_IDR_Addr    (GPIOA_BASE+16)
#define GPIOB_IDR_Addr    (GPIOB_BASE+16)
#define GPIOC_IDR_Addr    (GPIOC_BASE+16)
#define GPIOD_IDR_Addr    (GPIOD_BASE+16)
#define GPIOE_IDR_Addr    (GPIOE_BASE+16)
#define GPIOF_IDR_Addr    (GPIOF_BASE+16)
#define GPIOG_IDR_Addr    (GPIOG_BASE+16)
#define GPIOH_IDR_Addr    (GPIOH_BASE+16)

// 单独操作 GPIO的某一个IO口，n(0,1,2...15),
// n表示具体是哪一个IO口
#define PAout(n)   BIT_ADDR(GPIOA_ODR_Addr,n)  //输出
#define PAin(n)    BIT_ADDR(GPIOA_IDR_Addr,n)  //输入

#define PBout(n)   BIT_ADDR(GPIOB_ODR_Addr,n)  //输出
#define PBin(n)    BIT_ADDR(GPIOB_IDR_Addr,n)  //输入

#define PCout(n)   BIT_ADDR(GPIOC_ODR_Addr,n)  //输出
#define PCin(n)    BIT_ADDR(GPIOC_IDR_Addr,n)  //输入

#define PDout(n)   BIT_ADDR(GPIOD_ODR_Addr,n)  //输出
#define PDin(n)    BIT_ADDR(GPIOD_IDR_Addr,n)  //输入

#define PEout(n)   BIT_ADDR(GPIOE_ODR_Addr,n)  //输出
#define PEin(n)    BIT_ADDR(GPIOE_IDR_Addr,n)  //输入

#define PFout(n)   BIT_ADDR(GPIOF_ODR_Addr,n)  //输出
#define PFin(n)    BIT_ADDR(GPIOF_IDR_Addr,n)  //输入

#define PGout(n)   BIT_ADDR(GPIOG_ODR_Addr,n)  //输出
#define PGin(n)    BIT_ADDR(GPIOG_IDR_Addr,n)  //输入

#define PHout(n)   BIT_ADDR(GPIOH_ODR_Addr,n)  //输出
#define PHin(n)    BIT_ADDR(GPIOH_IDR_Addr,n)  //输入

#define PIout(n)   BIT_ADDR(GPIOI_ODR_Addr,n)  //输出
#define PIin(n)    BIT_ADDR(GPIOI_IDR_Addr,n)  //输入

#define PJout(n)   BIT_ADDR(GPIOJ_ODR_Addr,n)  //输出
#define PJin(n)    BIT_ADDR(GPIOJ_IDR_Addr,n)  //输入

#define PKout(n)   BIT_ADDR(GPIOK_ODR_Addr,n)  //输出
#define PKin(n)    BIT_ADDR(GPIOK_IDR_Addr,n)  //输入