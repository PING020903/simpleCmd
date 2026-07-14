/**
 * @file DBG_macro.h
 * @brief 调试打印宏定义集合
 * 
 * 提供以下功能：
 * - 彩色日志输出（支持 ANSI 转义码）
 * - 变量值打印（自动获取变量名和行号）
 * - 数组十六进制打印
 * - 字符串和宏值打印
 * - 位操作辅助宏（SET_BIT/CLEAR_BIT/CHECK_BIT）
 * 
 * @note 所有打印宏的参数已添加括号保护，调用时无需额外加括号
 *       例如：VAR_PRINT_INT(myVar + 1) 可直接使用，无需 VAR_PRINT_INT((myVar + 1))
 */

#pragma once
#ifndef _DBG_MACRO_H_
#define _DBG_MACRO_H_

#if 1
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#else
#include "CH58x_common.h"
#endif

/** @brief 调试缓冲区默认长度（256 字节） */
#define DBG_DEFAULT_BUFFER_LEN    0x100

/** @brief 是否启用彩色输出（1=启用，0=禁用） */
#define DBG_COLOR                 1

#if DBG_COLOR
#define LOG_COLOR_BLACK           "\033[30m"
#define LOG_COLOR_RED             "\033[31m"
#define LOG_COLOR_GREEN           "\033[32m"
#define LOG_COLOR_YELLOW          "\033[33m"
#define LOG_COLOR_BLUE            "\033[34m"
#define LOG_COLOR_MAGENTA         "\033[35m"
#define LOG_COLOR_CYAN            "\033[36m"
#define LOG_COLOR_WHITE           "\033[37m"
#define LOG_COLOR_RESET           "\033[0m"
#else
#define LOG_COLOR_BLACK           ""
#define LOG_COLOR_RED             ""
#define LOG_COLOR_GREEN           ""
#define LOG_COLOR_YELLOW          ""
#define LOG_COLOR_BLUE            ""
#define LOG_COLOR_MAGENTA         ""
#define LOG_COLOR_CYAN            ""
#define LOG_COLOR_WHITE           ""
#define LOG_COLOR_RESET           ""
#endif


extern char __DBG_string[DBG_DEFAULT_BUFFER_LEN];


/**
 * @brief 标准调试打印宏（内部使用）
 * @param COLOR 颜色代码
 * @param FMT 格式化字符串
 * @param ... 可变参数
 */
#define DEBUG_PRINT_STAND(COLOR, FMT, ...)                                                         \
    do {                                                                                           \
        memset (__DBG_string, 0, DBG_DEFAULT_BUFFER_LEN);                                          \
        snprintf (__DBG_string, DBG_DEFAULT_BUFFER_LEN, "" COLOR "[%s]:" LOG_COLOR_RESET FMT "\n", \
                  __func__, ##__VA_ARGS__);                                                        \
        fputs ((char *)__DBG_string, stdout);                                                      \
    } while (0)

/**
 * @brief 普通调试信息打印（绿色）
 * @param FMT 格式化字符串
 * @param ... 可变参数
 */
#define DEBUG_PRINT(FMT, ...)       DEBUG_PRINT_STAND(LOG_COLOR_GREEN, FMT, ##__VA_ARGS__)

/**
 * @brief 错误信息打印（红色）
 * @param FMT 格式化字符串
 * @param ... 可变参数
 */
#define ERROR_PRINT(FMT, ...)       DEBUG_PRINT_STAND(LOG_COLOR_RED, FMT, ##__VA_ARGS__)

/**
 * @brief 函数调用及返回值打印
 * @param FUNCTION 要调用的函数
 * @param RET 存储返回值的变量
 * @param RET_FORMAT 返回值格式化字符串
 */
#define DEBUG_PRINT_FUNC(FUNCTION, RET, RET_FORMAT)              \
    do {                                                         \
        RET = FUNCTION;                                          \
        DEBUG_PRINT_STAND(LOG_COLOR_GREEN,                       \
                          "callFunc[%s]->ret:[" RET_FORMAT "]",  \
                          #FUNCTION, RET);                       \
    } while (0)

/** @brief 获取变量名的字符串形式 */
#define VAR_NAME(x)               #x

/**
 * @brief 标准变量打印宏（内部使用）
 * @param COLOR 颜色代码
 * @param VAR 变量（已加括号保护）
 * @param FMT 格式化字符串
 * @param ... 可变参数
 */
#define VAR_PRINT_STAND(COLOR, VAR, FMT, ...)                                                            \
    do {                                                                                                 \
        memset (__DBG_string, 0, DBG_DEFAULT_BUFFER_LEN);                                                \
        snprintf (__DBG_string, DBG_DEFAULT_BUFFER_LEN, "" COLOR "[%s]VAR(%s)" LOG_COLOR_RESET FMT "\n", \
                  __func__, VAR_NAME(VAR), ##__VA_ARGS__);                                               \
        fputs ((char *)__DBG_string, stdout);                                                            \
    } while (0)

/**
 * @brief 打印无符号整数（带行号）
 * @param VAR 要打印的变量
 */
#define VAR_PRINT_UD(VAR)         VAR_PRINT_STAND(LOG_COLOR_YELLOW, (VAR), "line[%d]value(%u)", __LINE__, (VAR))

/**
 * @brief 打印 unsigned long long 类型（带行号）
 * @param VAR 要打印的变量
 */
#define VAR_PRINT_LLU(VAR)        VAR_PRINT_STAND(LOG_COLOR_YELLOW, (VAR), "line[%d]value(%llu)", __LINE__, (VAR))

/**
 * @brief 打印有符号整数（带行号）
 * @param VAR 要打印的变量
 */
#define VAR_PRINT_INT(VAR)        VAR_PRINT_STAND(LOG_COLOR_YELLOW, (VAR), "line[%d]value(%d)", __LINE__, (VAR))

/**
 * @brief 打印 long long 类型（带行号）
 * @param VAR 要打印的变量
 */
#define VAR_PRINT_LL(VAR)         VAR_PRINT_STAND(LOG_COLOR_YELLOW, (VAR), "line[%d]value(%lld)", __LINE__, (VAR))

/**
 * @brief 打印十六进制值（带行号）
 * @param VAR 要打印的变量
 */
#define VAR_PRINT_HEX(VAR)        VAR_PRINT_STAND(LOG_COLOR_YELLOW, (VAR), "line[%d]value(%x)HEX", __LINE__, (VAR))

/**
 * @brief 打印浮点数（带行号）
 * @param VAR 要打印的变量
 */
#define VAR_PRINT_FLOAT(VAR)      VAR_PRINT_STAND(LOG_COLOR_YELLOW, (VAR), "line[%d]value(%f)", __LINE__, (VAR))

/**
 * @brief 打印字符（带行号）
 * @param VAR 要打印的变量
 */
#define VAR_PRINT_CH(VAR)         VAR_PRINT_STAND(LOG_COLOR_YELLOW, (VAR), "line[%d]CH[%c]", __LINE__, (VAR))

/**
 * @brief 打印指针地址（带行号）
 * @param VAR 要打印的变量
 */
#define VAR_PRINT_POS(VAR)        VAR_PRINT_STAND(LOG_COLOR_YELLOW, (VAR), "line[%d]ADDR[%p]", __LINE__, (VAR))

/**
 * @brief 以十六进制数组形式打印数据
 * @param VAR 数组指针
 * @param _VAR_SIZE 数组大小
 */
#define VAR_PRINT_ARR_HEX(VAR, _VAR_SIZE)                                            \
    do {                                                                             \
        memset (__DBG_string, 0, DBG_DEFAULT_BUFFER_LEN);                            \
        const uint16_t __DBG_MaxLine = 8;                                            \
        uint16_t __DBG_len = 0x0, __DBG_varSize_lines = (_VAR_SIZE) / __DBG_MaxLine; \
        int __DBG_pos = 0;                                                           \
        snprintf ((char *)__DBG_string, DBG_DEFAULT_BUFFER_LEN,                      \
                  "" LOG_COLOR_YELLOW "[%s]VAR(%s)[size:%u]HEX" LOG_COLOR_RESET "",  \
                  __func__, VAR_NAME(VAR), (unsigned int)(_VAR_SIZE));               \
        fputs ((char *)__DBG_string, stdout);                                        \
        __DBG_len = strlen((char *)__DBG_string);                                    \
        __DBG_len /= 2;                                                              \
        if (__DBG_len > 8)                                                           \
            __DBG_len = 8;                                                           \
        putchar('\n');                                                               \
        __DBG_varSize_lines += ((_VAR_SIZE) % __DBG_MaxLine) ? 1 : 0;                \
        for (int __DBG_loopA = 0; __DBG_loopA < __DBG_varSize_lines; __DBG_loopA++) {\
            for (int __DBG_loopB = 0; __DBG_loopB < __DBG_len; __DBG_loopB++) {      \
                putchar(' ');                                                        \
            }                                                                        \
            for (int __DBG_loopC = 0; __DBG_loopC < __DBG_MaxLine; __DBG_loopC++) {  \
                printf("[%x] ", (*((VAR) + __DBG_pos++)));                           \
                if (__DBG_pos == (_VAR_SIZE))                                        \
                    break;                                                           \
            }                                                                        \
            putchar('\n');                                                           \
        }                                                                            \
    } while (0)

/** @brief 无效变量的错误提示字符串 */
#define __STRING__INVALID_VAR     "invalid_var[NULL]"

/**
 * @brief 支持 NULL 检查的标准变量打印宏（内部使用）
 * @param COLOR 颜色代码
 * @param VAR 变量（已加括号保护）
 * @param FMT 格式化字符串
 * @param ... 可变参数
 */
#define VAR_PRINT_STAND_1(COLOR, VAR, FMT, ...)                                                              \
    do {                                                                                                     \
        memset (__DBG_string, 0, DBG_DEFAULT_BUFFER_LEN);                                                    \
        if ((VAR) == NULL) {                                                                                 \
            snprintf (__DBG_string, DBG_DEFAULT_BUFFER_LEN, "" COLOR "[%s]VAR(%s)" LOG_COLOR_RESET "%s\n",   \
                      __func__, VAR_NAME(VAR), __STRING__INVALID_VAR);                                       \
        } else {                                                                                             \
            snprintf (__DBG_string, DBG_DEFAULT_BUFFER_LEN, "" COLOR "[%s]VAR(%s)" LOG_COLOR_RESET FMT "\n", \
                      __func__, VAR_NAME(VAR), ##__VA_ARGS__);                                               \
        }                                                                                                    \
        fputs ((char *)__DBG_string, stdout);                                                                \
    } while (0)

/**
 * @brief 打印字符串（带 NULL 检查）
 * @param VAR 要打印的字符串指针
 */
#define VAR_PRINT_STRING(VAR)     VAR_PRINT_STAND_1(LOG_COLOR_YELLOW, (VAR), "STR{%s}", (VAR))

/**
 * @brief 标准宏值打印宏（内部使用）
 * @param COLOR 颜色代码
 * @param VAR 宏名（字符串形式）
 * @param FMT 格式化字符串
 * @param ... 可变参数
 */
#define MACRO_PRINT_STAND(COLOR, VAR, FMT, ...)                                                            \
    do {                                                                                                   \
        memset (__DBG_string, 0, DBG_DEFAULT_BUFFER_LEN);                                                  \
        snprintf (__DBG_string, DBG_DEFAULT_BUFFER_LEN, "" COLOR "[%s]MACRO(%s)" LOG_COLOR_RESET FMT "\n", \
                  __func__, VAR, ##__VA_ARGS__);                                                           \
        fputs ((char *)__DBG_string, stdout);                                                              \
    } while (0)

/**
 * @brief 打印字符串宏的值
 * @param VAR 宏名
 */
#define MACRO_PRINT_STR(VAR)      MACRO_PRINT_STAND(LOG_COLOR_CYAN, #VAR, "STR{%s}", (VAR))

/**
 * @brief 打印整数宏的值
 * @param VAR 宏名
 */
#define MACRO_PRINT_INT(VAR)      MACRO_PRINT_STAND(LOG_COLOR_CYAN, #VAR, "value{%d}", (VAR))

/**
 * @brief 打印无符号整数宏的值
 * @param VAR 宏名
 */
#define MACRO_PRINT_UD(VAR)       MACRO_PRINT_STAND(LOG_COLOR_CYAN, #VAR, "value{%u}", (VAR))

/**
 * @brief 打印十六进制宏的值
 * @param VAR 宏名
 */
#define MACRO_PRINT_HEX(VAR)      MACRO_PRINT_STAND(LOG_COLOR_CYAN, #VAR, "value{0x%x}", (VAR))

/**
 * @brief 打印浮点宏的值
 * @param VAR 宏名
 */
#define MACRO_PRINT_FLOAT(VAR)    MACRO_PRINT_STAND(LOG_COLOR_CYAN, #VAR, "value{%f}", (VAR))

/**
 * @brief 计算数组元素个数
 * @param ARR 数组名
 * @return 数组元素个数
 */
#define SIZE_ARRARY(ARR)          (sizeof(ARR) / sizeof((ARR)[0]))

/**
 * @brief 设置指定的位为 1（置位）
 * @param REG 要操作的寄存器或变量（指针类型）
 * @param BIT 要设置的位位置（0-based index）
 * 
 * 原理：使用按位或 (|) 操作，将目标位置 1，其他位保持不变。
 * 示例：SET_BIT(myReg, 3) 等价于 myReg |= (1 << 3)
 */
#define SET_BIT(REG, BIT)         ((REG) |= (1U << (BIT)))

/**
 * @brief 清除指定的位为 0（清零）
 * @param REG 要操作的寄存器或变量（指针类型）
 * @param BIT 要清除的位位置（0-based index）
 * 
 * 原理：使用按位与 (&) 和按位取反 (~) 操作。
 * 先创建一个除了目标位为 0 其他位都为 1 的掩码，然后与原值相与。
 * 示例：CLEAR_BIT(myReg, 3) 等价于 myReg &= ~(1 << 3)
 */
#define CLEAR_BIT(REG, BIT)       ((REG) &= ~(1U << (BIT)))

/**
 * @brief 检查指定的位是否为 1
 * @param REG 要检查的寄存器或变量的值
 * @param BIT 要检查的位位置（0-based index）
 * @return 如果指定位为 1，返回非零值；如果为 0，返回 0
 * 
 * 原理：使用按位与 (&) 操作，将目标位提取出来，结果非零则为真。
 * 示例：CHECK_BIT(myReg, 3) 等价于 (myReg & (1 << 3)) != 0
 */
#define CHECK_BIT(REG, BIT)       ((REG) & (1U << (BIT)))

/**
 * @brief 从事件标志中选择当前优先级最高的事件
 * @param _EVENTS 事件标志变量
 * @param _CURRENT_EVENT 输出变量，存储选中的事件
 * 
 * 算法：从高位向低位扫描，返回第一个为 1 的位对应的事件
 */
#define DBG_SELET_CURRENT_EVENT(_EVENTS, _CURRENT_EVENT)                  \
    do {                                                                  \
        const uint16_t _DBG_MaxCnt = 16;                                  \
        uint16_t _DBG_ret = 0x0, _DBG_loop = 0;                           \
        for (; _DBG_loop < _DBG_MaxCnt; _DBG_loop++) {                    \
            if (CHECK_BIT((_EVENTS), _DBG_MaxCnt - _DBG_loop))            \
                break;                                                    \
        }                                                                 \
        SET_BIT(_DBG_ret, _DBG_MaxCnt - _DBG_loop);                       \
        _CURRENT_EVENT = _DBG_ret;                                        \
    } while (0)


#endif /* _DBG_MACRO_H_ */
