#pragma once
#ifndef _CMDUSERSTRINGPARSER_H_
#define _CMDUSERSTRINGPARSER_H_

#include <stdint.h>
#include <stddef.h>

#include "cmdTreeCfg.h"   // 所有功能开关宏统一在此配置

typedef struct {
    void* strHead;
    size_t len;
}userString;// 用户字符串

// 做外部实现的内存申请&释放的函数接口声明, 可以让用户自行实现内存该如何申请&释放
extern void *cmd_MemoryAlloc(size_t bytes);
extern void cmd_MemoryFree(void *mem);

void RESET_USERDATA_RECORD(void);

int userParse_GetUserParamCnt(void);

userString* userParse_pUserData(void);

void* ParseSpace(const char* userParam);

#if ENABLE_WCHAR

#include <wchar.h>
void* ParseSpaceW(const wchar_t* userParam);
#endif

#endif // !_CMDUSERSTRINGPARSER_H_
