#pragma once
#ifndef _CMDUSERSTRINGPARSER_H_
#define _CMDUSERSTRINGPARSER_H_

#define ENABLE_WCHAR 0


#define PARSE_SIZE 128 // 需注意, 要与缓冲区长度一致, 不然会读取到脏字符

typedef struct {
    void* strHead;
    size_t len;
}userString;// 用户字符串

void RESET_USERDATA_RECORD(void);

int userParse_GetUserParamCnt(void);

userString* userParse_pUserData(void);

void* ParseSpace(const char* userParam);

#if ENABLE_WCHAR

void* ParseSpaceW(const wchar_t* userParam);
#endif

#endif // !_CMDUSERSTRINGPARSER_H_
