#ifndef _DBG_MACRO_H_
#define _DBG_MACRO_H_

#if 1
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#else
#include "CH57x_common.h"
#endif

#ifndef PRINT
#define PRINT(X...) printf(X)
#endif

#define DEFAULT_STRING_LEN 512
#define DBG_COLOR 1

#if DBG_COLOR
#define LOG_COLOR_BLACK "\033[30m"
#define LOG_COLOR_RED "\033[31m"
#define LOG_COLOR_GREEN "\033[32m"
#define LOG_COLOR_YELLOW "\033[33m"
#define LOG_COLOR_BLUE "\033[34m"
#define LOG_COLOR_MAGENTA "\033[35m"  // 紫色或洋红
#define LOG_COLOR_CYAN "\033[36m"     // 青色或蓝绿色
#define LOG_COLOR_WHITE "\033[37m"
#define LOG_COLOR_RESET "\033[0m"
#else
#define LOG_COLOR_BLACK ""
#define LOG_COLOR_RED ""
#define LOG_COLOR_GREEN ""
#define LOG_COLOR_YELLOW ""
#define LOG_COLOR_BLUE ""
#define LOG_COLOR_MAGENTA ""  // 紫色或洋红
#define LOG_COLOR_CYAN ""     // 青色或蓝绿色
#define LOG_COLOR_WHITE ""
#define LOG_COLOR_RESET ""
#endif

#define DEBUG_PRINT_STAND(COLOR, FMT, ...)                                                     \
    do {                                                                                       \
        char __DBG_string[DEFAULT_STRING_LEN] = {0};                                           \
        snprintf (__DBG_string, DEFAULT_STRING_LEN, "" COLOR "[%s]:" LOG_COLOR_RESET FMT "\n", \
                  __func__, ##__VA_ARGS__);                                                    \
        PRINT ("%s", __DBG_string);                                                            \
    } while (0)
#define DEBUG_PRINT(FMT, ...) DEBUG_PRINT_STAND (LOG_COLOR_GREEN, FMT, ##__VA_ARGS__)

#define VAR_NAME(x) #x
#define VAR_PRINT_STAND(COLOR, VAR, FMT, ...)                                                        \
    do {                                                                                             \
        char __DBG_string[DEFAULT_STRING_LEN] = {0};                                                 \
        snprintf (__DBG_string, DEFAULT_STRING_LEN, "" COLOR "[%s]VAR(%s)" LOG_COLOR_RESET FMT "\n", \
                  __func__, VAR_NAME (VAR), ##__VA_ARGS__);                                          \
        PRINT ("%s", __DBG_string);                                                                  \
    } while (0)
#define VAR_PRINT_UD(VAR) VAR_PRINT_STAND (LOG_COLOR_YELLOW, VAR, "value(%u)", VAR)
#define VAR_PRINT_LLU(VAR) VAR_PRINT_STAND (LOG_COLOR_YELLOW, VAR, "value(%llu)", VAR)
#define VAR_PRINT_INT(VAR) VAR_PRINT_STAND (LOG_COLOR_YELLOW, VAR, "value(%d)", VAR)
#define VAR_PRINT_LL(VAR) VAR_PRINT_STAND (LOG_COLOR_YELLOW, VAR, "value(%lld)", VAR)
#define VAR_PRINT_HEX(VAR) VAR_PRINT_STAND (LOG_COLOR_YELLOW, VAR, "value(0x%x)", VAR)
#define VAR_PRINT_FLOAT(VAR) VAR_PRINT_STAND (LOG_COLOR_YELLOW, VAR, "value(%f)", VAR)
#define VAR_PRINT_CH(VAR) VAR_PRINT_STAND (LOG_COLOR_YELLOW, VAR, "CH[%c]", VAR)
#define VAR_PRINT_POS(VAR) VAR_PRINT_STAND (LOG_COLOR_YELLOW, VAR, "ADDR[0X%p]", VAR)

#define VAR_PRINT_ARR_HEX(VAR, _VAR_SIZE)                                             \
    do {                                                                              \
        char __DBG_string[DEFAULT_STRING_LEN] = {0};                                  \
        const uint16_t __DBG_MaxLine = 8;                                             \
        uint16_t __DBG_len = 0x0, __DBG_varSize_lines = _VAR_SIZE / __DBG_MaxLine;    \
        int __DBG_pos = 0;                                                            \
        snprintf (__DBG_string, DEFAULT_STRING_LEN,                                   \
                  "" LOG_COLOR_YELLOW "[%s]VAR(%s)[size:%u]HEX" LOG_COLOR_RESET "",   \
                  __func__, VAR_NAME (VAR), (unsigned int)_VAR_SIZE);                 \
        PRINT (__DBG_string);                                                         \
        __DBG_len = strlen (__DBG_string);                                            \
        __DBG_len /= 2;                                                               \
        putchar ('\n');                                                               \
        __DBG_varSize_lines += (_VAR_SIZE % __DBG_MaxLine) ? 1 : 0;                   \
        for (int __DBG_loopA = 0; __DBG_loopA < __DBG_varSize_lines; __DBG_loopA++) { \
            for (int __DBG_loopB = 0; __DBG_loopB < __DBG_len; __DBG_loopB++) {       \
                putchar (' ');                                                        \
            }                                                                         \
            for (int __DBG_loopC = 0; __DBG_loopC < __DBG_MaxLine; __DBG_loopC++) {   \
                PRINT ("[%x] ", *((VAR) + __DBG_pos++));                              \
                if (__DBG_pos == _VAR_SIZE)                                           \
                    break;                                                            \
            }                                                                         \
            putchar ('\n');                                                           \
        }                                                                             \
    } while (0)

#define __STRING__INVALID_VAR "invalid_var[NULL]"
#define VAR_PRINT_STAND_1(COLOR, VAR, FMT, ...)                                                          \
    do {                                                                                                 \
        char __DBG_string[DEFAULT_STRING_LEN] = {0};                                                     \
        if (VAR == NULL) {                                                                               \
            snprintf (__DBG_string, DEFAULT_STRING_LEN, "" COLOR "[%s]VAR(%s)" LOG_COLOR_RESET "%s\n",   \
                      __func__, VAR_NAME (VAR), __STRING__INVALID_VAR);                                  \
        } else {                                                                                         \
            snprintf (__DBG_string, DEFAULT_STRING_LEN, "" COLOR "[%s]VAR(%s)" LOG_COLOR_RESET FMT "\n", \
                      __func__, VAR_NAME (VAR), ##__VA_ARGS__);                                          \
        }                                                                                                \
        PRINT ("%s", __DBG_string);                                                                      \
    } while (0)
#define VAR_PRINT_STRING(VAR) VAR_PRINT_STAND_1 (LOG_COLOR_YELLOW, VAR, "STR{%s}", VAR)


#define MACRO_PRINT_STAND(COLOR, VAR, FMT, ...)                                                        \
    do {                                                                                               \
        char __DBG_string[DEFAULT_STRING_LEN] = {0};                                                   \
        snprintf (__DBG_string, DEFAULT_STRING_LEN, "" COLOR "[%s]MACRO(%s)" LOG_COLOR_RESET FMT "\n", \
                  __func__, VAR, ##__VA_ARGS__);                                                       \
        PRINT ("%s", __DBG_string);                                                                    \
    } while (0)
#define MACRO_PRINT_STR(VAR) MACRO_PRINT_STAND (LOG_COLOR_CYAN, #VAR, "STR{%s}", VAR)
#define MACRO_PRINT_INT(VAR) MACRO_PRINT_STAND (LOG_COLOR_CYAN, #VAR, "value{%d}", VAR)
#define MACRO_PRINT_UD(VAR) MACRO_PRINT_STAND (LOG_COLOR_CYAN, #VAR, "value{%u}", VAR)
#define MACRO_PRINT_HEX(VAR) MACRO_PRINT_STAND (LOG_COLOR_CYAN, #VAR, "value{0x%x}", VAR)
#define MACRO_PRINT_FLOAT(VAR) MACRO_PRINT_STAND (LOG_COLOR_CYAN, #VAR, "value{%f}", VAR)

#define SIZE_ARRARY(ARR) sizeof (ARR) / sizeof (ARR[0])

/**
 * SET_BIT - 设置指定的位为 1 (置位)
 * @param REG: 要操作的寄存器或变量的地址 (指针)
 * @param BIT: 要设置的位的位置 (0-based index)
 *
 * 原理: 使用按位或 (|) 操作。将目标位置1，其他位保持不变。
 * 例如: REG = REG | (1 << BIT)
 */
#define SET_BIT(REG, BIT) ((REG) |= (1U << (BIT)))

/**
 * CLEAR_BIT - 清除指定的位为 0 (清零)
 * @param REG: 要操作的寄存器或变量的地址 (指针)
 * @param BIT: 要清除的位的位置 (0-based index)
 *
 * 原理: 使用按位与 (&) 和按位取反 (~) 操作。
 * 先创建一个除了目标位为0其他位都为1的掩码，然后与原值相与。
 * 例如: REG = REG & ~(1 << BIT)
 */
#define CLEAR_BIT(REG, BIT) ((REG) &= ~(1U << (BIT)))

/**
 * CHECK_BIT - 检查指定的位是否为 1
 * @param REG: 要检查的寄存器或变量的值
 * @param BIT: 要检查的位的位置 (0-based index)
 * @return: 如果指定位为1，返回非零值 (真)；如果为0，返回0 (假)
 *
 * 原理: 使用按位与 (&) 操作。将目标位提取出来，结果非零则为真。
 * 例如: (REG & (1 << BIT)) != 0
 */
#define CHECK_BIT(REG, BIT) ((REG) & (1U << (BIT)))

#define DBG_SELET_CURRENT_EVENT(_EVENTS, _CURRENT_EVENT)      \
    do {                                                      \
        const uint16_t _DBG_MaxCnt = 16;                      \
        uint16_t _DBG_ret = 0x0, _DBG_loop = 0;               \
        for (; _DBG_loop < _DBG_MaxCnt; _DBG_loop++) {        \
            if (CHECK_BIT (_EVENTS, _DBG_MaxCnt - _DBG_loop)) \
                break;                                        \
        }                                                     \
        SET_BIT (_DBG_ret, _DBG_MaxCnt - _DBG_loop);          \
        _CURRENT_EVENT = _DBG_ret;                            \
    } while (0)

#define DBG_CHECK_SP()                                             \
    do {                                                           \
        uint32_t _DGB_sp = 0;                                      \
        __asm volatile ("mv %0, sp"                                \
                        : "=r"(_DGB_sp)                            \
                        :                                          \
                        : "memory");                               \
        DEBUG_PRINT ("sp[%x], [%u]KB", _DGB_sp, (_DGB_sp / 1024)); \
    } while (0)

#endif
