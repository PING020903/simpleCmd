#pragma once
#ifndef __COMMAND_PARSE_H__
#define __COMMAND_PARSE_H__

#define CMD_METHOD_NODE 1
#define CMD_METHOD_TABLE 1

#include <stdbool.h>
#include "cmdUserStringParse.h"

#define ENABLE_REG 1




#define NODE_OK 0
#define NODE_FAIL -1
#define NODE_ARG_ERR (NODE_OK - 2)          // 函数传递参数错误
#define NODE_NOT_FIND (NODE_OK - 3)
#define NODE_NOT_FIND_CMD (NODE_OK - 4)     // 未找到命令节点
#define NODE_NOT_FIND_PARAM (NODE_OK - 5)   // 未找到参数节点
#define NODE_ALLOC_ERR (NODE_OK - 6)        // 内存申请失败
#define NODE_CMD_NODE_NULL (NODE_OK - 7)    // 命令节点为空
#define NODE_PARAM_NODE_NULL (NODE_OK - 8)  // 参数节点为空
#define NODE_REPEATING (NODE_OK - 9)        // 节点重复
#define NODE_CMD_TOO_LONG (NODE_OK - 10)    // 输入命令过长
#define NODE_PARAM_TOO_LONG (NODE_OK - 11)  // 命令过长
#define NODE_PARSE_ERR (NODE_OK - 12)       // 字符串解析错误
#define NODE_NOT_YET_INIT (NODE_OK - 13)    // 节点尚未初始化
#define NODE_UNSUPPORT (NODE_OK -14)        // 暂不支持该操作
#define NODE_NO_HANDLER (NODE_OK - 15)

#define CMDHASH_INVALID_INDEX -1

#define MAX_COMMAND 16
#define MAX_PARAMETER 16
#define COMMAND_SIZE PARSE_SIZE // 需注意, 要与缓冲区长度一致, 不然会读取到脏字符
#define MAX_HASH_LIST 16

#define GET_RAW_STRING 0 // 是否获得原始字符串


#ifndef WCHAR_MIN
#ifndef WCHAR_MAX
#include "wchar.h"
#endif
#endif


typedef void(*ParameterHandler)(void* arg);// 命令参数处理

#if CMD_METHOD_NODE
typedef struct {
    void* prev;// 上一节点
    void* next;// 下一节点
    bool isWch : 1; // 该命令是否使用宽字符
    wchar_t command_string[MAX_COMMAND];// 命令字符串
    void* ParameterNode_head;// 该命令下的参数
}command_node;// 命令节点


// 参数的字符继承命令是否使用宽字符
typedef struct {
    void* prev;// 上一节点
    void* next;// 下一节点
    bool isRawStr : 1;// 传递参数的形式是否为原字符串
    wchar_t parameter_string[MAX_PARAMETER];// 参数字符串
    void* handlerArg;// handler 的参数
    ParameterHandler handler;// 参数处理
}parameter_node;// 参数节点

typedef struct {
    void* command;// 命令名称
    void* node;// 节点地址
}command_info;// 命令节点信息
#endif

#if CMD_METHOD_TABLE
typedef struct {
    void* next;
    unsigned int command;
    unsigned int parameter;
    ParameterHandler handler;
} cmdHash_node;
#endif

/**
 * @brief 获得解析到的用户参数个数
 * @return
 */
#define simpleCmd_GetUserParamsCnt() userParse_GetUserParamCnt()


#if CMD_METHOD_NODE
void cmdNode_showParam(command_node* CmdNode);

void cmdNode_showList(void);

command_node* cmdNode_FindCommand(const char* command, const wchar_t* commandW);

int cmdNode_RegisterCommand(const bool isWch, const void* cmdStr);


int cmdNode_unRegisterAllParameters(command_node* node);

int cmdNode_unRegisterCommand(const char* command, const wchar_t* commandW);

int cmdNode_updateCommand(char* oldCommand, wchar_t* oldCommandW,
    char* newCommand, wchar_t* newCommandW);

int cmdNode_unRegisterAllCommand(void);

int cmdNode_RegisterParameter(command_node* node,
    ParameterHandler hook,
    const bool isRaw,
    const void* paramStr);

int cmdNode_unRegisterParameter(command_node* node,
    const void* paramStr);

int cmdNode_updateParameter(const command_node* CmdNode, ParameterHandler hook,
    const bool isRaw, const void* oldParam, const void* newParam);

int cmdNode_CommandParse(const char* commandString);

#if ENABLE_WCHAR
#ifdef WCHAR_MIN
#ifdef WCHAR_MAX
int cmdNode_CommandParseW(const wchar_t* commandString);

#endif // WCHAR_MAX
#endif // WCHAR_MIN
#endif

int cmdNode_GetLastError(void);

#if ENABLE_REG

int NodeGetCommandMap(command_info** map);

int defaultRegCmd_init(void);
#endif // ENABLE_REG
#endif // CMD_METHOD_NODE

#if CMD_METHOD_TABLE

unsigned int cmdTable_CmdToHash(const char* string, int len);

int cmdTable_RegisterCMD(void* cmd, int cmd_len,
    void* param, int param_len, ParameterHandler handler);

int cmdTable_CommandParse(const char* commandString);

int cmdTable_updataCMDarg(cmdHash_node* _old, cmdHash_node* _new);

int cmdTable_resetTable(void);

int cmdTable_GetLastError(void);
#endif

#endif  // __COMMAND_PARSE_H__