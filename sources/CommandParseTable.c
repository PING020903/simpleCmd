#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "DBG_macro.h"
#include "CommandParse.h"


#if CMD_METHOD_TABLE
#define HASH_TABLE_DEBUG 0

static cmdHash_node_t hashList_static[MAX_HASH_LIST] = { 0 };
static int hashListEnd = CMDHASH_INVALID_INDEX;
static cmd_err_t lastError = CMD_NODE_OK;


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
    void* param, int param_len, handler_fn_t handler) {
    unsigned int cmdHash, paramHash;
    int nextIndex;
    unsigned char cmdArr[MAX_COMMAND], paramArr[MAX_PARAMETER];
    if (!cmd || !param || !handler)
        return lastError = CMD_NODE_ERR_ARG;

    nextIndex = hashListEnd + 1;
    if (nextIndex >= MAX_HASH_LIST)
        return lastError = CMD_NODE_ERR_FAIL;
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
    return lastError = CMD_NODE_OK;
}


/**
 * @brief 命令解析
 * @param commandString 用户输入的字符串
 * @return OK: CMD_NODE_OK
 * @return ERROR: CMD_NODE_ERR_ARG, CMD_NODE_ERR_TOOLONG, CMD_NODE_ERR_PARAM_TOOLONG,
 * CMD_NODE_ERR_NOT_FINDCMD, CMD_NODE_ERR_NOT_FINDPARAM, CMD_NODE_ERR_PARSE
 */
int cmdTable_CommandParse(const char* commandString) {

    int targetIndex = CMDHASH_INVALID_INDEX;
    unsigned int cmdHash, paramHash;
    userString* userData = NULL;
    if (commandString == NULL){
        ERROR_PRINT("");
        return lastError = CMD_NODE_ERR_ARG;
    }
        

    if (hashListEnd == CMDHASH_INVALID_INDEX) {
        DEBUG_PRINT("no registration command...");
        return CMD_NODE_ERR_NOTINIT;
    }

    ParseSpace(commandString);
    if (userParse_GetUserParamCnt() < 2) {
        ERROR_PRINT("2");
        lastError = CMD_NODE_ERR_ARG;
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
        lastError = CMD_NODE_ERR_NOT_FIND;
        goto _err;
    }
#if HASH_TABLE_DEBUG
    DEBUG_PRINT("[%s](%d)", VAR_NAME(userParse_GetUserParamCnt()), userParse_GetUserParamCnt());
#endif
    (userParse_GetUserParamCnt() > 2)
        ? hashList_static[targetIndex].handler(&userData[2])
        : hashList_static[targetIndex].handler(NULL);
    lastError = CMD_NODE_OK;

_err:
    RESET_USERDATA_RECORD();
    return lastError;
}

int cmdTable_updataCMDarg(cmdHash_node_t* _old, cmdHash_node_t* _new)
{
    if (!_old || !_new)
    {
        lastError = CMD_NODE_ERR_ARG;
        return lastError;
    }

    if (hashListEnd == CMDHASH_INVALID_INDEX)
    {
        lastError = CMD_NODE_ERR_NOTINIT;
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
    lastError = CMD_NODE_OK;
    return lastError;
}

int cmdTable_resetTable(void)
{
    lastError = CMD_NODE_OK;
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
cmd_err_t cmdTable_GetLastError(void)
{
    switch (lastError)
    {
    case CMD_NODE_OK:
        DEBUG_PRINT("hashTable OK\n");
        break;
    case CMD_NODE_ERR_FAIL:
        DEBUG_PRINT("hashTable unknow error\n");
        break;
    case CMD_NODE_ERR_ARG:
        DEBUG_PRINT("hashTable parameter passing error\n");
        break;
    case CMD_NODE_ERR_NOT_FIND:
        DEBUG_PRINT("hashTable not find\n");
        break;
    case CMD_NODE_ERR_NOT_FINDCMD:
        DEBUG_PRINT("hashTable not found command\n");
        break;
    case CMD_NODE_ERR_NOT_FINDPARAM:
        DEBUG_PRINT("hashTable not found parameter\n");
        break;
    case CMD_NODE_ERR_MEM:
        DEBUG_PRINT("hashTable Alloc error, maybe RAM is not enough...\n");
        break;
    case CMD_NODE_ERR_NULLNODE:
        DEBUG_PRINT("hashTable this command is null\n");
        break;
    case CMD_NODE_ERR_NULLPARAM:
        DEBUG_PRINT("hashTable command has no paramNode\n");
        break;
    case CMD_NODE_ERR_REPEATING:
        DEBUG_PRINT("hashTable is repeating create\n");
        break;
    case CMD_NODE_ERR_TOOLONG:
        DEBUG_PRINT("hashTable 'command' is too long\n");
        break;
    case CMD_NODE_ERR_PARAM_TOOLONG:
        DEBUG_PRINT("hashTable 'parameter' is too long\n");
        break;
    case CMD_NODE_ERR_PARSE:
        DEBUG_PRINT("hashTable parsing string error\n");
        break;
    case CMD_NODE_ERR_NOTINIT:
        DEBUG_PRINT("table have not been initialized...\n");
        break;
    default:
        ERROR_PRINT("unknow error...");
        break;
    }
    return lastError;
}
#endif // CMD_METHOD_TABLE

