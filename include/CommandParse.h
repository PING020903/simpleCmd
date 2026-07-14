#pragma once
#ifndef __COMMAND_PARSE_H__
#define __COMMAND_PARSE_H__

#define CMD_METHOD_NODE 1
#define CMD_METHOD_TABLE 0

#include <stdbool.h>
#include "ll.h"
#include "cmdUserStringParse.h"

#define ENABLE_REG 1




typedef enum {
    CMD_NODE_OK = 0,
    CMD_NODE_ERR_FAIL = -1,
    CMD_NODE_ERR_ARG = -2,
    CMD_NODE_ERR_NOT_FIND = -3,
    CMD_NODE_ERR_NOT_FINDCMD = -4,
    CMD_NODE_ERR_NOT_FINDPARAM = -5,
    CMD_NODE_ERR_MEM = -6,
    CMD_NODE_ERR_NULLNODE = -7,
    CMD_NODE_ERR_NULLPARAM = -8,
    CMD_NODE_ERR_REPEATING = -9,
    CMD_NODE_ERR_TOOLONG = -10,
    CMD_NODE_ERR_PARAM_TOOLONG = -11,
    CMD_NODE_ERR_PARSE = -12,
    CMD_NODE_ERR_NOTINIT = -13,
    CMD_NODE_ERR_UNSUPPORT = -14,
    CMD_NODE_ERR_NOHANDLER = -15,
} cmd_err_t;

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


typedef void (*handler_fn_t)(void *arg); // 命令参数处理
typedef struct {
    const handler_fn_t *handlers; // 函数表
    const size_t *handlerCnt;     // 函数表长度
    size_t handlerIdx;           // 当前处理函数索引
} ParameterHandlers; // 数据处理方式表

#if CMD_METHOD_NODE
typedef struct command_node {
    ll_t node;           // 第三方链表节点
    bool isWch : 1;      // 该命令是否使用宽字符
    wchar_t command_string[MAX_COMMAND];// 命令字符串
    ll_t param_head;     // 该命令下的参数链表头
}command_node_t;// 命令节点


// 参数的字符继承命令是否使用宽字符
typedef struct param_node {
    ll_t node;           // 第三方链表节点
    bool isRawStr : 1;   // 传递参数的形式是否为原字符串
    wchar_t parameter_string[MAX_PARAMETER];// 参数字符串
    void* handlerArg;    // handler 的参数
    handler_fn_t handler;// 参数处理
}parameter_node_t;// 参数节点

typedef struct {
    void* command;// 命令名称
    void* node;// 节点地址
}command_info_t;// 命令节点信息
#endif

#if CMD_METHOD_TABLE
typedef struct {
    void* next;
    unsigned int command;
    unsigned int parameter;
    handler_fn_t handler;
} cmdHash_node_t;
#endif

/**
 * @brief 获得解析到的用户参数个数
 * @return
 */
#define simpleCmd_GetUserParamsCnt() userParse_GetUserParamCnt()


#if CMD_METHOD_NODE
void cmdNode_showParam(command_node_t* CmdNode);

void cmdNode_showList(void);

command_node_t* cmdNode_FindCommand(const char* command, const wchar_t* commandW);

int cmdNode_RegisterCommand(const bool isWch, const void* cmdStr);


int cmdNode_unRegisterAllParameters(command_node_t* node);

int cmdNode_unRegisterCommand(const char* command, const wchar_t* commandW);

int cmdNode_updateCommand(char* oldCommand, wchar_t* oldCommandW,
    char* newCommand, wchar_t* newCommandW);

int cmdNode_unRegisterAllCommand(void);

int cmdNode_RegisterParameter(command_node_t* node,
    handler_fn_t hook,
    const bool isRaw,
    const void* paramStr);

int cmdNode_unRegisterParameter(command_node_t* node,
    const void* paramStr);

int cmdNode_updateParameter(const command_node_t* CmdNode, handler_fn_t hook,
    const bool isRaw, const void* oldParam, const void* newParam);

int cmdNode_CommandParse(const char* commandString);

#if ENABLE_WCHAR
#ifdef WCHAR_MIN
#ifdef WCHAR_MAX
int cmdNode_CommandParseW(const wchar_t* commandString);

#endif // WCHAR_MAX
#endif // WCHAR_MIN
#endif

cmd_err_t cmdNode_GetLastError(void);

#if ENABLE_REG

int NodeGetCommandMap(command_info_t** map);

int defaultRegCmd_init(void);
#endif // ENABLE_REG
#endif // CMD_METHOD_NODE

#if CMD_METHOD_TABLE

unsigned int cmdTable_CmdToHash(const char* string, int len);

int cmdTable_RegisterCMD(void* cmd, int cmd_len,
    void* param, int param_len, handler_fn_t handler);

int cmdTable_CommandParse(const char* commandString);

int cmdTable_updataCMDarg(cmdHash_node_t* _old, cmdHash_node_t* _new);

int cmdTable_resetTable(void);

cmd_err_t cmdTable_GetLastError(void);
#endif

#endif  // __COMMAND_PARSE_H__