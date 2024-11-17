#ifndef __COMMAND_PARSE_H__
#define __COMMAND_PARSE_H__


#include <stdbool.h>

#define ERR_CHECK(err) do{\
                          if(err){\
                                  printf("err=%llu, file:%s, line:%d",\
                                         (unsigned long long)err, __FILE__, __LINE__);}\
                          }while(0)




#define NODE_OK 0
#define NODE_FAIL -1
#define NODE_ARG_ERR (NODE_OK + 1) // 函数传递参数错误
#define NODE_NOT_FIND (NODE_OK + 2)
#define NODE_NOT_FIND_CMD (NODE_OK + 3) // 未找到命令节点
#define NODE_NOT_FIND_PARAM (NODE_OK + 4) // 未找到参数节点
#define NODE_ALLOC_ERR (NODE_OK + 5) // 内存申请失败
#define NODE_CMD_NODE_NULL (NODE_OK + 6) // 命令节点为空
#define NODE_PARAM_NODE_NULL (NODE_OK + 7) // 参数节点为空
#define NODE_REPEATING (NODE_OK + 8) // 节点重复
#define NODE_CMD_TOO_LONG (NODE_OK + 9) // 输入命令过长
#define NODE_PARAM_TOO_LONG (NODE_OK + 10) // 命令过长
#define NODE_PARSE_ERR (NODE_OK + 11) // 字符串解析错误
#define NODE_NOT_YET_INIT (NODE_OK + 12) // 节点尚未初始化

#define MAX_COMMAND 16
#define MAX_PARAMETER 16
#define COMMAND_SIZE 128 // 需注意, 要与缓冲区长度一致, 不然会读取到脏字符

#define GET_RAW_STRING 0 // 是否获得原始字符串


#ifndef WCHAR_MIN
#ifndef WCHAR_MAX
#include "wchar.h"
#endif
#endif


typedef void(*ParameterHandler)(void* arg);// 命令参数处理

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

typedef struct {
    void* strHead;
    size_t len;
}userString;// 用户字符串

void showParam(command_node* CmdNode);

void showList(void);

command_node* FindCommand(const char* command, const wchar_t* commandW);

int RegisterCommand(const bool isWch, const void* cmdStr);


int unRegisterAllParameters(command_node* node);

int unRegisterCommand(const char* command, const wchar_t* commandW);

int updateCommand(char* oldCommand, wchar_t* oldCommandW,
                  char* newCommand, wchar_t* newCommandW);

int unRegisterAllCommand(void);

int RegisterParameter(command_node* node,
                      ParameterHandler hook,
                      const bool isRaw,
                      const void* paramStr);

int unRegisterParameter(command_node* node,
                        const void* paramStr);

int updateParameter(const command_node* CmdNode, ParameterHandler hook,
                    const bool isRaw, const void* oldParam, const void* newParam);

int NodeGetCommandMap(command_info** map);

size_t NodeGetUserParamsCnt();

int CommandParse(const char* commandString);

#ifdef WCHAR_MIN
#ifdef WCHAR_MAX
int CommandParseW(const wchar_t* commandString);

#endif // WCHAR_MAX
#endif // WCHAR_MIN

int NodeGetLastError(void);

int defaultRegCmd_init(void);

#endif  // __COMMAND_PARSE_H__