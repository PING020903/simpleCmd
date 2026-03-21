#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "DBG_macro.h"
#include "cmdUserStringParse.h"

// 做外部实现的内存申请&释放的函数接口声明, 可以让用户自行实现内存该如何申请&释放
extern void *cmd_MemoryAlloc(size_t bytes);
extern void cmd_MemoryFree(void *mem);

/**
 * @brief 用户的字符串
 */
static userString *userData = NULL;

/**
 * @brief 用户参数的数量
 */
static int userDataCnt = 0;

static size_t characters_remaining = 0U;

void RESET_USERDATA_RECORD(void)
{
    cmd_MemoryFree(userData);
    userData = NULL;
    userDataCnt = 0;
}

/*
 * @brief 获取解析到的用户参数个数
 */
int userParse_GetUserParamCnt(void)
{
    return userDataCnt;
}

userString *userParse_pUserData(void)
{
    return userData;
}

/**
 * @brief 可解析的字符( 参数用 )
 * @param ch 当前地址的字符
 * @return OK: 0,  ERROR: 1
 */
static inline int passableChParam(const char ch)
{
    static const char NoSupportChar = ' ';
    return (ch > NoSupportChar)
               ? 0
               : 1;
}

#if 0
/**
 * @brief 解析空格中混含的用户参数
 * @param userParam 用户参数
 * @return 保存用户参数的首地址
 */
void* ParseSpace(const char* userParam)
{
    const char space = ' ';
    size_t passLen = 0; // 已经处理的长度
    const size_t passConstant = PARSE_SIZE - userDataPass;
    userString* tmp = NULL;
    const char* str = userParam, * str2 = NULL;
    if (userParam == NULL)
        return NULL;

    passLen = passConstant;
    do
    {
        // 在指令支持的最大长度内跳过所空格, 先检查是否还有有效字符
        for (; passableChParam(*str) && str < userParam + passConstant; str++);
        if (str >= userParam + passConstant)
        {
#if NODE_DEBUG
            printf("--<%s>%d--too far:%lld, strAdd:%p, tooFarAdd:%p\n",
                __func__, __LINE__, userParam + passConstant - str, str, userParam + passConstant);
#endif
            return userData; // 超出可解析的长度
        }

        userDataCnt++;
        passLen += str - userParam;
        userData = (userString*)realloc(tmp, userDataCnt * sizeof(userString));
#if 0
        if (userData == tmp)
        {
            free(userData);
            return userData = NULL;
        }
#endif // 此处有个拿捏不定的 bug, 若 userDataCnt 在结束一次 CommandParse 后没有置0的情况下,
       // 会导致 realloc 失败, 但返回的是原先的地址
        if (userData == NULL)
        {
            ERROR_PRINT("<%s>alloc memory fail", __func__);
            free(tmp);
            return (void*)userData;
        }

        (userData + userDataCnt - 1)->strHead = (void*)str;// 字符串头

        str2 = str;
        for (; *str2 != space &&
            str2 < userParam + passConstant &&
            *str2 != '\0'; str2++); // 略过非空格
        if (str2 > (passConstant + userParam))
        {
            // 超出了限定的长度
            DEBUG_PRINT("<%s>userParam is too long( %u, %p )...\
 the end string has no end\n",
                __func__, PARSE_SIZE,
                (void*)(str2 - (passConstant + userParam)));
            (userData + userDataCnt - 1)->len =
                (size_t)((passConstant + userParam) - str);
            return (void*)userData;
        }

        (userData + userDataCnt - 1)->len = (size_t)(str2 - str);
        str += (userData + userDataCnt - 1)->len;

        tmp = userData;
#if NODE_DEBUG
        printf("<%s>len:%llu, |%s|\n", __func__,
            (userData + userDataCnt - 1)->len, (char*)((userData + userDataCnt - 1)->strHead));
#endif
    } while (str <= userParam + passConstant || *str2 == '\0');
    return (void*)userData;
}
#endif
static const char *jumpSpace(const char *uStr, size_t *len)
{
    if (!uStr || !len || *len == 0U || *uStr == '\0')
        return NULL;

    while (passableChParam(*uStr) && uStr < uStr + *len)
    {
        uStr++;
        (*len)--;
    }
    return uStr;
}

static const char *juemNotSpace(const char *uStr, size_t *len)
{
    if (!uStr || !len || *len == 0U || *uStr == '\0')
        return NULL;

    while (!passableChParam(*uStr) && uStr < uStr + *len)
    {
        uStr++;
        (*len)--;
    }
    return uStr;
}

#define DEBUG_PARSE_SPACE_CNT 0
// 该函数的table为非安全实现, 表的大小需等于大于参数个数
static size_t ParseSpaceCnt(const char *userParam, size_t remaining, userString *table)
{
    const char *str = userParam, *str2 = NULL;
    size_t paramCnt = 0U, paramSize = 0U;
    int tabIdx = 0;

    do
    {
        str = jumpSpace(str, &remaining);
        if (!str)
            break;
#if DEBUG_PARSE_SPACE_CNT
        VAR_PRINT_HEX((uintptr_t)str);
        VAR_PRINT_STRING(str);
#endif
        paramCnt++;
        if (table)
            table[tabIdx].strHead = (void*)str;
#if DEBUG_PARSE_SPACE_CNT
        DEBUG_PRINT("");
#endif
        str2 = juemNotSpace(str, &remaining);
        if (!str2)
            break;
        paramSize = str2 - str;
        if (table)
            table[tabIdx].len = paramSize;
#if DEBUG_PARSE_SPACE_CNT
        VAR_PRINT_UD(paramSize);
#endif
        str = str2;
        tabIdx++;
#if DEBUG_PARSE_SPACE_CNT
        VAR_PRINT_HEX((uintptr_t)str);
        VAR_PRINT_STRING(str);
        printf("\n");
#endif
    } while (1);

    return paramCnt;
}

void *ParseSpace(const char *userParam)
{
    if (!userParam || userData) // 参数为空或者表格尚未释放
        return NULL;

    userDataCnt = ParseSpaceCnt(userParam, PARSE_SIZE, NULL);    // 此时尚未申请用户参数表格, 因为先要计算用户参数个数
    userData = cmd_MemoryAlloc(sizeof(*userData) * userDataCnt); // 一次申请, 避免使用realloc
    if (!userData)
        goto _end;

    if (ParseSpaceCnt(userParam, PARSE_SIZE, userData) != userDataCnt)
    { // 缓冲区的数据可能被修改了
        cmd_MemoryFree(userData);
        RESET_USERDATA_RECORD();
        goto _end;
    }

_end:
    return userData;
}

#if ENABLE_WCHAR
#ifdef WCHAR_MIN
#ifdef WCHAR_MAX

/**
 * @brief 可解析的字符( 参数用, 宽字符 )
 * @param ch 当前地址的字符
 * @return OK: 0,  ERROR: 1
 */
static inline int passableChParamW(const wchar_t ch)
{
    const wchar_t NoSupportChar = L' ';
    return (ch > NoSupportChar)
               ? 0
               : 1;
}

/**
 * @brief 解析空格中混含的用户参数( 宽字符 )
 * @param userParam 用户参数
 * @return 保存用户参数的首地址
 */
void *ParseSpaceW(const wchar_t *userParam)
{
    const wchar_t space = L' ';
    size_t passLen = 0; // 已经处理的长度
    const size_t passConstant = (PARSE_SIZE * 2U) - (MAX_COMMAND * 2U) -
                                (MAX_PARAMETER * 2U) - userDataPass;
    userString *tmp = NULL;
    const wchar_t *str = userParam, *str2 = NULL;
    if (userParam == NULL)
        return NULL;

    passLen = passConstant;
    do
    {
        // 在指令支持的最大长度内跳过所空格, 先检查是否还有有效字符
        for (; passableChParamW(*str) && str < userParam + passConstant; str++)
            ;
        if (str >= userParam + passConstant)
        {
#if NODE_DEBUG
            printf("--<%s>%d--too far:%lld, strAdd:%p, tooFarAdd:%p\n",
                   __func__, __LINE__, userParam + passConstant - str, str, userParam + passConstant);
#endif
            return userData; // 超出可解析的长度
        }

        userDataCnt++;
        passLen += str - userParam;
        userData = (userString *)realloc(tmp, userDataCnt * sizeof(userString));
#if 0
        if (userData == tmp)
        {
            free(userData);
            return userData = NULL;
        }
#endif // 被注释的原因同上面一样
        if (userData == NULL)
        {
            printf("<%s>alloc memory fail\n", __func__);
            free(tmp);
            return (void *)userData;
        }

        (userData + userDataCnt - 1)->strHead = (void *)str; // 字符串头

        str2 = str;
        for (; *str2 != space &&
               str2 < userParam + passConstant &&
               *str2 != L'\0';
             str2++)
            ; // 略过非空格
        if (str2 > (passConstant + userParam))
        {
            // 超出了限定的长度
            printf("<%s>userParam is too long( %u, %p )...\
 the end string has no end\n",
                   __func__, PARSE_SIZE,
                   (void *)(str2 - (passConstant + userParam)));
            (userData + userDataCnt - 1)->len =
                (size_t)((passConstant + userParam) - str);
            return (void *)userData;
        }

        (userData + userDataCnt - 1)->len = (size_t)(str2 - str);
        str += (userData + userDataCnt - 1)->len;

        tmp = userData;
#if NODE_DEBUG
        printf("<%s>len:%llu\n", __func__, (userData + userDataCnt - 1)->len);
#endif
    } while (str <= userParam + passConstant || *str2 == L'\0');
    return (void *)userData;
}
#endif
#endif
#endif
