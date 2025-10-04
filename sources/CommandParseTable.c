#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "DBG_macro.h"
#include "CommandParse.h"

#define HASH_TABLE_DEBUG 0

static cmdHash_node hashList_static[MAX_HASH_LIST] = { 0 };
static int hashListEnd = CMDHASH_INVALID_INDEX;
static int lastError = NODE_OK;


unsigned int cmdTable_CmdToHash(const char* string, int len) {
#define FNV_PRIME 0x01000193         // ✅ 乘数（不是初始值！）
#define FNV_OFFSET 0x811C9DC5        // ✅ 初始种子
#if HASH_TABLE_DEBUG
    char tempstring[PARSE_SIZE] = { 0 };
    memcpy(tempstring, string, len);
    VAR_PRINT_STRING(tempstring);
#endif
    unsigned int hash = FNV_OFFSET;  // ✅ 必须初始化！
    while (len) {
        hash = (hash ^ (*string)) * FNV_PRIME;
        string++;
        len--;
    }
    return hash;
}



int cmdTable_RegisterCMD(void* cmd, int cmd_len,
    void* param, int param_len, ParameterHandler handler) {
    unsigned int cmdHash, paramHash;
    int nextIndex;
    unsigned char cmdArr[MAX_COMMAND], paramArr[MAX_PARAMETER];
    if (!cmd || !param || !handler)
        return lastError = NODE_ARG_ERR;

    nextIndex = hashListEnd + 1;
    if (nextIndex >= MAX_HASH_LIST)
        return lastError = NODE_FAIL;
    cmdHash = cmdTable_CmdToHash(cmd, cmd_len);
    paramHash = cmdTable_CmdToHash(param, param_len);
    hashList_static[nextIndex].command = cmdHash;
    hashList_static[nextIndex].parameter = paramHash;
    hashList_static[nextIndex].handler = handler;
    hashList_static[nextIndex].next = &hashList_static[nextIndex + 1];


    memset(cmdArr, 0, MAX_COMMAND);
    memset(paramArr, 0, MAX_PARAMETER);
    memcpy(cmdArr, cmd, cmd_len);
    memcpy(paramArr, param, param_len);
    hashListEnd = nextIndex;
#if 0
    DEBUG_PRINT("cmd{%s}[%x] param{%s}[%x]", cmdArr, cmdHash, paramArr, paramHash);
    VAR_PRINT_HEX(hashListEnd);
#endif
    return lastError = NODE_OK;
}


/**
 * @brief 命令解析
 * @param commandString 用户输入的字符串
 * @return OK: NODE_OK
 * @return ERROR: NODE_ARG_ERR, NODE_CMD_TOO_LONG, NODE_PARAM_TOO_LONG,
 * NODE_NOT_FIND_CMD, NODE_NOT_FIND_PARAM, NODE_PARSE_ERR
 */
int cmdTable_CommandParse(const char* commandString) {

    int targetIndex = CMDHASH_INVALID_INDEX;
    unsigned int cmdHash, paramHash;
    userString* userData = NULL;
    if (commandString == NULL)
        return lastError = NODE_ARG_ERR;

    if (hashListEnd == CMDHASH_INVALID_INDEX) {
        DEBUG_PRINT("no registration command...");
        return NODE_NOT_YET_INIT;
    }

    ParseSpace(commandString);
    if (userParse_GetUserParamCnt() < 2) {
        lastError = NODE_ARG_ERR;
        goto _err;
    }

    userData = userParse_pUserData();
    if (userData == NULL)
        goto _err;
    cmdHash = cmdTable_CmdToHash(userData[0].strHead, userData[0].len);
    paramHash = cmdTable_CmdToHash(userData[1].strHead, userData[1].len);
    for (targetIndex = 0; targetIndex < hashListEnd + 1; targetIndex++) {
        if (hashList_static[targetIndex].command == cmdHash &&
            hashList_static[targetIndex].parameter == paramHash)
            break;
    }
    if (targetIndex > hashListEnd) {
        DEBUG_PRINT("not found target command.");
        lastError = NODE_NOT_FIND;
        goto _err;
    }
#if HASH_TABLE_DEBUG
    DEBUG_PRINT("[%s](%d)", VAR_NAME(userParse_GetUserParamCnt()), userParse_GetUserParamCnt());
#endif
    (userParse_GetUserParamCnt() > 2)
        ? hashList_static[targetIndex].handler(&userData[2])
        : hashList_static[targetIndex].handler(NULL);
    lastError = NODE_OK;

_err:
    RESET_USERDATA_RECORD();
    return lastError;
}

int cmdTable_updataCMDarg(cmdHash_node* _old, cmdHash_node* _new)
{
    if (!_old || !_new)
    {
        lastError = NODE_ARG_ERR;
        return lastError;
    }

    if (hashListEnd == CMDHASH_INVALID_INDEX)
    {
        lastError = NODE_NOT_YET_INIT;
        return lastError;
    }
    for (int i = 0; i < (hashListEnd + 1); i++)
    {
        if (_old->command == hashList_static[i].command &&
            _old->parameter == hashList_static[i].parameter)
        {
            hashList_static[i].command = _new->command;
            hashList_static[i].parameter = _new->parameter;
            if (_new->handler != NULL)
                hashList_static[i].handler = _new->handler;
            break;
        }
    }
    lastError = NODE_OK;
    return lastError;
}

int cmdTable_resetTable(void)
{
    lastError = NODE_OK;
    if (hashListEnd == CMDHASH_INVALID_INDEX)
        return lastError;

    for (int i = 0; i < (hashListEnd + 1); i++)
    {
        hashList_static[i].command = 0U;
        hashList_static[i].parameter = 0U;
        hashList_static[i].handler = NULL;
        hashList_static[i].next = NULL;
    }
    DEBUG_PRINT("reset hash table success.");
    return lastError;
}

/**
 * @brief 获得上次运行链表管理函数的错误
 * @param
 * @return lastError
 */
int cmdTable_GetLastError(void)
{
    switch (lastError)
    {
    case NODE_OK:
        DEBUG_PRINT("hashTable OK\n");
        break;
    case NODE_FAIL:
        DEBUG_PRINT("hashTable unknow error\n");
        break;
    case NODE_ARG_ERR:
        DEBUG_PRINT("hashTable parameter passing error\n");
        break;
    case NODE_NOT_FIND:
        DEBUG_PRINT("hashTable not find\n");
        break;
    case NODE_NOT_FIND_CMD:
        DEBUG_PRINT("hashTable not found command\n");
        break;
    case NODE_NOT_FIND_PARAM:
        DEBUG_PRINT("hashTable not found parameter\n");
        break;
    case NODE_ALLOC_ERR:
        DEBUG_PRINT("hashTable Alloc error, maybe RAM is not enough...\n");
        break;
    case NODE_CMD_NODE_NULL:
        DEBUG_PRINT("hashTable this command is null\n");
        break;
    case NODE_PARAM_NODE_NULL:
        DEBUG_PRINT("hashTable command has no paramNode\n");
        break;
    case NODE_REPEATING:
        DEBUG_PRINT("hashTable is repeating create\n");
        break;
    case NODE_CMD_TOO_LONG:
        DEBUG_PRINT("hashTable 'command' is too long\n");
        break;
    case NODE_PARAM_TOO_LONG:
        DEBUG_PRINT("hashTable 'parameter' is too long\n");
        break;
    case NODE_PARSE_ERR:
        DEBUG_PRINT("hashTable parsing string error\n");
        break;
    case NODE_NOT_YET_INIT:
        DEBUG_PRINT("table have not been initialized...\n");
        break;
    default:
        ERROR_PRINT("unknow error...");
        break;
    }
    return lastError;
}
