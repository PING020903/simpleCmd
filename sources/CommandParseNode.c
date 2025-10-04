#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <stdlib.h>
#include "DBG_macro.h"
#include "CommandParse.h"

#if CMD_METHOD_NODE

#define NODE_DEBUG 0

#define DEFAULT_CMD "reg"
#define DEFAULT_PARAM_cmd "cmd"
#define DEFAULT_PARAM_param "param"
#define DEFAULT_PARAM_DelCmd "DelCmd"
#define DEFUALT_PARAM_DelAllCmd "DelAllCmd"
#define DEFUALT_PARAM_DelAllParam "DelAllParam"
#define DEFUALT_PARAM_DelParam "DelParam"
#define DEFUALT_PARAM_ls "ls"

#define DEFAULT_CMD_W L"reg"

/**
 * @brief 初始节点
 */
static command_node* FristNode = NULL;
static const char* successStr = "Successful registration.";
static const wchar_t* successStrW = L"(isWch) Successful registration.";
/**
 * @brief 上一次函数运行的错误
 */
static int lastError = NODE_OK;



#define ERR_CHECK(err) do{\
                          if(err){\
                                  printf("err=%llu, file:%s, line:%d",\
                                         (unsigned long long)err, __FILE__, __LINE__);}\
                          }while(0)

#if NODE_DEBUG
static command_node* compare1 = NULL, * compare2 = NULL;
static parameter_node* compare3 = NULL, * compare4 = NULL;
#endif

#define CHECK_BUF(buf) do{\
if(buf==NULL){\
free(buf);\
return lastError = NODE_ALLOC_ERR;\
}}while(0)

#if NODE_DEBUG
/**
 * @brief 打印指定节点的左右临近节点命令名
 * @param CmdNode 命令节点
 * @return OK： NODE_OK
 * @return ERROR: NODE_ARG_ERR
 */
static int printCmdNode_command(command_node* CmdNode)
{
    command_node* NEXT = NULL, * PREV = NULL;
    if (CmdNode == NULL)
        return NODE_ARG_ERR;

    NEXT = CmdNode->next;
    PREV = CmdNode->prev;



    (PREV->isWch)
        ? wprintf(L" prev:<%ls>,", PREV->command_string)
        : printf(" prev:<%s>,", (char*)(PREV->command_string));

    (CmdNode->isWch)
        ? wprintf(L" this:<%ls>,", CmdNode->command_string)
        : printf(" this:<%s>,", (char*)(CmdNode->command_string));

    (NEXT->isWch)
        ? wprintf(L" next:<%ls>\n", NEXT->command_string)
        : printf(" next:<%s>\n", (char*)(NEXT->command_string));

    return NODE_OK;
}
#endif

/**
 * @brief 可解析的字符( 命令用 )
 * @param ch 当前地址的字符
 * @return OK: 0,  ERROR: 1
 */
static int passableChCmd(const char ch)
{
    const char NoSupportChar = ' ';
    return (ch > NoSupportChar)
        ? 0
        : 1;
}

/**
 * @brief 可解析的字符( 参数用 )
 * @param ch 当前地址的字符
 * @return OK: 0,  ERROR: 1
 */
static int passableChParam(const char ch)
{
    const char NoSupportChar = ' ';
    return (ch > NoSupportChar)
        ? 0
        : 1;
}

/// @brief 命令字符解析器
/// @param str 字符串
/// @return character
/// @note 将大写字符转小写, 其余直接返回
static char UserCharacterParse(const char* str)
{
    const unsigned char constant = 'a' - 'A';
    if ('A' <= *str && *str <= 'Z')
        return (*str) + constant;

    return (str)
        ? *str
        : '\0';
}

/**
 * @brief 大写转小写
 * @param str 目标字符串
 * @return OK: NODE_OK
 * @return ERROR: NODE_ARG_ERR
 */
static int UppercaseToLowercase(char* str)
{
    size_t len = 0, i = 0;
    if (str == NULL)
        return NODE_ARG_ERR;

    len = strlen(str);

    for (; i < len; i++)
    {
        // 将不规则的大小写输入统一为小写
        *(str + i) = UserCharacterParse(str + i);
    }
    return NODE_OK;
}

#ifdef WCHAR_MIN
#ifdef WCHAR_MAX
/// @brief 命令字符解析器( 宽字符版本 )
/// @param str 字符串
/// @return character
/// @note 将大写字符转小写, 其余直接返回
static wchar_t UserCharacterParseW(const wchar_t* str)
{
    const unsigned short constant = L'a' - L'A';
    if (L'A' <= *str && *str <= L'Z')
        return (*str) + constant;

    return (str)
        ? *str
        : L'\0';
}

/**
 * @brief 大写转小写( 宽字符版本 )
 * @param str
 * @return OK: NODE_OK
 * @return ERROR: NODE_ARG_ERR
 */
static int UppercaseToLowercaseW(wchar_t* str)
{
    size_t len = 0, i = 0;
    if (str == NULL)
        return NODE_ARG_ERR;

    len = wcslen(str);

    for (; i < len; i++)
    {
        // 将不规则的大小写输入统一为小写
        *(str + i) = UserCharacterParseW(str + i);
    }
    return NODE_OK;
}

/**
 * @brief 可解析的字符( 命令用, 宽字符 )
 * @param ch 当前地址的字符
 * @return OK: 0,  ERROR: 1
 */
static int passableChCmdW(const wchar_t ch)
{
    const wchar_t NoSupportChar = L' ';
    return (ch > NoSupportChar)
        ? 0
        : 1;
}

/**
 * @brief 可解析的字符( 参数用, 宽字符 )
 * @param ch 当前地址的字符
 * @return OK: 0,  ERROR: 1
 */
static int passableChParamW(const wchar_t ch)
{
    const wchar_t NoSupportChar = L' ';
    return (ch > NoSupportChar)
        ? 0
        : 1;
}
#endif
#endif

/**
 * @brief 当前命令链表中最后的节点
 * @return target node
 */
static command_node* CommandFinalNode()
{
    command_node* CmdNode = FristNode;
    while (CmdNode->next != FristNode)// 遍历链表
    {
        if (CmdNode != CmdNode->next)
        {
            CmdNode = CmdNode->next;
        }
        else
        {
            break;
        }
    }
    return CmdNode;
}

/**
 * @brief 参数链表中最后的节点
 * @param node
 * @return target node
 */
static parameter_node* ParameterFinalNode(const command_node* node)
{
    parameter_node* paramNode = node->ParameterNode_head;
    if (node == NULL)
        return NULL;
    while (paramNode->next != node->ParameterNode_head)
    {
        paramNode = paramNode->next;
    }
    return paramNode;
}

/**
 * @brief 分配命令节点的字符串
 * @param node 当前命令节点
 * @param isWch 是否使用宽字符
 * @param cmdStr 命令
 * @return OK: NODE_OK
 * @return ERROR: NODE_ARG_ERR, NODE_CMD_TOO_LONG
 */
static int AssignCommandNodeStr(command_node* node,
    const bool isWch,
    const void* cmdStr)
{
    size_t len = 0;
    wchar_t* strW = NULL;
    char* str = NULL, * tmp = NULL;
    if (node == NULL || cmdStr == NULL)
        return lastError = NODE_ARG_ERR;

    node->isWch = isWch;
    switch (isWch)
    {
    case true:
    {
#if (ENABLE_WCHAR == 1)
        strW = (wchar_t*)cmdStr;
        len = wcslen(strW);
        if (len > (MAX_COMMAND - 1))
            return lastError = NODE_CMD_TOO_LONG;

        // 非法字符检查
        for (size_t i = 0; i < len; i++)
        {
            if (passableChCmdW(*(strW + i)))
                return lastError = NODE_ARG_ERR;
        }
        wcscpy(node->command_string, strW);
        UppercaseToLowercaseW(node->command_string);
#else
        lastError = NODE_UNSUPPORT;
#endif
    }
    break;
    default:
    {
        tmp = (char*)(node->command_string);
        str = (char*)cmdStr;
        len = strlen(str);
        if (len > (MAX_COMMAND * sizeof(wchar_t) - 1))
            return lastError = NODE_CMD_TOO_LONG;

        // 非法字符检查
        for (size_t i = 0; i < len; i++)
        {
            if (passableChCmd(*(str + i)))
                return lastError = NODE_ARG_ERR;
        }
        strcpy(tmp, str);
        UppercaseToLowercase(tmp);
    }
    break;
    }
    return lastError = NODE_OK;
}

/**
 * @brief 打印指定命令下所有参数
 * @param CmdNode 命令节点
 */
void cmdNode_showParam(command_node* CmdNode)
{
    parameter_node* node = CmdNode->ParameterNode_head;
    char* cmdstring = NULL;
    char* paramstring = NULL;
    if (CmdNode == NULL)
        return;

    if (node == NULL)
    {
#if (ENABLE_WCHAR == 1)
        (CmdNode->isWch)
            ? wprintf(L"<%ls>this command has no parameters...\n", CmdNode->command_string)
            : printf("<%s>this command has no parameters...\n", (char*)(CmdNode->command_string));
#else
        cmdstring = CmdNode->command_string;
        if (CmdNode->isWch) {
            printf("this command has no parameters...\n");
            return;
        }
        printf("<%s>this command has no parameters...\n", cmdstring);
#endif
        return;
    }

    putchar('\n');
    printf("  parameters:\n");
    do
    {
#if (ENABLE_WCHAR == 1)
        (CmdNode->isWch)
            ? wprintf(L"        %ls\n", node->parameter_string)
            : printf("        %s\n", (char*)(node->parameter_string));
#else
        paramstring = node->parameter_string;
        if (CmdNode->isWch) {
            printf("    (unsupport display)\n");
            goto _nextNode;
        }
        printf("        %s\n", paramstring);
#endif
    _nextNode:
        node = node->next;
    } while (node != CmdNode->ParameterNode_head);
}

/**
 * @brief 打印命令链表
 */
void cmdNode_showList(void)
{
    command_node* node = FristNode;
    char* cmdstring = NULL;
    int cnt = 1;

    if (node == NULL)
    {
        printf("list null...\n");
        return;
    }

    putchar('\n');
    do
    {
#if (ENABLE_WCHAR == 1)
        (node->isWch)
            ? wprintf(L"%llu:command  <%ls>(isWch)     ", cnt++, node->command_string)
            : printf("%llu:command  <%s>     ", cnt++, (char*)(node->command_string));
#else
        cmdstring = node->command_string;
        if (node->isWch) {
            printf("%d:command(isWchar unsupport display) <>", cnt++);
            goto _nextNode;
        }
        printf("%d:command  <%s>     ", cnt++, cmdstring);
#endif
    _nextNode:
        cmdNode_showParam(node);
        node = node->next;
    } while (node != FristNode); // 为了保证遍历完整
    putchar('\n');
}

/**
 * @brief 寻找命令节点
 * @param command 命令, 取决于注册节点时是否使用了宽字符 , 若未使用宽字符, 另一个则填 NULL
 * @param commandW 命令( 宽字符 )
 * @return ERROR: NULL
 * @return OK: target node
 */
command_node* cmdNode_FindCommand(const char* command,
    const wchar_t* commandW)
{
    command_node* CmdNode = FristNode;
    char CmdTmp[MAX_COMMAND * sizeof(wchar_t)] = { 0 };
    wchar_t CmdTmpW[MAX_COMMAND] = { 0 };

    if (CmdNode == NULL)
    {
        lastError = NODE_NOT_YET_INIT;
        return NULL;
    }


    if (command == NULL && commandW == NULL)
    {
        lastError = NODE_ARG_ERR;
        return NULL;
    }


    if (command != NULL)
    {
        if (strlen(command) > (MAX_COMMAND * sizeof(wchar_t) - 1))
        {
            lastError = NODE_CMD_TOO_LONG;
            return NULL;
        }

        strcpy(CmdTmp, command);
        if (UppercaseToLowercase(CmdTmp))// 确保字符统一小写
            return NULL;

        do
        {
            if (strcmp((char*)(CmdNode->command_string), CmdTmp) == 0)
                return CmdNode;

            CmdNode = CmdNode->next;
        } while (CmdNode != FristNode);
    }
#if (ENABLE_WCHAR == 1)
    else if (commandW != NULL)
    {
        if (wcslen(commandW) > (MAX_COMMAND - 1))
        {
            lastError = NODE_CMD_TOO_LONG;
            return NULL;
        }

        wcscpy(CmdTmpW, commandW);
        if (UppercaseToLowercaseW(CmdTmpW))// 确保字符统一小写
            return NULL;

        do
        {
            if (wcscmp(CmdNode->command_string, CmdTmpW) == 0)
                return CmdNode;

            CmdNode = CmdNode->next;
        } while (CmdNode != FristNode);
    }
#else
    else if (commandW != NULL)
    {
        lastError = NODE_UNSUPPORT;
        return NULL;
    }
#endif
    else
    {
        lastError = NODE_FAIL;
        return NULL;
    }
    lastError = NODE_NOT_FIND_CMD;
    return NULL; // 遍历后均无所获
}

/**
 * @brief 注册命令
 * @param isWch 该命令是否使用宽字符
 * @param cmdStr 命令
 * @return OK: NODE_OK
 * @return ERROR: NODE_FAIL, NODE_ARG_ERR, NODE_CMD_TOO_LONG, NODE_REPEATING
 */
int cmdNode_RegisterCommand(const bool isWch,
    const void* cmdStr)
{
    command_node* tmp = NULL, * repeating = NULL;
    command_node* CmdNode = NULL;// 要被初始化的节点
    if (cmdStr == NULL)
    {
        printf("The command is null, exit func:<%s>\n", __func__);
        return lastError = NODE_ARG_ERR;
    }

#if NODE_DEBUG
    (isWch)
        ? wprintf(L"(RegisterCommand)>>isWch--%ls\n", (wchar_t*)cmdStr)
        : printf("(RegisterCommand)>>--%s\n", (char*)cmdStr);
#endif
    if (FristNode == NULL)// 首次注册命令节点
    {
        FristNode = (command_node*)malloc(sizeof(command_node));
        CHECK_BUF(FristNode);
        FristNode->prev = FristNode;
        FristNode->next = FristNode;
        FristNode->ParameterNode_head = NULL;
        if (AssignCommandNodeStr(FristNode, isWch, cmdStr))
        {
            free(FristNode);
            return lastError;
        }
#if NODE_DEBUG
        PRINT_CMD(FristNode);
#endif
        return lastError = NODE_OK;
    }
    else
    {
        CmdNode = (command_node*)malloc(sizeof(command_node));
        CHECK_BUF(CmdNode);
        CmdNode->ParameterNode_head = NULL;
        if (AssignCommandNodeStr(CmdNode, isWch, cmdStr))
        {
            free(CmdNode);
            return lastError;
        }

        // 寻找是否有相同命令的节点, 当前节点尚未接入链表, 故此不会影响寻找
#if (ENABLE_WCHAR == 1)
        repeating = (isWch)
            ? cmdNode_FindCommand(NULL, CmdNode->command_string)
            : cmdNode_FindCommand((char*)(CmdNode->command_string), NULL);
#else
        repeating = cmdNode_FindCommand((char*)(CmdNode->command_string), NULL);
#endif
        if (repeating != NULL)
        {
            printf("The same command already exists...\n");
            free(CmdNode);
            return lastError = NODE_REPEATING;
        }
        tmp = CommandFinalNode();

        // 节点接驳
        CmdNode->prev = tmp;        // this to currentEnd
        tmp->next = CmdNode;        // currentEnd to this
        CmdNode->next = FristNode;  // currentEnd to Frist
        FristNode->prev = CmdNode;  // Frist to currentEnd

#if NODE_DEBUG
        PRINT_CMD(CmdNode);
#endif
        return lastError = NODE_OK;
    }

    return lastError = NODE_FAIL;
}

/**
 * @brief 取消注册单个命令下的所有参数
 * @param node 命令节点
 * @return OK: NODE_OK
 * @return ERROR: NODE_ARG_ERR, NODE_PARAM_NODE_NULL
 */
int cmdNode_unRegisterAllParameters(command_node* node)
{
    parameter_node* tmp = NULL, * nextNode = NULL;
    char* string = NULL;
    if (node == NULL)
        return lastError = NODE_ARG_ERR;
    else
        tmp = node->ParameterNode_head;

    if (tmp == NULL)
    {
#if (ENABLE_WCHAR == 1)
        (node->isWch)
            ? wprintf(L"<%ls>(isWch) ParameterNode is NULL.\n", node->command_string)
            : printf("<%s> ParameterNode is NULL.\n", (char*)(node->command_string));
#else
        string = node->command_string;
        if (node->isWch) {
            printf("<>(wchar is unsupport) ParameterNode is NULL.\n");
            return lastError = NODE_OK;
        }
        printf("<%s> ParameterNode is NULL.\n", string);
#endif
        return lastError = NODE_OK;
    }

    do
    {
        nextNode = tmp->next;// 保存下一节点地址
        if (nextNode == NULL)
            return lastError = NODE_PARAM_NODE_NULL;
        tmp->prev = NULL;
        tmp->next = NULL;
        free(tmp);// 释放当前节点
        tmp = nextNode;// 切换到下一节点
    } while (tmp != node->ParameterNode_head);
    node->ParameterNode_head = NULL; // 此时已经释放完毕

#if (ENABLE_WCHAR == 1)
    (node->isWch)
        ? wprintf(L"<%ls>(isWch) ParameterNodes unregister finish.\n", node->command_string)
        : printf("<%s> ParameterNodes unregister finish.\n", (char*)(node->command_string));
#else
    printf("<%s> ParameterNodes unregister finish.\n", (char*)(node->command_string));
#endif
    return lastError = NODE_OK;
}

/**
 * @brief 寻找参数节点
 * @param node 命令节点
 * @param paramStr 参数
 * @return
 */
static parameter_node* FindParameter(const command_node* node,
    const void* paramStr)
{
    parameter_node* paramNode = NULL;
    char* str = NULL;
    wchar_t* strW = NULL;
    size_t len = 0;
    if (node == NULL)
    {
        lastError = NODE_PARAM_NODE_NULL;
        return NULL;
    }

    if (paramStr == NULL)
    {
        lastError = NODE_ARG_ERR;
        return NULL;
    }


    paramNode = node->ParameterNode_head;
    switch (node->isWch)
    {
    case false:
    {
        str = (char*)paramStr;
        len = strlen(str);

        if (len > (MAX_PARAMETER * sizeof(wchar_t) - 1))
        {
            lastError = NODE_PARAM_TOO_LONG;
            return NULL;
        }

        do// 即便只有一个 node 也要比较
        {
            if (strcmp((char*)(paramNode->parameter_string), str) == 0)
                return paramNode;
            paramNode = paramNode->next;
        } while (paramNode != node->ParameterNode_head);
    }
    break;
    case true:
    {
#if (ENABLE_WCHAR == 1)
        strW = (wchar_t*)paramStr;
        len = wcslen(strW);

        if (len > (MAX_PARAMETER - 1))
        {
            lastError = NODE_PARAM_TOO_LONG;
            return NULL;
        }

        do// 即便只有一个 node 也要比较
        {
            if (wcscmp(paramNode->parameter_string, strW) == 0)
                return paramNode;
            paramNode = paramNode->next;
        } while (paramNode != node->ParameterNode_head);
#else
        lastError = NODE_UNSUPPORT;
        return NULL;
#endif
    }
    break;
    }

    lastError = NODE_NOT_FIND_PARAM;
    return NULL; // 遍历后均无所获
}

/**
 * @brief 取消已注册命令
 * @param command 取决于注册节点时是否使用了宽字符, 若未使用宽字符, 另一个则填 NULL
 * @param commandW 目标命令( 宽字符 )
 * @return OK: NODE_OK
 * @return ERROR: NODE_FAIL, NODE_ARG_ERR, NODE_CMD_NODE_NULL, NODE_NOT_YET_INIT,
 * NODE_NOT_FIND_CMD
 */
int cmdNode_unRegisterCommand(const char* command, const wchar_t* commandW)
{
    command_node* CmdNode = NULL;// 要被删除的命令节点
    char cmdArr[MAX_COMMAND * sizeof(wchar_t)] = { 0 };
    wchar_t cmdArrW[MAX_COMMAND] = { 0 };
    int ret = 0;

    if (command == NULL && commandW == NULL)
    {
        printf("The command is null, exit func:<%s>\n", __func__);
        return lastError = NODE_ARG_ERR;
    }

    if (FristNode == NULL)
    {
        if (command) {
            printf("<%s>Command node does not yet exist\n", command);
            goto _end;
        }
#if (ENABLE_WCHAR == 1) 
        if (commandW) {
            wprintf(L"<%ls>Command node does not yet exist\n", commandW);
            goto _end;
        }
#endif
    _end:
        lastError = NODE_CMD_NODE_NULL;
        return lastError;
    }

    if (command != NULL)
    {
        strcpy(cmdArr, command);
        UppercaseToLowercase(cmdArr);
        CmdNode = cmdNode_FindCommand(cmdArr, NULL);
    }
    else    if (commandW != NULL)
    {
#if (ENABLE_WCHAR == 1)
        wcscpy(cmdArrW, commandW);
        UppercaseToLowercaseW(cmdArrW);
        CmdNode = cmdNode_FindCommand(NULL, cmdArrW);
#else
        lastError = NODE_UNSUPPORT;
        return lastError;
#endif
    }
    else
    {
        return lastError = NODE_FAIL;
    }




    if (CmdNode != NULL)// 通过 command 寻找 CommandNode
    {
        // 若只剩下头节点
        if (CmdNode->prev == CmdNode && CmdNode->next == CmdNode)
            FristNode = NULL;

        // 更改当前节点的邻居节点对本节点的指针
        ((command_node*)(CmdNode->prev))->next = CmdNode->next;
#if NODE_DEBUG
        printCmdNode_command(CmdNode->prev);
#endif
        if (CmdNode == FristNode)// 若头节点被删除, 更新头节点
        {
            FristNode = CmdNode->next;
#if NODE_DEBUG
            printf("update FristNode, %p\n", FristNode);
#endif
        }


        ((command_node*)(CmdNode->next))->prev = CmdNode->prev;
#if NODE_DEBUG
        printCmdNode_command(CmdNode->next);
#endif

        ret = cmdNode_unRegisterAllParameters(CmdNode);
        if (ret == NODE_OK)
        {
#if (ENABLE_WCHAR == 1)
            (CmdNode->isWch)
                ? wprintf(L"<%ls> CommandNodes unregister finish.\n",
                    CmdNode->command_string)
                : printf("<%s> CommandNodes unregister finish.\n",
                    (char*)(CmdNode->command_string));
#else
            printf("<%s> CommandNodes unregister finish.\n",
                (char*)(CmdNode->command_string));
#endif
            free(CmdNode);
            return lastError = NODE_OK;
        }

    }
    else
    {
        if (command)
            printf("<%s>not find \n", command);
        if (commandW)
            wprintf(L"<%ls>not find \n", commandW);
        return lastError;
    }
    return lastError = NODE_FAIL;
}

/**
 * @brief 更新命令
 * @param oldCommand 取决于注册节点时是否使用了宽字符, 若未使用宽字符, 另一个则填 NULL
 * @param oldCommandW 旧命令( 宽字符 )
 * @param newCommand 新命令
 * @param newCommandW 新命令( 宽字符 )
 * @return OK: NODE_OK
 * @return ERROR: NODE_ARG_ERR, NODE_NOT_FIND_CMD, NODE_REPEATING, NODE_NOT_YET_INIT,
 * NODE_FAIL
 */
int cmdNode_updateCommand(char* oldCommand, wchar_t* oldCommandW,
    char* newCommand, wchar_t* newCommandW)
{
    command_node* CmdNode = NULL;
    int ret = NODE_OK;
    if (oldCommand)
    {
        if (newCommand == NULL)
            return lastError = NODE_ARG_ERR;
    }
    else if (oldCommandW)
    {
        if (newCommandW == NULL)
            return lastError = NODE_ARG_ERR;
    }
    else
    {
        return lastError = NODE_FAIL;
    }

    if (oldCommand)
    {
        ret = UppercaseToLowercase(oldCommand);
        ERR_CHECK(ret);

        ret = UppercaseToLowercase(newCommand);
        ERR_CHECK(ret);

        // 寻找目标命令节点
        CmdNode = cmdNode_FindCommand(oldCommand, NULL);
        if (CmdNode == NULL)
        {
            printf("<%s>not find\n", oldCommand);
            return lastError = NODE_NOT_FIND_CMD;
        }

        // 判断是否已经存在新命令节点
        if (cmdNode_FindCommand(newCommand, NULL) != NULL)
        {
            printf("The same command<%s> already exists, \
it has reverted to the old command<%s> name.\n", newCommand, oldCommand);
            return lastError = NODE_REPEATING;
        }

        AssignCommandNodeStr(CmdNode, CmdNode->isWch, newCommand);
        return lastError = NODE_OK;
    }

    if (oldCommandW)
    {
#if (ENABLE_WCHAR == 1)
        ret = UppercaseToLowercaseW(oldCommandW);
        ERR_CHECK(ret);

        ret = UppercaseToLowercaseW(newCommandW);
        ERR_CHECK(ret);

        // 寻找目标命令节点
        CmdNode = cmdNode_FindCommand(NULL, oldCommandW);
        if (CmdNode == NULL)
        {
            wprintf(L"<%ls>not find\n", oldCommandW);
            return lastError;
        }

        // 判断是否已经存在新命令节点
        if (cmdNode_FindCommand(NULL, oldCommandW))
        {
            wprintf(L"The same command<%ls> already exists, \
it has reverted to the old command<%s> name.\n", newCommandW, oldCommandW);
            return lastError = NODE_REPEATING;
        }

        AssignCommandNodeStr(CmdNode, CmdNode->isWch, newCommandW);
        return lastError = NODE_OK;
#else
        lastError = NODE_UNSUPPORT;
        return lastError;
#endif
    }
    return lastError = NODE_FAIL;
}

/**
 * @brief 取消注册全部命令
 * @param
 * @return OK: NODE_OK
 * @return ERROR: NODE_FAIL, NODE_NOT_YET_INIT
 */
int cmdNode_unRegisterAllCommand(void)
{
    command_node* CmdNode = FristNode;
    command_node* nextNode = NULL;
    char* string = NULL;
    if (CmdNode == NULL)
    {
        printf("Command not yet created\n");
        return lastError = NODE_NOT_YET_INIT;
    }

    do
    {
        lastError = cmdNode_unRegisterAllParameters(CmdNode);
        if (lastError != NODE_OK)
        {
            FristNode = CmdNode;
            printf("The delete command encountered an unknown failure\n");
            return lastError;
        }
#if (ENABLE_WCHAR == 1)
        (CmdNode->isWch)
            ? wprintf(L"<%ls>(isWch) deleted\n", CmdNode->command_string)
            : printf("<%s> deleted\n", (char*)(CmdNode->command_string));
#else
        string = CmdNode->command_string;
        if (CmdNode->isWch) {
            printf("<wchar unsupport display> deleted\n");
        }
        else
            printf("<%s> deleted\n", string);
#endif

        nextNode = CmdNode->next;// 先保存 next 节点地址
        CmdNode->prev = NULL;// 断开 prev 节点
        CmdNode->next = NULL;// 断开 next 节点
        free(CmdNode);       // 释放当前节点
        CmdNode = nextNode;  // 节点移动
    } while (CmdNode != FristNode);


    FristNode = NULL;
    printf("All commands deleted successfully\n");
    return lastError;
}

/**
 * @brief 分配参数节点的字符串
 * @param node 当前参数节点
 * @param isWch 命令节点的 isWch
 * @param paramStr 参数
 * @return OK: NODE_OK
 * @return ERROR: NODE_ARG_ERR, NODE_PARAM_TOO_LONG
 */
static int AssignParameterNodeStr(parameter_node* node,
    const bool isWch,
    const void* paramStr)
{
    char* str = NULL;
    wchar_t* strW = NULL;
    size_t len = 0;

    if (node == NULL)
        return lastError = NODE_ARG_ERR;

    if (paramStr == NULL)
        return lastError = NODE_ARG_ERR;

    switch (isWch)
    {
    case false:
    {
        str = (char*)paramStr;
        len = strlen(str);

        if (len > (MAX_PARAMETER * sizeof(wchar_t) - 1))
            return lastError = NODE_PARAM_TOO_LONG;
        else
        {
            // 检查参数是否有非法字符
            for (size_t i = 0; i < len; i++)
            {
                if (passableChParam(*(str + i)))
                    return lastError = NODE_ARG_ERR;
            }
            memset(node->parameter_string, 0, sizeof(node->parameter_string));
            strcpy((char*)(node->parameter_string), str);
        }
    }
    break;
    default:
    {
#if (ENABLE_WCHAR == 1)
        strW = (wchar_t*)paramStr;
        len = wcslen(strW);

        if (len > (MAX_PARAMETER - 1))
            return lastError = NODE_PARAM_TOO_LONG;
        else
        {
            for (size_t i = 0; i < len; i++)
            {
                // 检查参数是否有非法字符
                if (passableChParamW(*(strW + i)))
                    return lastError = NODE_ARG_ERR;
            }
        }
        memset(node->parameter_string, 0, sizeof(node->parameter_string));
        wcscpy(node->parameter_string, strW);
#else
        lastError = NODE_UNSUPPORT;
        return lastError;
#endif
    }
    break;
    }
    return lastError = NODE_OK;
}

/**
 * @brief 注册参数
 * @param node 注册参数的命令
 * @param hook 参数的处理函数
 * @param isRaw 是否给 hook 传递源字符串
 * @param paramStr 参数
 * @return OK: NODE_OK
 * @return ERROR: NODE_CMD_NODE_NULL, NODE_ARG_ERR, NODE_PARAM_TOO_LONG,
 * NODE_FAIL, NODE_REPEATING
 *
 */
int cmdNode_RegisterParameter(command_node* node,
    ParameterHandler hook,
    const bool isRaw,
    const void* paramStr)
{
    parameter_node* paramNode = NULL, * tmp = NULL;
    const char* warningStr = "Not the same configuration(isWch)\
 as the command node...\n";
    if (node == NULL)// 若传入的 node 为 NULL 则会访问错误的地址, 所以先判断
        return lastError = NODE_CMD_NODE_NULL;

    if (hook == NULL)
    {
        printf("this hook is NULL.");
        return lastError = NODE_ARG_ERR;
    }

    if (paramStr == NULL)
    {
        printf("The parameter is null, exit func:<%s>", __func__);
        return lastError = NODE_ARG_ERR;
    }

    paramNode = node->ParameterNode_head;
    if (paramNode == NULL)
    {
        paramNode = (parameter_node*)malloc(sizeof(parameter_node));
        CHECK_BUF(paramNode);
        if (AssignParameterNodeStr(paramNode, node->isWch, paramStr) != NODE_OK)
        {
            free(paramNode);
            return lastError;
        }
        paramNode->handler = hook;
        paramNode->isRawStr = isRaw;

        paramNode->prev = node;
        paramNode->next = paramNode;
        node->ParameterNode_head = paramNode;
#if NODE_DEBUG
        (node->isWch)
            ? wprintf(L"<%ls><%ls> %ls\n",
                node->command_string, (wchar_t*)(paramStr), successStrW)
            : printf("<%s><%s> %s\n",
                (char*)(node->command_string), (char*)(paramStr), successStr);
#endif
        return lastError = NODE_OK;
    }
    else
    {
        paramNode = FindParameter(node, paramStr);
        if (paramNode != NULL)// 参数名称重复检查
        {
            printf("The same parameter already exists...\n");
            return lastError = NODE_REPEATING;
        }
        paramNode = (parameter_node*)malloc(sizeof(parameter_node));
        CHECK_BUF(paramNode);
        if (AssignParameterNodeStr(paramNode, node->isWch, paramStr) != NODE_OK)
        {
            free(paramNode);
            return lastError;
        }
        paramNode->handler = hook;
        paramNode->isRawStr = isRaw;
        tmp = ParameterFinalNode(node);

        paramNode->prev = tmp;
        tmp->next = paramNode;
        paramNode->next = node->ParameterNode_head;
#if NODE_DEBUG
        (node->isWch)
            ? wprintf(L"<%ls><%ls> %ls\n",
                node->command_string, (wchar_t*)(paramStr), successStrW)
            : printf("<%s><%s> %s\n",
                (char*)(node->command_string), (char*)(paramStr), successStr);
#endif
        return lastError = NODE_OK;
    }
    return lastError = NODE_FAIL;
}

/**
 * @brief 取消注册参数
 * @param node 要被删除参数的命令
 * @param paramStr 参数
 * @return OK: NODE_OK
 * @return ERROR: NODE_ARG_ERR, NODE_NOT_FIND_PARAM, NODE_FAIL, NODE_PARAM_TOO_LONG
 */
int cmdNode_unRegisterParameter(command_node* node,
    const void* paramStr)
{
    parameter_node* paramNode = NULL, * tmp = NULL;
    if (node == NULL)
        return lastError = NODE_ARG_ERR;

    if (paramStr == NULL)
        return lastError = NODE_ARG_ERR;

    paramNode = FindParameter(node, paramStr);
    if (paramNode) // 通过 parameter 寻找 ParameterNode
    {
        tmp = ParameterFinalNode(node);
        if (paramNode == node->ParameterNode_head)
        {
            // 头节点被移除, 更新头节点
            node->ParameterNode_head = paramNode->next;
            ((parameter_node*)(node->ParameterNode_head))->prev = node;

            // 更新末端节点
            tmp->next = node->ParameterNode_head;
        }
        else
        {
            // 更改当前节点的邻居节点对本节点的指针
            ((parameter_node*)(paramNode->prev))->next = paramNode->next;
            ((parameter_node*)(paramNode->next))->prev = paramNode->prev;
        }

        paramNode->next = NULL;
        paramNode->prev = NULL;

#if (ENABLE_WCHAR == 1)
        (node->isWch)
            ? wprintf(L"<%ls> %ls, parameterNode unregister finish.\n",
                node->command_string, paramNode->parameter_string)
            : printf("<%s> %s, parameterNode unregister finish.\n",
                (char*)(node->command_string), (char*)(paramNode->parameter_string));
#else
        printf("<%s> %s, parameterNode unregister finish.\n",
            (char*)(node->command_string), (char*)(paramNode->parameter_string));
#endif

        free(paramNode);
        return lastError = NODE_OK;
    }
    else
    {
#if (ENABLE_WCHAR == 1)
        (node->isWch)
            ? wprintf(L"<%s>, not find parameterNode\n", node->command_string)
            : printf("<%s>, not find parameterNode\n", (char*)(node->command_string));
#else
        printf("<%s>, not find parameterNode\n", (char*)(node->command_string));
#endif
        return lastError;
    }
    return lastError = NODE_FAIL;
}

/**
 * @brief 更新参数
 * @param CmdNode 要被更改参数的命令
 * @param hook 参数处理函数
 * @param isRaw 是否传递原始字符串作为 hook 的参数
 * @param oldParam 旧参数
 * @param newParam 新参数
 * @return OK: NODE_OK
 * @return ERROR: NODE_ARG_ERR, NODE_REPEATING, NODE_NOT_FIND_PARAM
 */
int cmdNode_updateParameter(const command_node* CmdNode, ParameterHandler hook,
    const bool isRaw, const void* oldParam, const void* newParam)
{
    const char* charWarning = "this string has illegal character...";
    parameter_node* ParamNode = NULL;
    char* oldStr = NULL, * newStr = NULL;
    wchar_t* oldStrW = NULL, * newStrW = NULL;
    size_t len = 0;
    if (CmdNode == NULL)
        return lastError = NODE_ARG_ERR;


    if (oldParam == NULL)
    {
        if (newParam == NULL)
            return lastError = NODE_ARG_ERR;
    }
    else
    {
        if (newParam == NULL)
            return lastError = NODE_ARG_ERR;
    }

    // 字符串合法检查
    switch (CmdNode->isWch)
    {
    case false:
    {
        oldStr = (char*)oldParam;
        if (oldStr == NULL)
            return lastError = NODE_ARG_ERR;

        len = strlen(oldStr);
        if (len > (MAX_PARAMETER * sizeof(wchar_t) - 1))
            return lastError = NODE_ARG_ERR;

        for (size_t i = 0; i < len; i++)
        {
            if (passableChParam(*(oldStr + i)))
            {
                printf("%s\n", charWarning);
                return lastError = NODE_ARG_ERR;
            }
        }

        newStr = (char*)newParam;
        if (newStr == NULL)
            return lastError = NODE_ARG_ERR;

        len = strlen(newStr);
        if (len > (MAX_PARAMETER * sizeof(wchar_t) - 1))
            return lastError = NODE_ARG_ERR;

        for (size_t i = 0; i < len; i++)
        {
            if (passableChParam(*(newStr + i)))
            {
                printf("%s\n", charWarning);
                return lastError = NODE_ARG_ERR;
            }
        }
    }
    break;
    default:
    {
#if (ENABLE_WCHAR == 1)
        oldStrW = (wchar_t*)oldParam;
        if (oldStrW == NULL)
            return lastError = NODE_ARG_ERR;

        len = wcslen(oldStrW);
        if (len > (MAX_PARAMETER - 1))
            return lastError = NODE_ARG_ERR;

        for (size_t i = 0; i < len; i++)
        {
            if (passableChParamW(*(oldStrW + i)))
            {
                printf("%s...\n", charWarning);
                return lastError = NODE_ARG_ERR;
            }
        }

        newStrW = (wchar_t*)newParam;
        if (newStrW == NULL)
            return lastError = NODE_ARG_ERR;

        len = wcslen(newStrW);
        if (len > (MAX_PARAMETER - 1))
            return lastError = NODE_ARG_ERR;

        for (size_t i = 0; i < len; i++)
        {
            if (passableChParamW(*(newStrW + i)))
            {
                printf("%s\n", charWarning);
                return lastError = NODE_ARG_ERR;
            }
        }
#else
        lastError = NODE_UNSUPPORT;
        return lastError;
#endif
    }
    break;
    }


    // 寻找旧参数
    ParamNode = FindParameter(CmdNode, oldParam);
    if (ParamNode == NULL)
    {
#if (ENABLE_WCHAR == 1)
        (CmdNode->isWch)
            ? wprintf(L"<%ls> <%ls>not find\n", CmdNode->command_string, (wchar_t*)oldParam)
            : printf("<%s> <%s>not find\n", (char*)(CmdNode->command_string), (char*)oldParam);
#else
        printf("<%s> <%s>not find\n", (char*)(CmdNode->command_string), (char*)oldParam);
#endif
        return lastError = NODE_NOT_FIND_PARAM;
    }

    // 检查新参数是否已经存在
    if (FindParameter(CmdNode, newParam) != NULL)
    {
#if (ENABLE_WCHAR == 1)
        (CmdNode->isWch)
            ? wprintf(L"The same param<%ls> already exists,\
 it has reverted to the old param<%ls> name.\n", (wchar_t*)newParam, (wchar_t*)oldParam)
            : printf("The same param<%s> already exists,\
 it has reverted to the old param<%s> name.\n", (char*)newParam, (char*)oldParam);
#else
        printf("The same param<%s> already exists,\
 it has reverted to the old param<%s> name.\n", (char*)newParam, (char*)oldParam);
#endif
        return lastError = NODE_REPEATING;
    }

    // 修改参数
    lastError = AssignParameterNodeStr(ParamNode, CmdNode->isWch, newParam);
    if (lastError != NODE_OK)
        return lastError;
    ParamNode->handler = hook;
    return lastError;
}


/**
 * @brief 命令解析
 * @param commandString 用户输入的字符串
 * @return OK: NODE_OK
 * @return ERROR: NODE_ARG_ERR, NODE_CMD_TOO_LONG, NODE_PARAM_TOO_LONG,
 * NODE_NOT_FIND_CMD, NODE_NOT_FIND_PARAM, NODE_PARSE_ERR
 */
int cmdNode_CommandParse(const char* commandString)
{
    const char* spaceStr = " ";
    command_node* CmdNode = NULL;
    parameter_node* ParamNode = NULL;
    userString* userData = NULL;
    char cmd[MAX_COMMAND] = { 0 };
    char param[MAX_PARAMETER] = { 0 };
    size_t len = 0;
    if (commandString == NULL)
        return lastError = NODE_ARG_ERR;

    ParseSpace(commandString);
    switch (userParse_GetUserParamCnt())
    {
    case 0:break;
    case 1: {

        userData = userParse_pUserData();
        if (userData == NULL)
            goto _err;
        memcpy(cmd, userData[0].strHead, userData[0].len);
        // 直接把源字符串扔进去寻找目标命令
        CmdNode = cmdNode_FindCommand(cmd, NULL);
        if (CmdNode == NULL)
        {
            printf("<%s>The command was not found.\n", cmd);
            break;
        }
        printf("<%s>There is no input parameter for this command...\n", cmd);
        cmdNode_showParam(CmdNode);
        break;
    }
    case 2:
    default: {

        userData = userParse_pUserData();
        if (userData == NULL)
            goto _err;

        memcpy(cmd, userData[0].strHead, userData[0].len);
        memcpy(param, userData[1].strHead, userData[1].len);
#if NODE_DEBUG
        VAR_PRINT_STRING(cmd);
        VAR_PRINT_STRING(param);
#endif

        CmdNode = cmdNode_FindCommand(cmd, NULL);
        if (CmdNode == NULL)
        {
            printf("<%s>The command was not found.\n", cmd);
            break;
        }

        ParamNode = FindParameter(CmdNode, param);
        if (ParamNode == NULL)
        {
            printf("<%s><%s>Parameter not found in current command...\n",
                cmd, param);
            break;
        }

        ParamNode->handlerArg = NULL;
#if NODE_DEBUG
        VAR_PRINT_INT(userParse_GetUserParamCnt());
#endif
        if (userParse_GetUserParamCnt() > 2)
        {
#if NODE_DEBUG
            VAR_PRINT_POS(userData[0]);
            VAR_PRINT_POS(userData[1]);
            VAR_PRINT_POS(userData[2]);
            VAR_PRINT_UD(ParamNode->isRawStr);
            VAR_PRINT_POS(userParse_pUserDataAddr());
            VAR_PRINT_POS(&userData[0]);
            VAR_PRINT_POS(&userData[2]);
            VAR_PRINT_POS(userData[0]);
            VAR_PRINT_POS(userData[2]);
            VAR_PRINT_POS(userData[2].strHead);
            VAR_PRINT_STRING((*userParse_pUserDataAddr())[0].strHead);
            VAR_PRINT_STRING((*userParse_pUserDataAddr())[2].strHead);
            VAR_PRINT_POS(&(*userParse_pUserDataAddr())[0]);
            VAR_PRINT_POS(&(*userParse_pUserDataAddr())[2]);
            VAR_PRINT_POS((*userParse_pUserDataAddr())[0]);
            VAR_PRINT_POS((*userParse_pUserDataAddr())[2]);
            //VAR_PRINT_STRING((*(userParse_pUserDataAddr() + (2 * sizeof(userString))))->strHead);
            //VAR_PRINT_STRING((*userParse_pUserDataAddr())->strHead);
#endif

            ParamNode->handlerArg = (ParamNode->isRawStr)
                ? userData[2].strHead
                : &userData[2];
#if NODE_DEBUG
            VAR_PRINT_POS(ParamNode->handlerArg);
#endif
        }


#if NODE_DEBUG
        VAR_PRINT_POS(ParamNode->handler);
#endif
        if (ParamNode->handler == NULL)
        {
            lastError = NODE_NO_HANDLER;
            goto _err;
        }
        ParamNode->handler(ParamNode->handlerArg);
        lastError = NODE_OK;
        break;
    }

    }
_err:
#if NODE_DEBUG
    DEBUG_PRINT("END");
#endif
    RESET_USERDATA_RECORD();
    return lastError;
}

#if ENABLE_WCHAR
#ifdef WCHAR_MIN
#ifdef WCHAR_MAX


/**
 * @brief 命令解析( 宽字符 )
 * @param commandString 用户输入的字符串
 * @return OK: NODE_OK
 * @return ERROR: NODE_ARG_ERR, NODE_CMD_TOO_LONG, NODE_PARAM_TOO_LONG,
 * NODE_NOT_FIND_CMD, NODE_NOT_FIND_PARAM, NODE_PARSE_ERR
 */

int cmdNode_CommandParseW(const wchar_t* commandString)
{
    const wchar_t* spaceStr = L" ";
    command_node* CmdNode = NULL;
    parameter_node* ParamNode = NULL;
    wchar_t cmd[MAX_COMMAND] = { 0 };
    wchar_t param[MAX_PARAMETER] = { 0 };
    const wchar_t* tmp = NULL, * tmp2 = NULL;
    size_t len = 0;
    if (commandString == NULL)
        return lastError = NODE_ARG_ERR;

    ParseSpaceW(commandString);
    switch (userParse_GetUserParamCnt())
    {
    case 0:break;
    case 1: {
        memcpy(cmd, userData, (userData[0].len) * sizeof(wchar_t));
        CmdNode = cmdNode_FindCommand(NULL, cmd);
        if (CmdNode == NULL)
        {
            wprintf(L"<%ls>The command was not found.\n", tmp2);
            break;
        }
        wprintf(L"<%ls>There is no input parameter for this command...\n", tmp2);
        cmdNode_showParam(CmdNode);
        break;
    }
    case 2:
    default: {
        memcpy(cmd, userData[0].strHead, (userData[0].len) * sizeof(wchar_t));
        memcpy(param, userData[1].strHead, (userData[1].len) * sizeof(wchar_t));
        CmdNode = cmdNode_FindCommand(NULL, cmd);
        if (CmdNode == NULL)
        {
            wprintf(L"<%ls>The command was not found.\n", tmp2);
            break;
        }
        ParamNode = cmdNode_FindParameter(CmdNode, tmp2);
        if (ParamNode == NULL)
        {
            wprintf(L"<%ls><%ls>Parameter not found in current command...\n", cmd, tmp2);
            break;
        }

        ParamNode->handlerArg = NULL;
        if (userParse_GetUserParamCnt() > 2)
            ParamNode->handlerArg = (ParamNode->isRawStr) ? userData[2].strHead : &userData[2];
        ParamNode->handler(ParamNode->handlerArg);
        lastError = NODE_OK;
        break;
    }
    }

    RESET_USERDATA_RECORD();
    return lastError;
}

#endif // WCHAR_MAX
#endif // WCHAR_MIN
#endif


/**
 * @brief 获得上次运行链表管理函数的错误
 * @param
 * @return lastError
 */
int cmdNode_GetLastError(void)
{
    switch (lastError)
    {
    case NODE_OK:
        DEBUG_PRINT("NODE OK\n");
        break;
    case NODE_FAIL:
        DEBUG_PRINT("NODE unknow error\n");
        break;
    case NODE_ARG_ERR:
        DEBUG_PRINT("NODE parameter passing error\n");
        break;
    case NODE_NOT_FIND:
        DEBUG_PRINT("NODE not find\n");
        break;
    case NODE_NOT_FIND_CMD:
        DEBUG_PRINT("NODE not found command\n");
        break;
    case NODE_NOT_FIND_PARAM:
        DEBUG_PRINT("NODE not found parameter\n");
        break;
    case NODE_ALLOC_ERR:
        DEBUG_PRINT("NODE Alloc error, maybe RAM is not enough...\n");
        break;
    case NODE_CMD_NODE_NULL:
        DEBUG_PRINT("NODE this command is null\n");
        break;
    case NODE_PARAM_NODE_NULL:
        DEBUG_PRINT("NODE command has no paramNode\n");
        break;
    case NODE_REPEATING:
        DEBUG_PRINT("NODE is repeating create\n");
        break;
    case NODE_CMD_TOO_LONG:
        DEBUG_PRINT("NODE 'command' is too long\n");
        break;
    case NODE_PARAM_TOO_LONG:
        DEBUG_PRINT("NODE 'parameter' is too long\n");
        break;
    case NODE_PARSE_ERR:
        DEBUG_PRINT("NODE parsing string error\n");
        break;
    case NODE_NOT_YET_INIT:
        DEBUG_PRINT("NODEs have not been initialized...\n");
        break;
    case NODE_NO_HANDLER:
        DEBUG_PRINT("NODE this command have no handler...");
    default:
        ERROR_PRINT("unknow error...");
        break;
    }
    return lastError;
}

#if ENABLE_REG
/**
 * @brief 获得命令节点图
 * @param map 节点图的首地址
 * @return map's len
 */
int NodeGetCommandMap(command_info** map)
{
    command_info* tmp = *map;
    command_node* current = FristNode;
    int len = 0;

    if (current == NULL)
    {
        printf("<%s>list null...\n", __func__);
        return 0;
    }

    do
    {
        len++;
        tmp = *map;// 先保存 map , 预防 realloc 失败导致内存泄露
        *map = (command_info*)realloc(tmp, len * sizeof(command_info));
        if (*map == NULL)
        {
            lastError = NODE_ALLOC_ERR;
            free(tmp);
            return 0;
        }

        (*map + len - 1)->command = (void*)(current->command_string);
        (*map + len - 1)->node = (void*)current;
        current = current->next;
    } while (current != FristNode);
    return len;
}


/**
 * @brief 输入指令 注册命令
 * @param arg
 */
static void regCmd(void* arg)
{
    userString* data = (userString*)arg;
    char cmd[MAX_COMMAND] = { 0 };
    int err = NODE_OK;
    command_node* cmdNode = NULL;
    if (data == NULL)
    {
        printf("<%s>userParam error\n", __func__);
        return;
    }

    memcpy(cmd, data->strHead, data->len);
    printf("<%s>parse cmd:<%s>\n", __func__, cmd);
    cmdNode = cmdNode_FindCommand(cmd, NULL);
    err = cmdNode_GetLastError();
    if (cmdNode == NULL && (err == NODE_NOT_YET_INIT || err == NODE_NOT_FIND_CMD))
    {
        cmdNode_RegisterCommand(0, cmd);
        cmdNode_GetLastError();
    }
    else
    {
        printf("<%s>The command already exists\n", __func__);
    }
    return;
}

/**
 * @brief 输入指令 注册命令的参数
 * @param arg
 */
static void regParam(void* arg)
{
    userString* data = (userString*)arg;
    char* endptr = NULL;
    void* func = NULL;
    unsigned long long funcAdd = 0;
    char cmdArr[MAX_COMMAND] = { 0 },
        paramArr[MAX_PARAMETER] = { 0 },
        funcArr[32] = { 0 };
    command_node* cmdNode = NULL;
    size_t paramPos = 0, paramCnt = userParse_GetUserParamCnt();
    if (data == NULL)
    {
        printf("<%s>userParam error\n", __func__);
        return;
    }

    if (paramCnt < 2)
    {
        printf("<%s>Insufficient input parameters...\n", __func__);
        return;
    }

    memcpy(cmdArr, (data + paramPos)->strHead, (data + paramPos)->len);
    printf("<%s>parse cmd:<%s>\n", __func__, cmdArr);

    paramPos++;
    memcpy(paramArr, (data + paramPos)->strHead, (data + paramPos)->len);
    printf("<%s>parse param:<%s>\n", __func__, paramArr);

    // 有额外参数, 默认是地址参数
    if (paramCnt > 2)
    {
        paramPos++;
        memcpy(funcArr, (data + paramPos)->strHead, (data + paramPos)->len);
        funcAdd = strtoull(funcArr, &endptr, 16);
        func = (void*)funcAdd;
        printf("<%s>parse funcAdd:<%llx><%s>\n", __func__, funcAdd, endptr);
    }

    // 寻找目标命令
    cmdNode = cmdNode_FindCommand(cmdArr, NULL);
    if (cmdNode == NULL)
    {
        cmdNode_GetLastError();
        return;
    }

    if (strcmp(paramArr, "exit") == 0)
    {
        cmdNode_RegisterParameter(cmdNode, exit, 1, paramArr);
        return;
    }

    (paramCnt > 2)
        ? cmdNode_RegisterParameter(cmdNode, func, 0, paramArr)
        : cmdNode_RegisterParameter(cmdNode, NULL, 0, paramArr);
    return;
}

/**
 * @brief 输入指令 删除指定的命令
 * @param arg
 */
static void regDelCmd(void* arg)
{
    char cmd[MAX_COMMAND] = { 0 };
    userString* data = (userString*)arg;
    if (data == NULL)
    {
        printf("<%s>userParam error\n", __func__);
        return;
    }

    memcpy(cmd, data->strHead, data->len);
    if (strcmp(cmd, DEFAULT_CMD) == 0)
    {
        printf("<%s>cannot be deleted command\n", __func__);
        return;
    }

    if (cmdNode_unRegisterCommand(cmd, NULL))
    {
        printf("<%s>", __func__);
        cmdNode_GetLastError();
        return;
    }
}

/**
 * @brief 输入指令 删除指定命令的所有参数
 * @param arg
 */
static void regDelAllParam(void* arg)
{
    userString* data = (userString*)arg;
    char cmd[MAX_COMMAND] = { 0 };
    command_node* cmdNode = NULL;
    if (data == NULL)
    {
        printf("<%s>userParam error\n", __func__);
        return;
    }

    memcpy(cmd, data->strHead, data->len);
    if (strcmp(cmd, DEFAULT_CMD) == 0)
    {
        printf("<%s>cannot be deleted command\n", __func__);
        return;
    }

    cmdNode = cmdNode_FindCommand(cmd, NULL);
    if (cmdNode == NULL)
    {
        printf("<%s>", __func__);
        cmdNode_GetLastError();
        return;
    }

    if (cmdNode_unRegisterAllParameters(cmdNode))
    {
        printf("<%s>", __func__);
        cmdNode_GetLastError();
        return;
    }
}

/**
 * @brief 输入指令 删除命令中的指定参数
 * @param arg
 */
static void regDelParam(void* arg)
{
    userString* data = (userString*)arg;
    char cmdArr[MAX_COMMAND] = { 0 },
        paramArr[MAX_PARAMETER] = { 0 };
    size_t dataCnt = userParse_GetUserParamCnt();
    command_node* cmdNode = NULL;
    if (data == NULL)
    {
        printf("<%s>userParam error\n", __func__);
        return;
    }

    if (dataCnt < 2)
    {
        printf("<%s>Insufficient input parameters\n", __func__);
        return;
    }

    memcpy(cmdArr, data->strHead, data->len);
    printf("<%s>parse cmd:%s\n", __func__, cmdArr);

    memcpy(paramArr, (data + 1)->strHead, (data + 1)->len);
    printf("<%s>parse param:%s\n", __func__, paramArr);

    if (strcmp(cmdArr, DEFAULT_CMD) == 0)
    {
        printf("<%s>cannot be deleted command\n", __func__);
        return;
    }

    cmdNode = cmdNode_FindCommand(cmdArr, NULL);
    if (cmdNode == NULL)
    {
        printf("<%s>", __func__);
        cmdNode_GetLastError();
        return;
    }

    if (cmdNode_unRegisterParameter(cmdNode, paramArr))
    {
        printf("<%s>", __func__);
        cmdNode_GetLastError();
        return;
    }
}

/**
 * @brief 输入指令 删除所有命令"reg"除外
 * @param arg
 */
static void regDelAllCmd(void* arg)
{
    int len = 0;
    command_info* Map = NULL;
    len = NodeGetCommandMap(&Map);
    if (len == 0 || Map == NULL)
    {
        printf("get list map fail, len:%d, map:%p\n", len, Map);
        free(Map);
        return;
    }

    for (size_t i = 0; i < len; i++)
    {
        switch (((command_node*)((Map + i)->node))->isWch)
        {
        case false:
        {
            if (strcmp((char*)((Map + i)->command), DEFAULT_CMD) == 0)
                continue;
            else
                cmdNode_unRegisterCommand((char*)(Map + i)->command, NULL);
        }
        break;
        default:
        {
            if (wcscmp((wchar_t*)((Map + i)->command), DEFAULT_CMD_W) == 0)
                continue;
            else
                cmdNode_unRegisterCommand(NULL, (wchar_t*)(Map + i)->command);
        }
        break;
        }
    }
    return;
}

/**
 * @brief 初始化默认命令 reg
 * @param
 * @return OK: 0
 * @return ERROR: others
 */
int defaultRegCmd_init(void)
{

    command_node* CmdNode = NULL;
    cmdNode_RegisterCommand(0, DEFAULT_CMD);
    CmdNode = cmdNode_FindCommand(DEFAULT_CMD, NULL);
    if (CmdNode == NULL)
    {
        printf("<%s>", __func__);
        cmdNode_GetLastError();
        return -1;
    }

    cmdNode_RegisterParameter(CmdNode, regCmd, false, DEFAULT_PARAM_cmd);
    cmdNode_RegisterParameter(CmdNode, regParam, false, DEFAULT_PARAM_param);
    cmdNode_RegisterParameter(CmdNode, regDelAllParam, false, DEFUALT_PARAM_DelAllParam);
    cmdNode_RegisterParameter(CmdNode, regDelCmd, false, DEFAULT_PARAM_DelCmd);
    cmdNode_RegisterParameter(CmdNode, regDelParam, false, DEFUALT_PARAM_DelParam);
    cmdNode_RegisterParameter(CmdNode, regDelAllCmd, false, DEFUALT_PARAM_DelAllCmd);
    cmdNode_RegisterParameter(CmdNode, cmdNode_showList, false, DEFUALT_PARAM_ls);

    return 0;
}

#endif
#endif // CMD_METHOD_NODE
