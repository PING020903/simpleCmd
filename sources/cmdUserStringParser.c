#include <string.h>
#include "cmdUserStringParse.h"

/**
 * @brief 用户的字符串
 */
static userString *userData = NULL;

/**
 * @brief 用户参数的数量
 */
static int userDataCnt = 0;

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

#define DEBUG_PARSE_SPACE_CNT 0

/**
 * @brief 扫描下一个 token，支持双引号括起的 raw 数据
 *
 * 规则：
 *   - 跳过前导空格
 *   - 遇到 " 进入引号模式：" 内空格保留，到下一个 "（或行尾）结束
 *   - 普通模式：空格或 " 作为 token 边界
 *   - 未闭合引号：从 " 到行尾作为一个 token
 *
 * @param str       [in/out] 当前扫描位置，调用后指向 token 之后的第一个字符
 * @param remaining [in/out] 剩余字符数
 * @param outHead   [out] token 内容起始地址（跳过前导引号）
 * @param outLen    [out] token 内容长度（不含引号）
 * @return 1=成功提取 token, 0=无更多 token
 */
static int scanNextToken(const char **str, size_t *remaining,
                         const char **outHead, size_t *outLen)
{
    const char *s = *str;
    size_t rem = *remaining;

    /* 跳过前导空格 */
    while (rem > 0 && *s == ' ') {
        s++;
        rem--;
    }
    if (rem == 0 || *s == '\0') {
        return 0;
    }

    if (*s == '"') {
        /* 引号模式：跳过开引号，内容直到闭引号或行尾 */
        s++; rem--;
        *outHead = s;

        const char *end = s;
        size_t endRem = rem;
        while (endRem > 0 && *end != '"' && *end != '\0') {
            end++;
            endRem--;
        }
        *outLen = (size_t)(end - s);

        if (endRem > 0 && *end == '"') {
            end++; endRem--;   /* 跳过闭引号 */
        }
        /* 未闭合引号：end 已指向 '\0'，token 内容到行尾 */

        *str = end;
        *remaining = endRem;
    } else {
        /* 普通模式：扫描到空格、引号或行尾 */
        *outHead = s;

        const char *end = s;
        size_t endRem = rem;
        while (endRem > 0 && *end != ' ' && *end != '"' && *end != '\0') {
            end++;
            endRem--;
        }
        *outLen = (size_t)(end - s);

        *str = end;
        *remaining = endRem;
    }

    return 1;
}

/**
 * @brief 分词计数 + 填表（两趟复用同一逻辑）
 * @param userParam 输入字符串
 * @param remaining 字符串有效长度
 * @param table     输出表；NULL 表示仅计数不填表
 * @return token 数量
 */
static size_t ParseSpaceCnt(const char *userParam, size_t remaining, userString *table)
{
    const char *str = userParam;
    size_t rem = remaining;
    size_t paramCnt = 0U;
    int tabIdx = 0;

    while (1) {
        const char *head = NULL;
        size_t len = 0;

        if (!scanNextToken(&str, &rem, &head, &len))
            break;

        paramCnt++;
        if (table) {
            table[tabIdx].strHead = (void *)head;
            table[tabIdx].len = len;
        }
        tabIdx++;
    }

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

/**
 * @brief 扫描下一个宽字符 token，支持双引号括起的 raw 数据
 *
 * 与 scanNextToken 逻辑完全一致，仅类型从 char 换为 wchar_t。
 */
static int scanNextTokenW(const wchar_t **str, size_t *remaining,
                          const wchar_t **outHead, size_t *outLen)
{
    const wchar_t *s = *str;
    size_t rem = *remaining;

    /* 跳过前导空格 */
    while (rem > 0 && *s == L' ') {
        s++;
        rem--;
    }
    if (rem == 0 || *s == L'\0') {
        return 0;
    }

    if (*s == L'"') {
        /* 引号模式：跳过开引号，内容直到闭引号或行尾 */
        s++; rem--;
        *outHead = s;

        const wchar_t *end = s;
        size_t endRem = rem;
        while (endRem > 0 && *end != L'"' && *end != L'\0') {
            end++;
            endRem--;
        }
        *outLen = (size_t)(end - s);

        if (endRem > 0 && *end == L'"') {
            end++; endRem--;
        }

        *str = end;
        *remaining = endRem;
    } else {
        /* 普通模式：扫描到空格、引号或行尾 */
        *outHead = s;

        const wchar_t *end = s;
        size_t endRem = rem;
        while (endRem > 0 && *end != L' ' && *end != L'"' && *end != L'\0') {
            end++;
            endRem--;
        }
        *outLen = (size_t)(end - s);

        *str = end;
        *remaining = endRem;
    }

    return 1;
}

/**
 * @brief 宽字符分词计数 + 填表
 */
static size_t ParseSpaceCntW(const wchar_t *userParam, size_t remaining, userString *table)
{
    const wchar_t *str = userParam;
    size_t rem = remaining;
    size_t paramCnt = 0U;
    int tabIdx = 0;

    while (1) {
        const wchar_t *head = NULL;
        size_t len = 0;

        if (!scanNextTokenW(&str, &rem, &head, &len))
            break;

        paramCnt++;
        if (table) {
            table[tabIdx].strHead = (void *)head;
            table[tabIdx].len = len;
        }
        tabIdx++;
    }

    return paramCnt;
}

/**
 * @brief 宽字符版 ParseSpace，支持双引号 raw 数据
 */
void *ParseSpaceW(const wchar_t *userParam)
{
    if (!userParam || userData)
        return NULL;

    /* PARSE_SIZE 为字符数，宽字符同样适用 */
    userDataCnt = ParseSpaceCntW(userParam, PARSE_SIZE, NULL);
    userData = cmd_MemoryAlloc(sizeof(*userData) * userDataCnt);
    if (!userData)
        goto _end;

    if (ParseSpaceCntW(userParam, PARSE_SIZE, userData) != userDataCnt) {
        cmd_MemoryFree(userData);
        RESET_USERDATA_RECORD();
        goto _end;
    }

_end:
    return userData;
}

#endif /* ENABLE_WCHAR */
