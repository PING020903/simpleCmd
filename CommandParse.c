#include "CommandParse.h"
#include <stdio.h>
#include <string.h>
#if COMMAND_SUPPORT_WCAHR
#include <wchar.h>
#endif
#include <stdlib.h>
#include "./include/DBG_macro.h"
#include "./include/CommandParse.h"


#define ERR_CHECK(err)                                              \
    do {                                                            \
        if (err) {                                                  \
            VAR_PRINT_INT (err);                                    \
            DEBUG_PRINT ("file:{%s}, line:%d", __FILE__, __LINE__); \
        }                                                           \
    } while (0)


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
static command_node *FristNode = NULL;
static const char *successStr = "Successful registration.";
#if COMMAND_SUPPORT_WCAHR
static const wchar_t *successStrW = L"(isWch) Successful registration.";
#endif
/**
 * @brief 上一次函数运行的错误
 */
static int lastError = NODE_OK;

/**
 * @brief 用户的字符串
 */
static userString *userData = NULL;

/**
 * @brief 被跳过的空格数量
 */
static size_t userDataPass = 0;

/**
 * @brief 用户参数的数量
 */
static size_t userDataCnt = 0;

#if NODE_DEBUG
static command_node *compare1 = NULL, *compare2 = NULL;
static parameter_node *compare3 = NULL, *compare4 = NULL;
#endif

#define CHECK_BUF(buf)                         \
    do {                                       \
        if (buf == NULL) {                     \
            free (buf);                        \
            return lastError = NODE_ALLOC_ERR; \
        }                                      \
    } while (0)

#if NODE_DEBUG
/**
 * @brief 打印指定节点的左右临近节点命令名
 * @param CmdNode 命令节点
 * @return OK： NODE_OK
 * @return ERROR: NODE_ARG_ERR
 */
static int printCmdNode_command (command_node *CmdNode) {
    command_node *NEXT = NULL, *PREV = NULL;
    if (CmdNode == NULL)
        return NODE_ARG_ERR;

    NEXT = CmdNode->next;
    PREV = CmdNode->prev;


    (PREV->isWch)
        ? wprintf (L" prev:<%ls>,", PREV->command_string)
        : DEBUG_PRINT (" prev:<%s>,", (char *)(PREV->command_string));

    (CmdNode->isWch)
        ? wprintf (L" this:<%ls>,", CmdNode->command_string)
        : DEBUG_PRINT (" this:<%s>,", (char *)(CmdNode->command_string));

    (NEXT->isWch)
        ? wprintf (L" next:<%ls>\n", NEXT->command_string)
        : DEBUG_PRINT (" next:<%s>\n", (char *)(NEXT->command_string));

    return NODE_OK;
}
#endif
/**
 * @brief 可解析的字符( 命令用 )
 * @param ch 当前地址的字符
 * @return OK: 0,  ERROR: 1
 */
static int passableChCmd (const char ch) {
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
static int passableChParam (const char ch) {
    const char NoSupportChar = ' ';
    return (ch > NoSupportChar)
               ? 0
               : 1;
}

/// @brief 命令字符解析器
/// @param str 字符串
/// @return character
/// @note 将大写字符转小写, 其余直接返回
static char UserCharacterParse (const char *str) {
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
static int UppercaseToLowercase (char *str) {
    size_t len = 0, i = 0;
    if (str == NULL)
        return NODE_ARG_ERR;

    len = strlen (str);

    for (; i < len; i++) {
        // 将不规则的大小写输入统一为小写
        *(str + i) = UserCharacterParse (str + i);
    }
    return NODE_OK;
}
#ifdef WCHAR_MIN
#ifdef WCHAR_MAX
#if COMMAND_SUPPORT_WCAHR
/// @brief 命令字符解析器( 宽字符版本 )
/// @param str 字符串
/// @return character
/// @note 将大写字符转小写, 其余直接返回
static wchar_t UserCharacterParseW (const wchar_t *str) {
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
static int UppercaseToLowercaseW (wchar_t *str) {
    size_t len = 0, i = 0;
    if (str == NULL)
        return NODE_ARG_ERR;

    len = wcslen (str);

    for (; i < len; i++) {
        // 将不规则的大小写输入统一为小写
        *(str + i) = UserCharacterParseW (str + i);
    }
    return NODE_OK;
}

/**
 * @brief 可解析的字符( 命令用, 宽字符 )
 * @param ch 当前地址的字符
 * @return OK: 0,  ERROR: 1
 */
static int passableChCmdW (const wchar_t ch) {
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
static int passableChParamW (const wchar_t ch) {
    const wchar_t NoSupportChar = L' ';
    return (ch > NoSupportChar)
               ? 0
               : 1;
}
#endif
#endif
#endif
/**
 * @brief 当前命令链表中最后的节点
 * @return target node
 */
static command_node *CommandFinalNode() {
    command_node *CmdNode = FristNode;
    while (CmdNode->next != FristNode)  // 遍历链表
    {
        if (CmdNode != CmdNode->next) {
            CmdNode = CmdNode->next;
        } else {
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
static parameter_node *ParameterFinalNode (const command_node *node) {
    parameter_node *paramNode = node->ParameterNode_head;
    if (node == NULL)
        return NULL;
    while (paramNode->next != node->ParameterNode_head) {
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
static int AssignCommandNodeStr (command_node *node,
                                 const bool isWch,
                                 const void *cmdStr) {
    size_t len = 0;
#if COMMAND_SUPPORT_WCAHR
    wchar_t *strW = NULL;
#endif
    char *str = NULL, *tmp = NULL;
    if (node == NULL || cmdStr == NULL)
        return lastError = NODE_ARG_ERR;

    node->isWch = isWch;
    switch ((int)isWch) {
    case true: {
#if COMMAND_SUPPORT_WCAHR
        strW = (wchar_t *)cmdStr;
        len = wcslen (strW);
        if (len > (MAX_COMMAND - 1))
            return lastError = NODE_CMD_TOO_LONG;

        // 非法字符检查
        for (size_t i = 0; i < len; i++) {
            if (passableChCmdW (*(strW + i)))
                return lastError = NODE_ARG_ERR;
        }
        wcscpy (node->command_string, strW);
        UppercaseToLowercaseW (node->command_string);
#endif
#if !COMMAND_SUPPORT_WCAHR
        return NODE_FAIL;
#endif
    } break;
    default: {
        tmp = (char *)(node->command_string);
        str = (char *)cmdStr;
        len = strlen (str);
        if (len > (MAX_COMMAND * sizeof (wchar_t) - 1))
            return lastError = NODE_CMD_TOO_LONG;

        // 非法字符检查
        for (size_t i = 0; i < len; i++) {
            if (passableChCmd (*(str + i)))
                return lastError = NODE_ARG_ERR;
        }
        strcpy (tmp, str);
        UppercaseToLowercase (tmp);
    } break;
    }
    return lastError = NODE_OK;
}

#if NODE_LINK_LIST
/**
 * @brief 打印指定命令下所有参数
 * @param CmdNode 命令节点
 */
void showParam (command_node *CmdNode) {
    parameter_node *node = CmdNode->ParameterNode_head;
    const void *ParamString = NULL;
    if (CmdNode == NULL)
        return;

    if (node == NULL) {
#if COMMAND_SUPPORT_WCAHR
        (CmdNode->isWch)
            ? wprintf (L"<%ls>this command has no parameters...\n", CmdNode->command_string)
            : DEBUG_PRINT ("<%s>this command has no parameters...\n", (char *)(CmdNode->command_string));
#else
        DEBUG_PRINT ("<%s>this command has no parameters...", (char *)(CmdNode->command_string));
#endif
        return;
    }

    DEBUG_PRINT ("");
    DEBUG_PRINT ("  parameters:");
    do {
#if COMMAND_SUPPORT_WCAHR
        (CmdNode->isWch)
            ? wprintf (L"        %ls\n", node->parameter_string)
            : DEBUG_PRINT ("        %s\n", (char *)(node->parameter_string));
#else
        // DEBUG_PRINT ("        %s", (char *)(node->parameter_string));
        ParamString = node->parameter_string;
        DEBUG_PRINT ("        %s", (char *)ParamString);
#endif
        node = node->next;
    } while (node != CmdNode->ParameterNode_head);
}

/**
 * @brief 打印命令链表
 */
void showList (void) {
    command_node *node = FristNode;
    size_t cnt = 1;

    if (node == NULL) {
        DEBUG_PRINT ("list null...");
        return;
    }

    DEBUG_PRINT ("");
    do {
#if COMMAND_SUPPORT_WCAHR
        (node->isWch)
            ? wprintf (L"%llu:command  <%ls>(isWch)     ", cnt++, node->command_string)
            : DEBUG_PRINT ("%llu:command  <%s>     ", cnt++, (char *)(node->command_string));
#else
        DEBUG_PRINT ("%u:command  <%s>     ", cnt++, (char *)(node->command_string));
#endif
        showParam (node);
        node = node->next;
    } while (node != FristNode);  // 为了保证遍历完整
    DEBUG_PRINT ("");
}
#if NODE_DEBUG
/**
 * @brief 检查当前命令节点是否有重复的命令
 * @param
 * @return OK: NODE_OK
 * @return ERROR: NODE_CMD_NODE_NULL, NODE_REPEATING, NODE_FAIL
 */
static int RepeatingCommandCheck (void) {
    command_node *outer = FristNode, *inner = NULL;
    if (outer == NULL) {
        DEBUG_PRINT ("Command not yet created.\n");
        return NODE_CMD_NODE_NULL;
    }

    do {
        inner = outer->next;
        while (inner != FristNode) {
            if (outer->isWch && inner->isWch) {
                if (wcscmp (outer->command_string, inner->command_string) == 0) {
                    compare1 = outer;
                    compare2 = inner;
                    return NODE_REPEATING;
                }
            } else if (!outer->isWch && !inner->isWch) {
                if (strcmp ((char *)(outer->command_string), (char *)(inner->command_string)) == 0) {
                    compare1 = outer;
                    compare2 = inner;
                    return NODE_REPEATING;
                }
            } else {
                return NODE_FAIL;
            }
            inner = inner->next;
        }
        outer = outer->next;
    } while (outer != FristNode);
    return NODE_OK;
}

/**
 * @brief 检查当前参数节点是否有重复的参数
 * @param CmdNode 要被检查的命令节点
 * @return OK: NODE_OK
 * @return ERROR: NODE_ARG_ERR, NODE_PARAM_NODE_NULL, NODE_REPEATING, NODE_FAIL
 */
static int RepeatingParamCheck (const command_node *CmdNode) {
    parameter_node *outer = NULL, *inner = NULL;
    if (CmdNode == NULL)
        return NODE_ARG_ERR;

    if (CmdNode->ParameterNode_head == NULL)
        return NODE_PARAM_NODE_NULL;

    outer = CmdNode->ParameterNode_head;
    do {

        inner = outer->next;
        while (inner != CmdNode->ParameterNode_head) {
            switch (CmdNode->isWch) {
            case true: {
                if (wcscmp (outer->parameter_string, inner->parameter_string) == 0) {
                    compare3 = outer;
                    compare4 = inner;
                    return NODE_REPEATING;
                }
            } break;
            case false: {
                if (strcmp ((char *)(outer->parameter_string),
                            (char *)(inner->parameter_string)) == 0) {
                    compare3 = outer;
                    compare4 = inner;
                    return NODE_REPEATING;
                }
            } break;
            }
        }
        outer = outer->next;
    } while (outer != CmdNode->ParameterNode_head);
    return NODE_OK;
}
#endif
/**
 * @brief 寻找命令节点
 * @param command 命令, 取决于注册节点时是否使用了宽字符 , 若未使用宽字符, 另一个则填 NULL
 * @param commandW 命令( 宽字符 )
 * @return ERROR: NULL
 * @return OK: target node
 */
command_node *FindCommand (const char *command,
                           const wchar_t *commandW) {
    command_node *CmdNode = FristNode;
    char CmdTmp[MAX_COMMAND * sizeof (wchar_t)] = {0};
    wchar_t CmdTmpW[MAX_COMMAND] = {0};

    if (CmdNode == NULL) {
        lastError = NODE_NOT_YET_INIT;
        return NULL;
    }


    if (command == NULL && commandW == NULL) {
        lastError = NODE_ARG_ERR;
        return NULL;
    }


    if (command != NULL) {
        if (strlen (command) > (MAX_COMMAND * sizeof (wchar_t) - 1)) {
            lastError = NODE_CMD_TOO_LONG;
            return NULL;
        }

        strcpy (CmdTmp, command);
        if (UppercaseToLowercase (CmdTmp))  // 确保字符统一小写
            return NULL;

        do {
            if (strcmp ((char *)(CmdNode->command_string), CmdTmp) == 0)
                return CmdNode;

            CmdNode = CmdNode->next;
        } while (CmdNode != FristNode);
    } else if (commandW != NULL) {
#if COMMAND_SUPPORT_WCAHR
        if (wcslen (commandW) > (MAX_COMMAND - 1)) {
            lastError = NODE_CMD_TOO_LONG;
            return NULL;
        }

        wcscpy (CmdTmpW, commandW);
        if (UppercaseToLowercaseW (CmdTmpW))  // 确保字符统一小写
            return NULL;

        do {
            if (wcscmp (CmdNode->command_string, CmdTmpW) == 0)
                return CmdNode;

            CmdNode = CmdNode->next;
        } while (CmdNode != FristNode);
#else
        return NULL;
#endif
    } else {
        lastError = NODE_FAIL;
        return NULL;
    }
    lastError = NODE_NOT_FIND_CMD;
    return NULL;  // 遍历后均无所获
}

/**
 * @brief 注册命令
 * @param isWch 该命令是否使用宽字符
 * @param cmdStr 命令
 * @return OK: NODE_OK
 * @return ERROR: NODE_FAIL, NODE_ARG_ERR, NODE_CMD_TOO_LONG, NODE_REPEATING
 */
int RegisterCommand (const bool isWch,
                     const void *cmdStr) {
#if COMMAND_SUPPORT_WCAHR
#define PRINT_CMD(NODE) (isWch)                                                           \
                            ? wprintf (L"<%ls> %ls\n", NODE->command_string, successStrW) \
                            : DEBUG_PRINT ("<%s> %s\n", (char *)(NODE->command_string), successStr)
#else
#define PRINT_CMD(NODE) DEBUG_PRINT ("<%s> %s", (char *)(NODE->command_string), successStr)
#endif

    command_node *tmp = NULL, *repeating = NULL;
    command_node *CmdNode = NULL;  // 要被初始化的节点
    if (cmdStr == NULL) {
        DEBUG_PRINT ("The command is null, exit func:<%s>", __func__);
        return lastError = NODE_ARG_ERR;
    }

#if NODE_DEBUG
    (isWch)
        ? wprintf (L"(RegisterCommand)>>isWch--%ls\n", (wchar_t *)cmdStr)
        : DEBUG_PRINT ("(RegisterCommand)>>--%s\n", (char *)cmdStr);
#endif
    if (FristNode == NULL)  // 首次注册命令节点
    {
        FristNode = (command_node *)malloc (sizeof (command_node));
        CHECK_BUF (FristNode);
        FristNode->prev = FristNode;
        FristNode->next = FristNode;
        FristNode->ParameterNode_head = NULL;
        if (AssignCommandNodeStr (FristNode, isWch, cmdStr)) {
            free (FristNode);
            return lastError;
        }
#if NODE_DEBUG
        PRINT_CMD (FristNode);
#endif
        return lastError = NODE_OK;
    } else {
        CmdNode = (command_node *)malloc (sizeof (command_node));
        CHECK_BUF (CmdNode);
        CmdNode->ParameterNode_head = NULL;
        if (AssignCommandNodeStr (CmdNode, isWch, cmdStr)) {
            free (CmdNode);
            return lastError;
        }

        // 寻找是否有相同命令的节点, 当前节点尚未接入链表, 故此不会影响寻找
        repeating = (isWch)
                        ? FindCommand (NULL, CmdNode->command_string)
                        : FindCommand ((char *)(CmdNode->command_string), NULL);
        if (repeating != NULL) {
            DEBUG_PRINT ("The same command already exists...");
            free (CmdNode);
            return lastError = NODE_REPEATING;
        }
        tmp = CommandFinalNode();

        // 节点接驳
        CmdNode->prev = tmp;        // this to currentEnd
        tmp->next = CmdNode;        // currentEnd to this
        CmdNode->next = FristNode;  // currentEnd to Frist
        FristNode->prev = CmdNode;  // Frist to currentEnd

#if NODE_DEBUG
        PRINT_CMD (CmdNode);
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
int unRegisterAllParameters (command_node *node) {
    parameter_node *tmp = NULL, *nextNode = NULL;
    if (node == NULL)
        return lastError = NODE_ARG_ERR;
    else
        tmp = node->ParameterNode_head;

    if (tmp == NULL) {
#if COMMAND_SUPPORT_WCAHR
        (node->isWch)
            ? wprintf (L"<%ls>(isWch) ParameterNode is NULL.\n", node->command_string)
            : DEBUG_PRINT ("<%s> ParameterNode is NULL.\n", (char *)(node->command_string));
#else
        DEBUG_PRINT ("<%s> ParameterNode is NULL.", (char *)(node->command_string));
#endif
        return lastError = NODE_OK;
    }

    do {
        nextNode = tmp->next;  // 保存下一节点地址
        if (nextNode == NULL)
            return lastError = NODE_PARAM_NODE_NULL;
        tmp->prev = NULL;
        tmp->next = NULL;
        free (tmp);                   // 释放当前节点
        tmp = nextNode;               // 切换到下一节点
    } while (tmp != node->ParameterNode_head);
    node->ParameterNode_head = NULL;  // 此时已经释放完毕

#if COMMAND_SUPPORT_WCAHR
    (node->isWch)
        ? wprintf (L"<%ls>(isWch) ParameterNodes unregister finish.\n", node->command_string)
        : DEBUG_PRINT ("<%s> ParameterNodes unregister finish.\n", (char *)(node->command_string));
#else
    DEBUG_PRINT ("<%s> ParameterNodes unregister finish.", (char *)(node->command_string));
#endif
    return lastError = NODE_OK;
}

/**
 * @brief 寻找参数节点
 * @param node 命令节点
 * @param paramStr 参数
 * @return
 */
static parameter_node *FindParameter (const command_node *node,
                                      const void *paramStr) {
    parameter_node *paramNode = NULL;
    char *str = NULL;
    wchar_t *strW = NULL;
    size_t len = 0;
    if (node == NULL) {
        lastError = NODE_PARAM_NODE_NULL;
        return NULL;
    }

    if (paramStr == NULL) {
        lastError = NODE_ARG_ERR;
        return NULL;
    }


    paramNode = node->ParameterNode_head;
    switch ((int)(node->isWch)) {
    case false: {
        str = (char *)paramStr;
        len = strlen (str);

        if (len > (MAX_PARAMETER * sizeof (wchar_t) - 1)) {
            lastError = NODE_PARAM_TOO_LONG;
            return NULL;
        }

        do  // 即便只有一个 node 也要比较
        {
            if (strcmp ((char *)(paramNode->parameter_string), str) == 0)
                return paramNode;
            paramNode = paramNode->next;
        } while (paramNode != node->ParameterNode_head);
    } break;
    case true: {
#if COMMAND_SUPPORT_WCAHR
        strW = (wchar_t *)paramStr;
        len = wcslen (strW);

        if (len > (MAX_PARAMETER - 1)) {
            lastError = NODE_PARAM_TOO_LONG;
            return NULL;
        }

        do  // 即便只有一个 node 也要比较
        {
            if (wcscmp (paramNode->parameter_string, strW) == 0)
                return paramNode;
            paramNode = paramNode->next;
        } while (paramNode != node->ParameterNode_head);
#endif
    } break;
    }

    lastError = NODE_NOT_FIND_PARAM;
    return NULL;  // 遍历后均无所获
}

/**
 * @brief 取消注册命令
 * @param command 取决于注册节点时是否使用了宽字符, 若未使用宽字符, 另一个则填 NULL
 * @param commandW 目标命令( 宽字符 )
 * @return OK: NODE_OK
 * @return ERROR: NODE_FAIL, NODE_ARG_ERR, NODE_CMD_NODE_NULL, NODE_NOT_YET_INIT,
 * NODE_NOT_FIND_CMD
 */
int unRegisterCommand (const char *command, const wchar_t *commandW) {
    command_node *CmdNode = NULL;  // 要被删除的命令节点
    char cmdArr[MAX_COMMAND * sizeof (wchar_t)] = {0};
    wchar_t cmdArrW[MAX_COMMAND] = {0};
    int ret = 0;

    if (command == NULL && commandW == NULL) {
        DEBUG_PRINT ("The command is null, exit func:<%s>", __func__);
        return lastError = NODE_ARG_ERR;
    }

    if (FristNode == NULL) {
        if (command)
            DEBUG_PRINT ("<%s>Command node does not yet exist", command);
#if COMMAND_SUPPORT_WCAHR
        if (commandW)
            wprintf (L"<%ls>Command node does not yet exist\n", commandW);
#endif
        return lastError = NODE_CMD_NODE_NULL;
    }

    if (command != NULL) {
        strcpy (cmdArr, command);
        UppercaseToLowercase (cmdArr);
        CmdNode = FindCommand (cmdArr, NULL);
    } else if (commandW != NULL) {
#if COMMAND_SUPPORT_WCAHR
        wcscpy (cmdArrW, commandW);
        UppercaseToLowercaseW (cmdArrW);
        CmdNode = FindCommand (NULL, cmdArrW);
#else
        lastError = NODE_FAIL;
        return lastError;
#endif
    } else {
        return lastError = NODE_FAIL;
    }


    if (CmdNode != NULL)  // 通过 command 寻找 CommandNode
    {
        // 若只剩下头节点
        if (CmdNode->prev == CmdNode && CmdNode->next == CmdNode)
            FristNode = NULL;

        // 更改当前节点的邻居节点对本节点的指针
        ((command_node *)(CmdNode->prev))->next = CmdNode->next;
#if NODE_DEBUG
        printCmdNode_command (CmdNode->prev);
#endif
        if (CmdNode == FristNode)  // 若头节点被删除, 更新头节点
        {
            FristNode = CmdNode->next;
#if NODE_DEBUG
            DEBUG_PRINT ("update FristNode, %p\n", FristNode);
#endif
        }


        ((command_node *)(CmdNode->next))->prev = CmdNode->prev;
#if NODE_DEBUG
        printCmdNode_command (CmdNode->next);
#endif

        ret = unRegisterAllParameters (CmdNode);
        if (ret == NODE_OK) {
#if COMMAND_SUPPORT_WCAHR
            (CmdNode->isWch)
                ? wprintf (L"<%ls> CommandNodes unregister finish.\n",
                           CmdNode->command_string)
                : DEBUG_PRINT ("<%s> CommandNodes unregister finish.\n",
                               (char *)(CmdNode->command_string));
#else
            DEBUG_PRINT ("<%s> CommandNodes unregister finish.",
                         (char *)(CmdNode->command_string));
#endif
            free (CmdNode);
            return lastError = NODE_OK;
        }
    } else {
        if (command)
            DEBUG_PRINT ("<%s>not find ", command);
#if COMMAND_SUPPORT_WCAHR
        if (commandW)
            wprintf (L"<%ls>not find \n", commandW);
#endif
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
int updateCommand (char *oldCommand, wchar_t *oldCommandW,
                   char *newCommand, wchar_t *newCommandW) {
    command_node *CmdNode = NULL;
    int ret = NODE_OK;
    if (oldCommand) {
        if (newCommand == NULL)
            return lastError = NODE_ARG_ERR;
    } else if (oldCommandW) {
        if (newCommandW == NULL)
            return lastError = NODE_ARG_ERR;
    } else {
        return lastError = NODE_FAIL;
    }

    if (oldCommand) {
        ret = UppercaseToLowercase (oldCommand);
        ERR_CHECK (ret);

        ret = UppercaseToLowercase (newCommand);
        ERR_CHECK (ret);

        // 寻找目标命令节点
        CmdNode = FindCommand (oldCommand, NULL);
        if (CmdNode == NULL) {
            DEBUG_PRINT ("<%s>not find", oldCommand);
            return lastError = NODE_NOT_FIND_CMD;
        }

        // 判断是否已经存在新命令节点
        if (FindCommand (newCommand, NULL) != NULL) {
            DEBUG_PRINT ("The same command<%s> already exists, \
it has reverted to the old command<%s> name.",
                         newCommand, oldCommand);
            return lastError = NODE_REPEATING;
        }

        AssignCommandNodeStr (CmdNode, CmdNode->isWch, newCommand);
        return lastError = NODE_OK;
    }
#if COMMAND_SUPPORT_WCAHR
    if (oldCommandW) {
        ret = UppercaseToLowercaseW (oldCommandW);
        ERR_CHECK (ret);

        ret = UppercaseToLowercaseW (newCommandW);
        ERR_CHECK (ret);

        // 寻找目标命令节点
        CmdNode = FindCommand (NULL, oldCommandW);
        if (CmdNode == NULL) {
            wprintf (L"<%ls>not find\n", oldCommandW);
            return lastError;
        }

        // 判断是否已经存在新命令节点
        if (FindCommand (NULL, oldCommandW)) {
            wprintf (L"The same command<%ls> already exists, \
it has reverted to the old command<%s> name.\n",
                     newCommandW, oldCommandW);
            return lastError = NODE_REPEATING;
        }

        AssignCommandNodeStr (CmdNode, CmdNode->isWch, newCommandW);
        return lastError = NODE_OK;
    }
#endif
    return lastError = NODE_FAIL;
}

/**
 * @brief 取消注册全部命令
 * @param
 * @return OK: NODE_OK
 * @return ERROR: NODE_FAIL, NODE_NOT_YET_INIT
 */
int unRegisterAllCommand (void) {
    command_node *CmdNode = FristNode;
    command_node *nextNode = NULL;
    if (CmdNode == NULL) {
        DEBUG_PRINT ("Command not yet created");
        return lastError = NODE_NOT_YET_INIT;
    }

    do {
        lastError = unRegisterAllParameters (CmdNode);
        if (lastError != NODE_OK) {
            FristNode = CmdNode;
            DEBUG_PRINT ("The delete command encountered an unknown failure");
            return lastError;
        }
#if COMMAND_SUPPORT_WCAHR
        (CmdNode->isWch)
            ? wprintf (L"<%ls>(isWch) deleted\n", CmdNode->command_string)
            : DEBUG_PRINT ("<%s> deleted\n", (char *)(CmdNode->command_string));
#else
        DEBUG_PRINT ("<%s> deleted", (char *)(CmdNode->command_string));
#endif

        nextNode = CmdNode->next;  // 先保存 next 节点地址
        CmdNode->prev = NULL;      // 断开 prev 节点
        CmdNode->next = NULL;      // 断开 next 节点
        free (CmdNode);            // 释放当前节点
        CmdNode = nextNode;        // 节点移动
    } while (CmdNode != FristNode);


    FristNode = NULL;
    DEBUG_PRINT ("All commands deleted successfully");
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
static int AssignParameterNodeStr (parameter_node *node,
                                   const bool isWch,
                                   const void *paramStr) {
    char *str = NULL;
    wchar_t *strW = NULL;
    size_t len = 0;

    if (node == NULL)
        return lastError = NODE_ARG_ERR;

    if (paramStr == NULL)
        return lastError = NODE_ARG_ERR;

    switch ((int)isWch) {
    case false: {
        str = (char *)paramStr;
        len = strlen (str);

        if (len > (MAX_PARAMETER * sizeof (wchar_t) - 1))
            return lastError = NODE_PARAM_TOO_LONG;
        else {
            // 检查参数是否有非法字符
            for (size_t i = 0; i < len; i++) {
                if (passableChParam (*(str + i)))
                    return lastError = NODE_ARG_ERR;
            }
            memset (node->parameter_string, 0, sizeof (node->parameter_string));
            strcpy ((char *)(node->parameter_string), str);
        }
    } break;
    default: {
#if COMMAND_SUPPORT_WCAHR
        strW = (wchar_t *)paramStr;
        len = wcslen (strW);

        if (len > (MAX_PARAMETER - 1))
            return lastError = NODE_PARAM_TOO_LONG;
        else {
            for (size_t i = 0; i < len; i++) {
                // 检查参数是否有非法字符
                if (passableChParamW (*(strW + i)))
                    return lastError = NODE_ARG_ERR;
            }
        }
        memset (node->parameter_string, 0, sizeof (node->parameter_string));
        wcscpy (node->parameter_string, strW);
#endif
    } break;
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
int RegisterParameter (command_node *node,
                       ParameterHandler hook,
                       const bool isRaw,
                       const void *paramStr) {
    parameter_node *paramNode = NULL, *tmp = NULL;
    const char *warningStr = "Not the same configuration(isWch)\
 as the command node...";
    if (node == NULL)  // 若传入的 node 为 NULL 则会访问错误的地址, 所以先判断
        return lastError = NODE_CMD_NODE_NULL;

    if (paramStr == NULL) {
        DEBUG_PRINT ("The parameter is null, exit func:<%s>", __func__);
        return lastError = NODE_ARG_ERR;
    }

    paramNode = node->ParameterNode_head;
    if (paramNode == NULL) {
        paramNode = (parameter_node *)malloc (sizeof (parameter_node));
        CHECK_BUF (paramNode);
        if (AssignParameterNodeStr (paramNode, node->isWch, paramStr) != NODE_OK) {
            free (paramNode);
            return lastError;
        }
        paramNode->handler = hook;
        paramNode->isRawStr = isRaw;

        paramNode->prev = node;
        paramNode->next = paramNode;
        node->ParameterNode_head = paramNode;
#if NODE_DEBUG
        (node->isWch)
            ? wprintf (L"<%ls><%ls> %ls\n",
                       node->command_string, (wchar_t *)(paramStr), successStrW)
            : DEBUG_PRINT ("<%s><%s> %s\n",
                           (char *)(node->command_string), (char *)(paramStr), successStr);
#endif
        return lastError = NODE_OK;
    } else {
        paramNode = FindParameter (node, paramStr);
        if (paramNode != NULL)  // 参数名称重复检查
        {
            DEBUG_PRINT ("The same parameter already exists...");
            return lastError = NODE_REPEATING;
        }
        paramNode = (parameter_node *)malloc (sizeof (parameter_node));
        CHECK_BUF (paramNode);
        if (AssignParameterNodeStr (paramNode, node->isWch, paramStr) != NODE_OK) {
            free (paramNode);
            return lastError;
        }
        paramNode->handler = hook;
        paramNode->isRawStr = isRaw;
        tmp = ParameterFinalNode (node);

        paramNode->prev = tmp;
        tmp->next = paramNode;
        paramNode->next = node->ParameterNode_head;
#if NODE_DEBUG
        (node->isWch)
            ? wprintf (L"<%ls><%ls> %ls\n",
                       node->command_string, (wchar_t *)(paramStr), successStrW)
            : DEBUG_PRINT ("<%s><%s> %s\n",
                           (char *)(node->command_string), (char *)(paramStr), successStr);
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
int unRegisterParameter (command_node *node,
                         const void *paramStr) {
    parameter_node *paramNode = NULL, *tmp = NULL;
    if (node == NULL)
        return lastError = NODE_ARG_ERR;

    if (paramStr == NULL)
        return lastError = NODE_ARG_ERR;

    paramNode = FindParameter (node, paramStr);
    if (paramNode)  // 通过 parameter 寻找 ParameterNode
    {
        tmp = ParameterFinalNode (node);
        if (paramNode == node->ParameterNode_head) {
            // 头节点被移除, 更新头节点
            node->ParameterNode_head = paramNode->next;
            ((parameter_node *)(node->ParameterNode_head))->prev = node;

            // 更新末端节点
            tmp->next = node->ParameterNode_head;
        } else {
            // 更改当前节点的邻居节点对本节点的指针
            ((parameter_node *)(paramNode->prev))->next = paramNode->next;
            ((parameter_node *)(paramNode->next))->prev = paramNode->prev;
        }

        paramNode->next = NULL;
        paramNode->prev = NULL;
#if COMMAND_SUPPORT_WCAHR
        (node->isWch)
            ? wprintf (L"<%ls> %ls, parameterNode unregister finish.\n",
                       node->command_string, paramNode->parameter_string)
            : DEBUG_PRINT ("<%s> %s, parameterNode unregister finish.\n",
                           (char *)(node->command_string), (char *)(paramNode->parameter_string));
#else
        DEBUG_PRINT ("<%s> %s, parameterNode unregister finish.",
                     (char *)(node->command_string), (char *)(paramNode->parameter_string));
#endif

        free (paramNode);
        return lastError = NODE_OK;
    } else {
#if COMMAND_SUPPORT_WCAHR
        (node->isWch)
            ? wprintf (L"<%s>, not find parameterNode\n", node->command_string)
            : DEBUG_PRINT ("<%s>, not find parameterNode\n", (char *)(node->command_string));
#else
        DEBUG_PRINT ("<%s>, not find parameterNode", (char *)(node->command_string));
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
int updateParameter (const command_node *CmdNode, ParameterHandler hook,
                     const bool isRaw, const void *oldParam, const void *newParam) {
    const char *charWarning = "this string has illegal character...";
    parameter_node *ParamNode = NULL;
    char *oldStr = NULL, *newStr = NULL;
    wchar_t *oldStrW = NULL, *newStrW = NULL;
    size_t len = 0;
    if (CmdNode == NULL)
        return lastError = NODE_ARG_ERR;


    if (oldParam == NULL) {
        if (newParam == NULL)
            return lastError = NODE_ARG_ERR;
    } else {
        if (newParam == NULL)
            return lastError = NODE_ARG_ERR;
    }

    // 字符串合法检查
    switch ((int)(CmdNode->isWch)) {
    case false: {
        oldStr = (char *)oldParam;
        if (oldStr == NULL)
            return lastError = NODE_ARG_ERR;

        len = strlen (oldStr);
        if (len > (MAX_PARAMETER * sizeof (wchar_t) - 1))
            return lastError = NODE_ARG_ERR;

        for (size_t i = 0; i < len; i++) {
            if (passableChParam (*(oldStr + i))) {
                DEBUG_PRINT ("%s", charWarning);
                return lastError = NODE_ARG_ERR;
            }
        }

        newStr = (char *)newParam;
        if (newStr == NULL)
            return lastError = NODE_ARG_ERR;

        len = strlen (newStr);
        if (len > (MAX_PARAMETER * sizeof (wchar_t) - 1))
            return lastError = NODE_ARG_ERR;

        for (size_t i = 0; i < len; i++) {
            if (passableChParam (*(newStr + i))) {
                DEBUG_PRINT ("%s", charWarning);
                return lastError = NODE_ARG_ERR;
            }
        }
    } break;
    default: {
#if COMMAND_SUPPORT_WCAHR
        oldStrW = (wchar_t *)oldParam;
        if (oldStrW == NULL)
            return lastError = NODE_ARG_ERR;

        len = wcslen (oldStrW);
        if (len > (MAX_PARAMETER - 1))
            return lastError = NODE_ARG_ERR;

        for (size_t i = 0; i < len; i++) {
            if (passableChParamW (*(oldStrW + i))) {
                DEBUG_PRINT ("%s...\n", charWarning);
                return lastError = NODE_ARG_ERR;
            }
        }

        newStrW = (wchar_t *)newParam;
        if (newStrW == NULL)
            return lastError = NODE_ARG_ERR;

        len = wcslen (newStrW);
        if (len > (MAX_PARAMETER - 1))
            return lastError = NODE_ARG_ERR;

        for (size_t i = 0; i < len; i++) {
            if (passableChParamW (*(newStrW + i))) {
                DEBUG_PRINT ("%s\n", charWarning);
                return lastError = NODE_ARG_ERR;
            }
        }
#endif
    } break;
    }


    // 寻找旧参数
    ParamNode = FindParameter (CmdNode, oldParam);
    if (ParamNode == NULL) {
#if COMMAND_SUPPORT_WCAHR
        (CmdNode->isWch)
            ? wprintf (L"<%ls> <%ls>not find\n", CmdNode->command_string, (wchar_t *)oldParam)
            : DEBUG_PRINT ("<%s> <%s>not find\n", (char *)(CmdNode->command_string), (char *)oldParam);
#else
        DEBUG_PRINT ("<%s> <%s>not find", (char *)(CmdNode->command_string), (char *)oldParam);
#endif
        return lastError = NODE_NOT_FIND_PARAM;
    }

    // 检查新参数是否已经存在
    if (FindParameter (CmdNode, newParam) != NULL) {
#if COMMAND_SUPPORT_WCAHR
        (CmdNode->isWch)
            ? wprintf (L"The same param<%ls> already exists,\
 it has reverted to the old param<%ls> name.\n",
                       (wchar_t *)newParam, (wchar_t *)oldParam)
            : DEBUG_PRINT ("The same param<%s> already exists,\
 it has reverted to the old param<%s> name.\n",
                           (char *)newParam, (char *)oldParam);
#else
        DEBUG_PRINT ("The same param<%s> already exists,\
 it has reverted to the old param<%s> name.",
                     (char *)newParam, (char *)oldParam);
#endif
        return lastError = NODE_REPEATING;
    }

    // 修改参数
    lastError = AssignParameterNodeStr (ParamNode, CmdNode->isWch, newParam);
    if (lastError != NODE_OK)
        return lastError;
    ParamNode->handler = hook;
    return lastError;
}

/**
 * @brief 获得命令节点图
 * @param map 节点图的首地址
 * @return map's len
 */
int NodeGetCommandMap (command_info **map) {
    command_info *tmp = *map;
    command_node *current = FristNode;
    int len = 0;

    if (current == NULL) {
        DEBUG_PRINT ("<%s>list null...", __func__);
        return 0;
    }

    do {
        len++;
        tmp = *map;  // 先保存 map , 预防 realloc 失败导致内存泄露
        *map = (command_info *)realloc (tmp, len * sizeof (command_info));
        if (*map == NULL) {
            lastError = NODE_ALLOC_ERR;
            free (tmp);
            return 0;
        }

        (*map + len - 1)->command = (void *)(current->command_string);
        (*map + len - 1)->node = (void *)current;
        current = current->next;
    } while (current != FristNode);
    return len;
}
#endif

/**
 * @brief 解析空格中混含的用户参数
 * @param userParam 用户参数
 * @return 保存用户参数的首地址
 */
static void *ParseSpace (const char *userParam) {
    const char space = ' ';
    size_t passLen = 0;  // 已经处理的长度
    const size_t passConstant = COMMAND_SIZE - userDataPass;
    userString *tmp = NULL;
    const char *str = userParam, *str2 = NULL;
    if (userParam == NULL)
        return NULL;

    passLen = passConstant;
    do {
        // 在指令支持的最大长度内跳过所空格, 先检查是否还有有效字符
        for (; passableChParam (*str) && str < userParam + passConstant; str++);
        if (str >= userParam + passConstant) {
#if NODE_DEBUG
            DEBUG_PRINT ("--<%s>%d--too far:%lld, strAdd:%p, tooFarAdd:%p\n",
                         __func__, __LINE__, userParam + passConstant - str, str, userParam + passConstant);
#endif
            return userData;  // 超出可解析的长度
        }

        userDataCnt++;
        passLen += str - userParam;
        userData = (userString *)realloc (tmp, userDataCnt * sizeof (userString));
#if 0
        if ( userData == tmp )
        {
            free(userData);
            return userData = NULL;
        }
#endif  // 此处有个拿捏不定的 bug, 若 userDataCnt 在结束一次 CommandParse 后没有置0的情况下,
        // 会导致 realloc 失败, 但返回的是原先的地址
        if (userData == NULL) {
            DEBUG_PRINT ("<%s>alloc memory fail", __func__);
            free (tmp);
            return (void *)userData;
        }

        (userData + userDataCnt - 1)->strHead = (void *)str;  // 字符串头

        str2 = str;
        for (; *str2 != space &&
               str2 < userParam + passConstant &&
               *str2 != '\0';
             str2++);  // 略过非空格
        if (str2 > (passConstant + userParam)) {
            // 超出了限定的长度
            DEBUG_PRINT ("<%s>userParam is too long( %u, %p )...\
 the end string has no end",
                         __func__, COMMAND_SIZE,
                         (void *)(str2 - (passConstant + userParam)));
            (userData + userDataCnt - 1)->len =
                (size_t)((passConstant + userParam) - str);
            return (void *)userData;
        }

        (userData + userDataCnt - 1)->len = (size_t)(str2 - str);
        str += (userData + userDataCnt - 1)->len;

        tmp = userData;
#if NODE_DEBUG
        DEBUG_PRINT ("<%s>len:%llu, |%s|\n", __func__,
                     (userData + userDataCnt - 1)->len, (char *)((userData + userDataCnt - 1)->strHead));
#endif
    } while (str <= userParam + passConstant || *str2 == '\0');
    return (void *)userData;
}

#if NODE_LINK_LIST
/**
 * @brief 获得解析到的用户参数个数
 * @return
 */
size_t NodeGetUserParamsCnt() {
    return userDataCnt;
}

/**
 * @brief 命令解析
 * @param commandString 用户输入的字符串
 * @return OK: NODE_OK
 * @return ERROR: NODE_ARG_ERR, NODE_CMD_TOO_LONG, NODE_PARAM_TOO_LONG,
 * NODE_NOT_FIND_CMD, NODE_NOT_FIND_PARAM, NODE_PARSE_ERR
 */
int CommandParse (const char *commandString) {
    const char *spaceStr = " ";
    command_node *CmdNode = NULL;
    parameter_node *ParamNode = NULL;
    char cmd[MAX_COMMAND * sizeof (wchar_t)] = {0};
    char param[MAX_PARAMETER * sizeof (wchar_t)] = {0};
    const char *tmp = NULL, *tmp2 = NULL;
    size_t len = 0;
    if (commandString == NULL)
        return lastError = NODE_ARG_ERR;


    tmp2 = commandString;  // 若字符串开头有空格, 跳过
#if NODE_DEBUG
    DEBUG_PRINT ("<%s>|%s|\n", __func__, tmp2);
#endif
    for (; passableChParam (*tmp2) && tmp2 < commandString + COMMAND_SIZE; tmp2++);
    userDataPass += tmp2 - commandString;
#if NODE_DEBUG
    DEBUG_PRINT ("<%s>|%s|\n", __func__, tmp2);
#endif

    // 寻找命令与参数的间隔字符
    tmp = strstr (tmp2, spaceStr);
    if (tmp == NULL) {
        // 计算长度是否超出
        len = strlen (tmp2);
        if (len >= MAX_COMMAND) {
            DEBUG_PRINT ("Entered command is too long...");
            return lastError = NODE_CMD_TOO_LONG;
        }

        // 直接把源字符串扔进去寻找目标命令
        CmdNode = FindCommand (tmp2, NULL);
        if (CmdNode == NULL) {
            DEBUG_PRINT ("<%s>The command was not found.", tmp2);
            return lastError;
        }

        DEBUG_PRINT ("<%s>There is no input parameter for this command...", tmp2);
        showParam (CmdNode);
        return lastError = NODE_PARSE_ERR;
    }

    len = tmp - tmp2;                   // 命令的长度
    memcpy (cmd, tmp2, len);            // 复制命令
    userDataPass += len;                // 将命令的长度也算进去, 等会在 ParseSpace 要用
    CmdNode = FindCommand (cmd, NULL);  // 寻找命令
    if (CmdNode == NULL) {
        DEBUG_PRINT ("<%s>The command was not found.", cmd);
        return lastError;
    }

    tmp2 = tmp;
    // 跳过所有不支持字符的地址
    for (; passableChParam (*tmp2) && tmp2 < commandString + COMMAND_SIZE; tmp2++);
    userDataPass += tmp2 - tmp;  // 累加跳过的空格
    if (tmp2 == commandString + COMMAND_SIZE) {
        DEBUG_PRINT ("<%s>There is no input parameter for this command...", cmd);
        showParam (CmdNode);
        return lastError = NODE_ARG_ERR;
    }
    tmp = strstr (tmp2, spaceStr);
    // 已经没有' '的情况
    if (tmp == NULL) {
        ParamNode = FindParameter (CmdNode, tmp2);
        if (ParamNode == NULL) {
            DEBUG_PRINT ("<%s><%s>Parameter not found in current command...",
                         cmd, tmp2);
            return lastError;
        }
        ParamNode->handlerArg = NULL;
        if (ParamNode->handler == NULL) {
            DEBUG_PRINT ("<%s><%s>No handler function", cmd, tmp2);
            return lastError = NODE_OK;
        }

        (ParamNode->handler) (ParamNode->handlerArg);  // 调用当前参数处理
        return lastError = NODE_OK;
    }
#if NODE_DEBUG
    DEBUG_PRINT ("--<%s>%d--\n", __func__, __LINE__);
#endif

    // 还有' '的情况
    len = tmp - tmp2;  // 参数的长度
    if (len >= MAX_PARAMETER) {
        DEBUG_PRINT ("Entered parameter is too long...");
        return lastError = NODE_PARAM_TOO_LONG;
    }
    memcpy (param, tmp2, len);                   // 复制参数
    userDataPass += len;                         // 将参数的长度也算进去, 等会在 ParseSpace 要用
    ParamNode = FindParameter (CmdNode, param);  // 寻找参数
    if (ParamNode == NULL) {
        DEBUG_PRINT ("<%s><%s>Parameter not found in current command...", cmd, param);
        return lastError;
    }

    ParamNode->handlerArg = (ParamNode->isRawStr)
                                ? (void *)tmp
                                : ParseSpace (tmp);
#if NODE_DEBUG
    DEBUG_PRINT ("--<%s>%d--\n", __func__, __LINE__);
#endif
    if (ParamNode->handler == NULL) {
        DEBUG_PRINT ("<%s><%s>No handler function", cmd, param);
        return lastError = NODE_OK;
    }
    (ParamNode->handler) (ParamNode->handlerArg);  // 调用当前参数处理
    free (userData);                               // userData 传递给用户处理函数并处理结束后释放

    ParamNode->handlerArg = NULL;                  // 清空指针
    userData = NULL;                               // 复位
    userDataCnt = 0;
    userDataPass = 0;
    return lastError = NODE_OK;
}
#ifdef WCHAR_MIN
#ifdef WCHAR_MAX
#if COMMAND_SUPPORT_WCAHR
/**
 * @brief 解析空格中混含的用户参数( 宽字符 )
 * @param userParam 用户参数
 * @return 保存用户参数的首地址
 */
static void *ParseSpaceW (const wchar_t *userParam) {
    const wchar_t space = L' ';
    size_t passLen = 0;  // 已经处理的长度
    const size_t passConstant = (COMMAND_SIZE * 2U) - (MAX_COMMAND * 2U) -
                                (MAX_PARAMETER * 2U) - userDataPass;
    userString *tmp = NULL;
    const wchar_t *str = userParam, *str2 = NULL;
    if (userParam == NULL)
        return NULL;

    passLen = passConstant;
    do {
        // 在指令支持的最大长度内跳过所空格, 先检查是否还有有效字符
        for (; passableChParamW (*str) && str < userParam + passConstant; str++);
        if (str >= userParam + passConstant) {
#if NODE_DEBUG
            DEBUG_PRINT ("--<%s>%d--too far:%lld, strAdd:%p, tooFarAdd:%p\n",
                         __func__, __LINE__, userParam + passConstant - str, str, userParam + passConstant);
#endif
            return userData;  // 超出可解析的长度
        }


        userDataCnt++;
        passLen += str - userParam;
        userData = (userString *)realloc (tmp, userDataCnt * sizeof (userString));
#if 0
        if ( userData == tmp )
        {
            free(userData);
            return userData = NULL;
        }
#endif  // 被注释的原因同上面一样
        if (userData == NULL) {
            DEBUG_PRINT ("<%s>alloc memory fail\n", __func__);
            free (tmp);
            return (void *)userData;
        }

        (userData + userDataCnt - 1)->strHead = (void *)str;  // 字符串头

        str2 = str;
        for (; *str2 != space &&
               str2 < userParam + passConstant &&
               *str2 != L'\0';
             str2++);  // 略过非空格
        if (str2 > (passConstant + userParam)) {
            // 超出了限定的长度
            DEBUG_PRINT ("<%s>userParam is too long( %u, %p )...\
 the end string has no end\n",
                         __func__, COMMAND_SIZE,
                         (void *)(str2 - (passConstant + userParam)));
            (userData + userDataCnt - 1)->len =
                (size_t)((passConstant + userParam) - str);
            return (void *)userData;
        }

        (userData + userDataCnt - 1)->len = (size_t)(str2 - str);
        str += (userData + userDataCnt - 1)->len;

        tmp = userData;
#if NODE_DEBUG
        DEBUG_PRINT ("<%s>len:%llu\n", __func__, (userData + userDataCnt - 1)->len);
#endif
    } while (str <= userParam + passConstant || *str2 == L'\0');
    return (void *)userData;
}

/**
 * @brief 命令解析( 宽字符 )
 * @param commandString 用户输入的字符串
 * @return OK: NODE_OK
 * @return ERROR: NODE_ARG_ERR, NODE_CMD_TOO_LONG, NODE_PARAM_TOO_LONG,
 * NODE_NOT_FIND_CMD, NODE_NOT_FIND_PARAM, NODE_PARSE_ERR
 */

int CommandParseW (const wchar_t *commandString) {
    const wchar_t *spaceStr = L" ";
    command_node *CmdNode = NULL;
    parameter_node *ParamNode = NULL;
    wchar_t cmd[MAX_COMMAND] = {0};
    wchar_t param[MAX_PARAMETER] = {0};
    const wchar_t *tmp = NULL, *tmp2 = NULL;
    size_t len = 0;
    if (commandString == NULL)
        return lastError = NODE_ARG_ERR;

    tmp2 = commandString;  // 若字符串开头有空格, 跳过
#if NODE_DEBUG
    DEBUG_PRINT ("<%s>|%p|\n", __func__, tmp2);
#endif
    for (; passableChParamW (*tmp2) &&
           tmp2 < commandString + (COMMAND_SIZE / sizeof (wchar_t));
         tmp2++);
    userDataPass += tmp2 - commandString;
#if NODE_DEBUG
    DEBUG_PRINT ("<%s>|%p|\n", __func__, tmp2);
#endif

    // 寻找命令与参数的间隔字符
    tmp = wcsstr (tmp2, spaceStr);
    if (tmp == NULL) {
        // 计算长度是否超出
        len = wcslen (tmp2);
        if (len >= MAX_COMMAND) {
            DEBUG_PRINT ("Entered command is too long...\n");
            return lastError = NODE_CMD_TOO_LONG;
        }

        // 直接把源字符串扔进去寻找目标命令
        CmdNode = FindCommand (NULL, tmp2);
        if (CmdNode == NULL) {
            wprintf (L"<%ls>The command was not found.\n", tmp2);
            return lastError;
        }
        wprintf (L"<%ls>There is no input parameter for this command...\n", tmp2);
        showParam (CmdNode);
        return lastError = NODE_PARSE_ERR;
    }

    len = tmp - tmp2;                          // 命令的长度
    memcpy (cmd, tmp2, len);                   // 复制命令
    userDataPass += (len / sizeof (wchar_t));  // 将命令的长度也算进去, 等会在 ParseSpaceW 要用
    CmdNode = FindCommand (NULL, cmd);         // 寻找命令
    if (CmdNode == NULL) {
        wprintf (L"<%s>The command was not found.\n", cmd);
        showParam (CmdNode);
        return lastError;
    }

    tmp2 = tmp;
    // 跳过所有不支持字符的地址
    for (; passableChParamW (*tmp2) && tmp2 < commandString + COMMAND_SIZE; tmp2++);
    userDataPass += (tmp2 - tmp);  // 累加跳过的空格
    if (tmp2 == commandString + COMMAND_SIZE) {
        wprintf (L"<%ls>There is no input parameter for this command...\n", cmd);
        return lastError = NODE_ARG_ERR;
    }
    tmp = wcsstr (tmp2, spaceStr);
    // 已经没有' '的情况
    if (tmp == NULL) {
        ParamNode = FindParameter (CmdNode, tmp2);
        if (ParamNode == NULL) {
            wprintf (L"<%ls><%ls>Parameter not found in current command...\n", cmd, tmp2);
            return lastError;
        }
        ParamNode->handlerArg = NULL;
        if (ParamNode->handler == NULL) {
            wprintf (L"<%ls><%ls>No handler function\n", cmd, tmp2);
            return lastError = NODE_OK;
        }
        (ParamNode->handler) (ParamNode->handlerArg);  // 调用当前参数处理
        return lastError = NODE_OK;
    }
#if NODE_DEBUG
    DEBUG_PRINT ("--<%s>%d--\n", __func__, __LINE__);
#endif

    // 还有' '的情况
    len = tmp - tmp2;                          // 参数的长度
    userDataPass += (len / sizeof (wchar_t));  // 将命令的长度也算进去, 等会在 ParseSpaceW 要用
    if (len >= MAX_PARAMETER) {
        DEBUG_PRINT ("Entered parameter is too long...\n");
        return lastError = NODE_PARAM_TOO_LONG;
    }
    memcpy (param, tmp2, len);                   // 复制参数
    ParamNode = FindParameter (CmdNode, param);  // 寻找参数
    if (ParamNode == NULL) {
        wprintf (L"<%ls><%ls>Parameter not found in current command...\n", cmd, param);
        return lastError;
    }

    ParamNode->handlerArg = (ParamNode->isRawStr)
                                ? (void *)tmp
                                : ParseSpaceW (tmp);
#if NODE_DEBUG
    DEBUG_PRINT ("--<%s>%d--\n", __func__, __LINE__);
#endif
    if (ParamNode->handler == NULL) {
        wprintf (L"<%ls><%ls>No handler function\n", cmd, param);
        return lastError = NODE_OK;
    }
    (ParamNode->handler) (ParamNode->handlerArg);  // 调用当前参数处理
    free (userData);                               // userData 传递给用户处理函数并处理结束后释放

    ParamNode->handlerArg = NULL;                  // 清空指针
    userData = NULL;                               // 复位
    userDataCnt = 0;
    userDataPass = 0;
    return lastError = NODE_OK;
}
#endif
#endif  // WCHAR_MAX
#endif  // WCHAR_MIN

#endif  // NODE_LINK_LIST
/**
 * @brief 获得上次运行链表管理函数的错误
 * @param
 * @return lastError
 */
int NodeGetLastError (void) {
    switch (lastError) {
    case NODE_OK:
        DEBUG_PRINT ("NODE OK");
        break;
    case NODE_FAIL:
        DEBUG_PRINT ("NODE unknow error");
        break;
    case NODE_ARG_ERR:
        DEBUG_PRINT ("NODE parameter passing error");
        break;
    case NODE_NOT_FIND:
        DEBUG_PRINT ("NODE not find");
        break;
    case NODE_NOT_FIND_CMD:
        DEBUG_PRINT ("NODE not found command");
        break;
    case NODE_NOT_FIND_PARAM:
        DEBUG_PRINT ("NODE not found parameter");
        break;
    case NODE_ALLOC_ERR:
        DEBUG_PRINT ("NODE Alloc error, maybe RAM is not enough...");
        break;
    case NODE_CMD_NODE_NULL:
        DEBUG_PRINT ("NODE this command is null");
        break;
    case NODE_PARAM_NODE_NULL:
        DEBUG_PRINT ("NODE command has no paramNode");
        break;
    case NODE_REPEATING:
        DEBUG_PRINT ("NODE is repeating create");
        break;
    case NODE_CMD_TOO_LONG:
        DEBUG_PRINT ("NODE 'command' is too long");
        break;
    case NODE_PARAM_TOO_LONG:
        DEBUG_PRINT ("NODE 'parameter' is too long");
        break;
    case NODE_PARSE_ERR:
        DEBUG_PRINT ("NODE parsing string error");
        break;
    case NODE_NOT_YET_INIT:
        DEBUG_PRINT ("NODEs have not been initialized...");
        break;
    default:
        break;
    }
    return lastError;
}
#if ENABLE_REG
/**
 * @brief 输入指令 注册命令
 * @param arg
 */
static void regCmd (void *arg) {
    userString *data = (userString *)arg;
    char cmd[MAX_COMMAND] = {0};
    int err = NODE_OK;
    command_node *cmdNode = NULL;
    if (data == NULL) {
        DEBUG_PRINT ("<%s>userParam error", __func__);
        return;
    }

    memcpy (cmd, data->strHead, data->len);
    DEBUG_PRINT ("parse cmd:<%s>", cmd);
    cmdNode = FindCommand (cmd, NULL);
    err = NodeGetLastError();
    if (cmdNode == NULL && (err == NODE_NOT_YET_INIT || err == NODE_NOT_FIND_CMD)) {
        RegisterCommand (0, cmd);
        NodeGetLastError();
    } else {
        DEBUG_PRINT ("<%s>The command already exists", cmd);
    }
    return;
}

/**
 * @brief 输入指令 注册命令的参数
 * @param arg
 */
static void regParam (void *arg) {
    userString *data = (userString *)arg;
    char *endptr = NULL;
    void *func = NULL;
    unsigned long long funcAdd = 0;
    char cmdArr[MAX_COMMAND] = {0},
         paramArr[MAX_PARAMETER] = {0},
         funcArr[32] = {0};
    command_node *cmdNode = NULL;
    size_t paramPos = 0, paramCnt = NodeGetUserParamsCnt();
    if (data == NULL) {
        DEBUG_PRINT ("<%s>userParam error", __func__);
        return;
    }

    if (paramCnt < 2) {
        DEBUG_PRINT ("<%s>Insufficient input parameters...", __func__);
        return;
    }

    memcpy (cmdArr, (data + paramPos)->strHead, (data + paramPos)->len);
    DEBUG_PRINT ("<%s>parse cmd:<%s>", __func__, cmdArr);

    paramPos++;
    memcpy (paramArr, (data + paramPos)->strHead, (data + paramPos)->len);
    DEBUG_PRINT ("<%s>parse param:<%s>", __func__, paramArr);

    // 有额外参数, 默认是地址参数
    if (paramCnt > 2) {
        paramPos++;
        memcpy (funcArr, (data + paramPos)->strHead, (data + paramPos)->len);
        funcAdd = strtoull (funcArr, &endptr, 16);
        func = (void *)funcAdd;
        DEBUG_PRINT ("<%s>parse funcAdd:<%llx><%s>", __func__, funcAdd, endptr);
    }

    // 寻找目标命令
    cmdNode = FindCommand (cmdArr, NULL);
    if (cmdNode == NULL) {
        NodeGetLastError();
        return;
    }

    if (strcmp (paramArr, "exit") == 0) {
        RegisterParameter (cmdNode, NULL, 1, paramArr);
        return;
    }

    (paramCnt > 2)
        ? RegisterParameter (cmdNode, func, 0, paramArr)
        : RegisterParameter (cmdNode, NULL, 0, paramArr);
    return;
}

/**
 * @brief 输入指令 删除指定的命令
 * @param arg
 */
static void regDelCmd (void *arg) {
    char cmd[MAX_COMMAND] = {0};
    userString *data = (userString *)arg;
    if (data == NULL) {
        DEBUG_PRINT ("<%s>userParam error", __func__);
        return;
    }

    memcpy (cmd, data->strHead, data->len);
    if (strcmp (cmd, DEFAULT_CMD) == 0) {
        DEBUG_PRINT ("<%s>cannot be deleted command", __func__);
        return;
    }

    if (unRegisterCommand (cmd, NULL)) {
        DEBUG_PRINT ("<%s>", __func__);
        NodeGetLastError();
        return;
    }
}

/**
 * @brief 输入指令 删除指定命令的所有参数
 * @param arg
 */
static void regDelAllParam (void *arg) {
    userString *data = (userString *)arg;
    char cmd[MAX_COMMAND] = {0};
    command_node *cmdNode = NULL;
    if (data == NULL) {
        DEBUG_PRINT ("<%s>userParam error", __func__);
        return;
    }

    memcpy (cmd, data->strHead, data->len);
    if (strcmp (cmd, DEFAULT_CMD) == 0) {
        DEBUG_PRINT ("<%s>cannot be deleted command", __func__);
        return;
    }

    cmdNode = FindCommand (cmd, NULL);
    if (cmdNode == NULL) {
        DEBUG_PRINT ("<%s>", __func__);
        NodeGetLastError();
        return;
    }

    if (unRegisterAllParameters (cmdNode)) {
        DEBUG_PRINT ("<%s>", __func__);
        NodeGetLastError();
        return;
    }
}

/**
 * @brief 输入指令 删除命令中的指定参数
 * @param arg
 */
static void regDelParam (void *arg) {
    userString *data = (userString *)arg;
    char cmdArr[MAX_COMMAND] = {0},
         paramArr[MAX_PARAMETER] = {0};
    size_t dataCnt = NodeGetUserParamsCnt();
    command_node *cmdNode = NULL;
    if (data == NULL) {
        DEBUG_PRINT ("<%s>userParam error", __func__);
        return;
    }

    if (dataCnt < 2) {
        DEBUG_PRINT ("<%s>Insufficient input parameters", __func__);
        return;
    }

    memcpy (cmdArr, data->strHead, data->len);
    DEBUG_PRINT ("<%s>parse cmd:%s", __func__, cmdArr);

    memcpy (paramArr, (data + 1)->strHead, (data + 1)->len);
    DEBUG_PRINT ("<%s>parse param:%s", __func__, paramArr);

    if (strcmp (cmdArr, DEFAULT_CMD) == 0) {
        DEBUG_PRINT ("<%s>cannot be deleted command", __func__);
        return;
    }

    cmdNode = FindCommand (cmdArr, NULL);
    if (cmdNode == NULL) {
        DEBUG_PRINT ("<%s>", __func__);
        NodeGetLastError();
        return;
    }

    if (unRegisterParameter (cmdNode, paramArr)) {
        DEBUG_PRINT ("<%s>", __func__);
        NodeGetLastError();
        return;
    }
}

/**
 * @brief 输入指令 删除所有命令"reg"除外
 * @param arg
 */
static void regDelAllCmd (void *arg) {
    int len = 0;
    command_info *Map = NULL;
    len = NodeGetCommandMap (&Map);
    if (len == 0 || Map == NULL) {
        DEBUG_PRINT ("get list map fail, len:%d, map:%p", len, Map);
        free (Map);
        return;
    }

    for (size_t i = 0; i < len; i++) {
        switch ((int)(((command_node *)((Map + i)->node))->isWch)) {
        case false: {
            if (strcmp ((char *)((Map + i)->command), DEFAULT_CMD) == 0)
                continue;
            else
                unRegisterCommand ((char *)(Map + i)->command, NULL);
        } break;
        default: {
            if (wcscmp ((wchar_t *)((Map + i)->command), DEFAULT_CMD_W) == 0)
                continue;
            else
                unRegisterCommand (NULL, (wchar_t *)(Map + i)->command);
        } break;
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
int defaultRegCmd_init (void) {

    command_node *CmdNode = NULL;
    RegisterCommand (0, DEFAULT_CMD);
    CmdNode = FindCommand (DEFAULT_CMD, NULL);
    if (CmdNode == NULL) {
        DEBUG_PRINT ("<%s>", __func__);
        NodeGetLastError();
        return -1;
    }

    RegisterParameter (CmdNode, regCmd, false, DEFAULT_PARAM_cmd);
    RegisterParameter (CmdNode, regParam, false, DEFAULT_PARAM_param);
    RegisterParameter (CmdNode, regDelAllParam, false, DEFUALT_PARAM_DelAllParam);
    RegisterParameter (CmdNode, regDelCmd, false, DEFAULT_PARAM_DelCmd);
    RegisterParameter (CmdNode, regDelParam, false, DEFUALT_PARAM_DelParam);
    RegisterParameter (CmdNode, regDelAllCmd, false, DEFUALT_PARAM_DelAllCmd);
    RegisterParameter (CmdNode, (ParameterHandler)showList, false, DEFUALT_PARAM_ls);

    return 0;
}
#endif

#define NODE_DEBUG_SIMPLE 0
static commandSimple_node *SimpleCommandTable = NULL;
static int SimpleCommandTableCnt = 0;

int RegisterCMD_simple (void *cmdStr, void *paramStr, const bool isWchar,
                        ParameterHandler cmdHandler, void *cmdHandlerArg, const bool isRawString) {
    commandSimple_node *p, *current;
    if (cmdStr == NULL || paramStr == NULL || cmdHandler == NULL)
        return lastError = NODE_ARG_ERR;

    if (strlen (cmdStr) > MAX_COMMAND)
        return lastError = NODE_CMD_TOO_LONG;
    if (strstr (cmdStr, " ") || strstr (paramStr, " "))
        return lastError = NODE_ARG_ERR;


    SimpleCommandTableCnt++;
    p = realloc (SimpleCommandTable, sizeof (commandSimple_node) * SimpleCommandTableCnt);
    if (!p)
        return lastError = NODE_ALLOC_ERR;

    SimpleCommandTable = p;
    current = &SimpleCommandTable[SimpleCommandTableCnt - 1];
    current->command_string = cmdStr;
    current->parameter_string = paramStr;
    current->handler = cmdHandler;
    current->handlerArg = cmdHandlerArg;
    current->isWch = isWchar;
    current->isRawStr = isRawString;
    return lastError = NODE_OK;
}

commandSimple_node *Findcommand_simple (const void *cmdStr, const void *paramStr, const bool isWchar) {
    int loop = 0, ret = 0;
    commandSimple_node *current = NULL;
    if (cmdStr == NULL || paramStr == NULL) {
        lastError = NODE_ARG_ERR;
        return NULL;
    }


    for (; loop < SimpleCommandTableCnt; loop++) {
        current = &SimpleCommandTable[loop];
#if NODE_DEBUG_SIMPLE
        DEBUG_PRINT ("");
        VAR_PRINT_STRING ((char *)cmdStr);
        VAR_PRINT_STRING ((char *)(current->command_string));
#endif
        ret = strcmp ((char *)(current->command_string), (char *)cmdStr);
        if (ret == 0) {
#if NODE_DEBUG_SIMPLE
            VAR_PRINT_STRING ((char *)paramStr);
            VAR_PRINT_STRING ((char *)(current->parameter_string));
#endif
            ret = strcmp ((char *)(current->parameter_string), (char *)paramStr);
            if (ret == 0)
                return current;
        }
    }
    lastError = NODE_NOT_FIND;
    return NULL;
}

static int showParam_simple (const void *cmdStr) {
    int loop = 0;
    commandSimple_node *current = NULL;

    if (cmdStr == NULL)
        return -1;

    printf ("cmd: {%s}\n", (char *)cmdStr);
    for (; loop < SimpleCommandTableCnt; loop++) {
        current = &SimpleCommandTable[loop];
        if (strcmp (cmdStr, current->command_string) == 0)
            printf ("    %s\n", (char *)(current->parameter_string));
    }

    return loop;
}

int showCmdAndParam_simple (void) {
    int loop = 0;
    commandSimple_node *current = NULL;

    for (; loop < SimpleCommandTableCnt; loop++) {
        current = &SimpleCommandTable[loop];
        printf ("cmd:{%s}, param:{%s}\n",
                (char *)(current->command_string), (char *)(current->parameter_string));
    }
    return loop;
}

int simpleCMD_freeList (void) {
    int loop = 0;
    commandSimple_node *current = NULL;
    for (; loop < SimpleCommandTableCnt; loop++) {
        current = &SimpleCommandTable[loop];
        current->command_string = NULL;
        current->parameter_string = NULL;
        current->handler = NULL;
        current->handlerArg = NULL;
        current->isWch = 0;
        current->isRawStr = 0;
    }
    free (SimpleCommandTable);
    DEBUG_PRINT ("free simple command list...");
    VAR_PRINT_POS (SimpleCommandTable);
    SimpleCommandTable = NULL;
    return NODE_OK;
}

/**
 * @brief 命令解析
 * @param commandString 用户输入的字符串
 * @return OK: NODE_OK
 * @return ERROR: NODE_ARG_ERR, NODE_CMD_TOO_LONG, NODE_PARAM_TOO_LONG,
 * NODE_NOT_FIND_CMD, NODE_NOT_FIND_PARAM, NODE_PARSE_ERR
 */
int CommandParse_simple (const char *commandString, const bool isWchar) {
    const char *spaceStr = " ";
    commandSimple_node *current = NULL;
    char cmd[MAX_COMMAND * sizeof (wchar_t)] = {0};
    char param[MAX_PARAMETER * sizeof (wchar_t)] = {0};
    const char *tmp = NULL, *tmp2 = NULL;
    size_t len = 0;
    if (commandString == NULL)
        return lastError = NODE_ARG_ERR;

    if (SimpleCommandTable == NULL) {
        DEBUG_PRINT ("no registration command...");
        return NODE_NOT_YET_INIT;
    }

    tmp2 = commandString;  // 若字符串开头有空格, 跳过
#if NODE_DEBUG_SIMPLE
    DEBUG_PRINT ("line:%u", __LINE__);
    VAR_PRINT_STRING (tmp2);
#endif
    for (; passableChParam (*tmp2) && tmp2 < commandString + COMMAND_SIZE; tmp2++);
    userDataPass += tmp2 - commandString;
#if NODE_DEBUG_SIMPLE
    DEBUG_PRINT ("line:%u", __LINE__);
    VAR_PRINT_STRING (tmp2);
#endif

    // 寻找命令与参数的间隔字符
    tmp = strstr (tmp2, spaceStr);
#if NODE_DEBUG_SIMPLE
    VAR_PRINT_STRING (tmp);
#endif
    if (tmp == NULL) {
        // 计算长度是否超出
        len = strlen (tmp2);
#if NODE_DEBUG_SIMPLE
        VAR_PRINT_LLU (len);
#endif
        if (len >= MAX_COMMAND) {
            DEBUG_PRINT ("Entered command is too long...");
            return lastError = NODE_CMD_TOO_LONG;
        }

        DEBUG_PRINT ("just had command...");
        return lastError = NODE_PARSE_ERR;
    }
#if NODE_DEBUG_SIMPLE
    DEBUG_PRINT ("line:%u", __LINE__);
#endif
    len = tmp - tmp2;  // 命令的长度
#if NODE_DEBUG_SIMPLE
    VAR_PRINT_LLU (len);
#endif
    memcpy (cmd, tmp2, len);  // 复制命令
#if NODE_DEBUG_SIMPLE
    DEBUG_PRINT ("line:%u", __LINE__);
#endif
    userDataPass += len;  // 将命令的长度也算进去, 等会在 ParseSpace 要用


    tmp2 = tmp;
#if NODE_DEBUG_SIMPLE
    VAR_PRINT_POS (tmp2);
#endif

    // 跳过所有不支持字符的地址
    for (; passableChParam (*tmp2) && tmp2 < commandString + COMMAND_SIZE; tmp2++);
    userDataPass += tmp2 - tmp;  // 累加跳过的空格
    if (tmp2 == commandString + COMMAND_SIZE) {
        DEBUG_PRINT ("<%s>There is no input parameter for this command...", cmd);
        showParam_simple (cmd);
        return lastError = NODE_ARG_ERR;
    }
    tmp = strstr (tmp2, spaceStr);
    // 已经没有' '的情况
    if (tmp == NULL) {

        current = Findcommand_simple (cmd, tmp2, 0);
#if NODE_DEBUG_SIMPLE
        VAR_PRINT_STRING (tmp2);
#endif
        if (current == NULL) {
            DEBUG_PRINT ("<%s><%s>Parameter not found in current command...",
                         cmd, tmp2);
            showCmdAndParam_simple();
            return lastError = NODE_NOT_FIND;
        }

        current->handlerArg = NULL;
        (current->handler) (current->handlerArg);
        return lastError = NODE_OK;
    }
#if NODE_DEBUG_SIMPLE
    DEBUG_PRINT ("line:%u", __LINE__);
#endif

    // 还有' '的情况
    len = tmp - tmp2;  // 参数的长度
    if (len >= MAX_PARAMETER) {
        DEBUG_PRINT ("Entered parameter is too long...");
        return lastError = NODE_PARAM_TOO_LONG;
    }
    memcpy (param, tmp2, len);  // 复制参数
    userDataPass += len;        // 将参数的长度也算进去, 等会在 ParseSpace 要用

    current = Findcommand_simple (cmd, param, 0);
    if (!current) {
        DEBUG_PRINT ("not find {%s}[%s]", cmd, param);
        showCmdAndParam_simple();
        return lastError = NODE_NOT_FIND;
    }

    current->handlerArg = (current->isRawStr)
                              ? (void *)tmp
                              : ParseSpace (tmp);
#if NODE_DEBUG_SIMPLE
    DEBUG_PRINT ("line:%u", __LINE__);
#endif
    if (current->handler == NULL) {
        DEBUG_PRINT ("<%s><%s>No handler function", cmd, param);
        return lastError = NODE_OK;
    }
    (current->handler) (current->handlerArg);  // 调用当前参数处理
    free (userData);                           // userData 传递给用户处理函数并处理结束后释放

    current->handlerArg = NULL;                // 清空指针

    userData = NULL;                           // 复位
    userDataCnt = 0;
    userDataPass = 0;
    return lastError = NODE_OK;
}
