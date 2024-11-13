#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <stdlib.h>
#include "CommandParse.h"

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

/**
 * @brief 用户的字符串
 */
static userString* userData = NULL;

/**
 * @brief 被跳过的空格数量
 */
static size_t userDataPassSpace = 0;

/**
 * @brief 用户参数的数量
 */
static size_t userDataCnt = 0;

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
    if ( CmdNode == NULL )
        return NODE_ARG_ERR;

    (CmdNode->isWch)
        ? wprintf(L" prev:<%ls>, this:<%ls>, next:<%ls>\n",
                  ((command_node*)(CmdNode->prev))->command_w,
                  CmdNode->command_w,
                  ((command_node*)(CmdNode->next))->command_w)
        : printf(" prev:<%s>, this:<%s>, next:<%s>\n",
                 ((command_node*)(CmdNode->prev))->command,
                 CmdNode->command,
                 ((command_node*)(CmdNode->next))->command);
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
    if ( 'A' <= *str && *str <= 'Z' )
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
    if ( str == NULL )
        return NODE_ARG_ERR;

    len = strlen(str);

    for ( ; i < len; i++ )
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
    if ( L'A' <= *str && *str <= L'Z' )
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
    if ( str == NULL )
        return NODE_ARG_ERR;

    len = wcslen(str);

    for ( ; i < len; i++ )
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
    while ( CmdNode->next != FristNode )// 遍历链表
    {
        if ( CmdNode != CmdNode->next )
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
    if ( node == NULL )
        return NULL;
    while ( paramNode->next != node->ParameterNode_head )
    {
        paramNode = paramNode->next;
    }
    return paramNode;
}

/**
 * @brief 分配命令节点的字符串
 * @param node 当前命令节点
 * @param isWch 是否使用宽字符
 * @param command 没有使用宽字符, 命令传入此参数, 另一个则填写 NULL
 * @param commandW
 * @return OK: NODE_OK
 * @return ERROR: NODE_ARG_ERR, NODE_CMD_TOO_LONG
 */
static int AssignCommandNodeStr(command_node* node,
                                const bool isWch,
                                const char* command,
                                const wchar_t* commandW)
{
    size_t len = 0;
    if ( node == NULL )
        return lastError = NODE_ARG_ERR;

    node->isWch = isWch;
    switch ( isWch )
    {
        case true:
        {
            if ( commandW == NULL )
                return lastError = NODE_ARG_ERR;

            len = wcslen(commandW);
            if ( len > (MAX_COMMAND - 1) )
                return lastError = NODE_CMD_TOO_LONG;

            // 非法字符检查
            for ( size_t i = 0; i < len; i++ )
            {
                if ( passableChCmdW(*(commandW + i)) )
                    return lastError = NODE_ARG_ERR;
            }
            wcscpy(node->command_w, commandW);
            UppercaseToLowercaseW(node->command_w);
        }
        break;
        default:
        {
            if ( command == NULL )
                return lastError = NODE_ARG_ERR;

            len = strlen(command);
            if ( len > (MAX_COMMAND - 1) )
                return lastError = NODE_CMD_TOO_LONG;

            // 非法字符检查
            for ( size_t i = 0; i < len; i++ )
            {
                if ( passableChCmd(*(command + i)) )
                    return lastError = NODE_ARG_ERR;
            }
            strcpy(node->command, command);
            UppercaseToLowercase(node->command);
        }
        break;
    }
    return lastError = NODE_OK;
}

/**
 * @brief 打印指定命令下所有参数
 * @param CmdNode 命令节点
 */
void showParam(command_node* CmdNode)
{
    parameter_node* node = CmdNode->ParameterNode_head;
    if ( CmdNode == NULL )
        return;

    if ( node == NULL )
    {
        (CmdNode->isWch)
            ? wprintf(L"<%ls>this command has no parameters...\n", CmdNode->command_w)
            : printf("<%s>this command has no parameters...\n", CmdNode->command);
        return;
    }

    putchar('\n');
    printf("  parameters:\n");
    do
    {
        (CmdNode->isWch)
            ? wprintf(L"        %ls\n", node->parameter_w)
            : printf("        %s\n", node->parameter);
        node = node->next;
    } while ( node != CmdNode->ParameterNode_head );
}

/**
 * @brief 打印命令链表
 */
void showList(void)
{
    command_node* node = FristNode;
    size_t cnt = 1;

    if ( node == NULL )
    {
        printf("list null...\n");
        return;
    }

    putchar('\n');
    do
    {
        (node->isWch)
            ? wprintf(L"%llu:command  <%ls>(isWch)     ", cnt++, node->command_w)
            : printf("%llu:command  <%s>     ", cnt++, node->command);
        showParam(node);
        node = node->next;
    } while ( node != FristNode ); // 为了保证遍历完整
    putchar('\n');
}

#if NODE_DEBUG
/**
 * @brief 检查当前命令节点是否有重复的命令
 * @param
 * @return OK: NODE_OK
 * @return ERROR: NODE_CMD_NODE_NULL, NODE_REPEATING, NODE_FAIL
 */
static int RepeatingCommandCheck(void)
{
    command_node* outer = FristNode, * inner = NULL;
    if ( outer == NULL )
    {
        printf("Command not yet created.\n");
        return NODE_CMD_NODE_NULL;
    }

    do
    {
        inner = outer->next;
        while ( inner != FristNode )
        {
            if ( outer->isWch && inner->isWch )
            {
                if ( wcscmp(outer->command_w, inner->command_w) == 0 )
                {
                    compare1 = outer;
                    compare2 = inner;
                    return NODE_REPEATING;
                }

            }
            else if ( !outer->isWch && !inner->isWch )
            {
                if ( strcmp(outer->command, inner->command) == 0 )
                {
                    compare1 = outer;
                    compare2 = inner;
                    return NODE_REPEATING;
                }

            }
            else
            {
                return NODE_FAIL;
            }
            inner = inner->next;
        }
        outer = outer->next;
    } while ( outer != FristNode );
    return NODE_OK;
}

/**
 * @brief 检查当前参数节点是否有重复的参数
 * @param CmdNode 要被检查的命令节点
 * @return OK: NODE_OK
 * @return ERROR: NODE_ARG_ERR, NODE_PARAM_NODE_NULL, NODE_REPEATING, NODE_FAIL
 */
static int RepeatingParamCheck(const command_node* CmdNode)
{
    parameter_node* outer = NULL, * inner = NULL;
    if ( CmdNode == NULL )
        return NODE_ARG_ERR;

    if ( CmdNode->ParameterNode_head == NULL )
        return NODE_PARAM_NODE_NULL;

    outer = CmdNode->ParameterNode_head;
    do
    {

        inner = outer->next;
        while ( inner != CmdNode->ParameterNode_head )
        {
            switch ( CmdNode->isWch )
            {
                case true:
                {
                    if ( wcscmp(outer->parameter_w, inner->parameter_w) == 0 )
                    {
                        compare3 = outer;
                        compare4 = inner;
                        return NODE_REPEATING;
                    }
                }
                break;
                case false:
                {
                    if ( strcmp(outer->parameter, inner->parameter) == 0 )
                    {
                        compare3 = outer;
                        compare4 = inner;
                        return NODE_REPEATING;
                    }
                }
                break;
            }
        }
        outer = outer->next;
    } while ( outer != CmdNode->ParameterNode_head );
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
command_node* FindCommand(const char* command,
                          const wchar_t* commandW)
{
    command_node* CmdNode = FristNode;
    char CmdTmp[MAX_COMMAND] = {0};
    wchar_t CmdTmpW[MAX_COMMAND] = {0};

    if ( CmdNode == NULL )
    {
        lastError = NODE_NOT_YET_INIT;
        return NULL;
    }


    if ( command == NULL && commandW == NULL )
        return NULL;

    if ( command != NULL )
    {
        if ( strlen(command) >= MAX_COMMAND )
        {
            lastError = NODE_CMD_TOO_LONG;
            return NULL;
        }

        strcpy(CmdTmp, command);
        if ( UppercaseToLowercase(CmdTmp) )// 确保字符统一小写
            return NULL;

        do
        {
            if ( strcmp(CmdNode->command, CmdTmp) == 0 )
                return CmdNode;

            CmdNode = CmdNode->next;
        } while ( CmdNode != FristNode );
    }
    else if ( commandW != NULL )
    {
        if ( wcslen(commandW) >= MAX_COMMAND )
        {
            lastError = NODE_CMD_TOO_LONG;
            return NULL;
        }

        wcscpy(CmdTmpW, commandW);
        if ( UppercaseToLowercaseW(CmdTmpW) )// 确保字符统一小写
            return NULL;

        do
        {
            if ( wcscmp(CmdNode->command_w, CmdTmpW) == 0 )
                return CmdNode;

            CmdNode = CmdNode->next;
        } while ( CmdNode != FristNode );
    }
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
 * @param command 命令, 若未使用宽字符, 另一个则填写 NULL
 * @param commandW 命令( 宽字符 )
 * @return OK: NODE_OK
 * @return ERROR: NODE_FAIL, NODE_ARG_ERR, NODE_CMD_TOO_LONG, NODE_REPEATING
 */
int RegisterCommand(const bool isWch,
                    const char* command,
                    const wchar_t* commandW)
{
#define PRINT_CMD(NODE) (isWch)\
    ? wprintf(L"<%ls> %ls\n", NODE->command_w, successStrW)\
       : printf("<%s> %s\n",  NODE->command, successStr)

    command_node* tmp = NULL, * repeating = NULL;
    command_node* CmdNode = NULL;// 要被初始化的节点
    if ( command == NULL && commandW == NULL )
    {
        printf("The command is null, exit func:<%s>\n", __func__);
        return lastError = NODE_ARG_ERR;
    }

    if ( FristNode == NULL )// 首次注册命令节点
    {
        FristNode = (command_node*)malloc(sizeof(command_node));
        CHECK_BUF(FristNode);
        FristNode->prev = FristNode;
        FristNode->next = FristNode;
        FristNode->ParameterNode_head = NULL;
        if ( AssignCommandNodeStr(FristNode, isWch, command, commandW) )
        {
            free(FristNode);
            return lastError;
        }

        PRINT_CMD(FristNode);
        return lastError = NODE_OK;
    }
    else
    {
        CmdNode = (command_node*)malloc(sizeof(command_node));
        CHECK_BUF(CmdNode);
        CmdNode->ParameterNode_head = NULL;
        if ( AssignCommandNodeStr(CmdNode, isWch, command, commandW) )
        {
            free(CmdNode);
            return lastError;
        }

        if ( command )
        {
            // 寻找是否有相同命令的节点, 当前节点尚在悬空, 故此不会影响寻找
            repeating = FindCommand(CmdNode->command, NULL);
        }
        else if ( commandW )
        {
            // 寻找是否有相同命令的节点, 当前节点尚在悬空, 故此不会影响寻找
            repeating = FindCommand(NULL, CmdNode->command_w);
        }
        else
        {
            printf("unknow error ! ! !\n");
            free(CmdNode);
            return lastError = NODE_FAIL;
        }

        if ( repeating != NULL )
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


        PRINT_CMD(CmdNode);
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
int unRegisterAllParameters(command_node* node)
{
    parameter_node* tmp = NULL, * nextNode = NULL;
    if ( node == NULL )
        return lastError = NODE_ARG_ERR;
    else
        tmp = node->ParameterNode_head;

    if ( tmp == NULL )
    {
        (node->isWch)
            ? wprintf(L"<%ls>(isWch) ParameterNode is NULL.\n", node->command_w)
            : printf("<%s> ParameterNode is NULL.\n", node->command);
        return lastError = NODE_OK;
    }

    do
    {
        nextNode = tmp->next;// 保存下一节点地址
        if ( nextNode == NULL )
            return lastError = NODE_PARAM_NODE_NULL;
        tmp->prev = NULL;
        tmp->next = NULL;
        free(tmp);// 释放当前节点
        tmp = nextNode;// 切换到下一节点
    } while ( tmp != node->ParameterNode_head );
    node->ParameterNode_head = NULL; // 此时已经释放完毕

    (node->isWch)
        ? wprintf(L"<%ls>(isWch) ParameterNodes unregister finish.\n", node->command_w)
        : printf("<%s> ParameterNodes unregister finish.\n", node->command);
    return lastError = NODE_OK;
}

/**
 * @brief 寻找参数节点
 * @param node 命令节点
 * @param param 参数
 * @param paramW 参数( 宽字符 )
 * @return
 */
static parameter_node* FindParameter(const command_node* node,
                                     const char* param,
                                     const wchar_t* paramW)
{
    parameter_node* paramNode = NULL;
    if ( node == NULL )
    {
        lastError = NODE_NOT_YET_INIT;
        return NULL;
    }


    if ( param == NULL && paramW == NULL )
        return NULL;

    paramNode = node->ParameterNode_head;
    switch ( node->isWch )
    {
        case false:
        {
            if ( param == NULL )
                return NULL;

            if ( strlen(param) >= MAX_PARAMETER )
            {
                lastError = NODE_PARAM_TOO_LONG;
                return NULL;
            }

            do// 即便只有一个 node 也要比较
            {
                if ( strcmp(paramNode->parameter, param) == 0 )
                    return paramNode;
                paramNode = paramNode->next;
            } while ( paramNode != node->ParameterNode_head );
        }
        break;
        case true:
        {
            if ( paramW == NULL )
                return NULL;

            if ( wcslen(paramW) >= MAX_PARAMETER )
            {
                lastError = NODE_PARAM_TOO_LONG;
                return NULL;
            }

            do// 即便只有一个 node 也要比较
            {
                if ( wcscmp(paramNode->parameter_w, paramW) == 0 )
                    return paramNode;
                paramNode = paramNode->next;
            } while ( paramNode != node->ParameterNode_head );
        }
        break;
    }

    lastError = NODE_NOT_FIND_PARAM;
    return NULL; // 遍历后均无所获
}

/**
 * @brief 取消注册命令
 * @param command 取决于注册节点时是否使用了宽字符, 若未使用宽字符, 另一个则填 NULL
 * @param commandW 目标命令( 宽字符 )
 * @return OK: NODE_OK
 * @return ERROR: NODE_FAIL, NODE_ARG_ERR, NODE_CMD_NODE_NULL, NODE_NOT_YET_INIT,
 * NODE_NOT_FIND_CMD
 */
int unRegisterCommand(char* command, wchar_t* commandW)
{
    command_node* CmdNode = NULL;// 要被删除的命令节点
    int ret = 0;
    if ( command == NULL && commandW == NULL )
    {
        printf("The command is null, exit func:<%s>\n", __func__);
        return lastError = NODE_ARG_ERR;
    }

    if ( FristNode == NULL )
    {
        if ( command )
            printf("<%s>Command node does not yet exist\n", command);
        if ( commandW )
            wprintf(L"<%ls>Command node does not yet exist\n", commandW);
        return lastError = NODE_CMD_NODE_NULL;
    }

    UppercaseToLowercaseW(commandW);
    UppercaseToLowercase(command);
    CmdNode = FindCommand(command, commandW);
    if ( CmdNode != NULL )// 通过 command 寻找 CommandNode
    {
        (CmdNode->isWch)
            ? wprintf(L"<%ls>find it\n", CmdNode->command_w)
            : printf("<%s>find it\n", CmdNode->command);


        // 更改当前节点的邻居节点对本节点的指针
        ((command_node*)(CmdNode->prev))->next = CmdNode->next;
#if NODE_DEBUG
        printCmdNode_command(CmdNode->prev);
#endif
        if ( CmdNode == FristNode )// 若头节点被删除, 更新头节点
        {
            FristNode = CmdNode->next;
            printf("update FristNode, %p\n", FristNode);
        }


        ((command_node*)(CmdNode->next))->prev = CmdNode->prev;
#if NODE_DEBUG
        printCmdNode_command(CmdNode->next);
#endif

        ret = unRegisterAllParameters(CmdNode);
        if ( ret == NODE_OK )
        {
            (CmdNode->isWch)
                ? wprintf(L"<%ls> CommandNodes unregister finish.\n",
                          CmdNode->command_w)
                : printf("<%s> CommandNodes unregister finish.\n",
                         CmdNode->command);
            free(CmdNode);
            return lastError = NODE_OK;
        }

    }
    else
    {
        if ( command )
            printf("<%s>not find \n", command);
        if ( commandW )
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
int updateCommand(char* oldCommand, wchar_t* oldCommandW,
                  char* newCommand, wchar_t* newCommandW)
{
    command_node* CmdNode = NULL;
    int ret = NODE_OK;
    if ( oldCommand )
    {
        if ( newCommand == NULL )
            return lastError = NODE_ARG_ERR;
    }
    else if ( oldCommandW )
    {
        if ( newCommandW == NULL )
            return lastError = NODE_ARG_ERR;
    }
    else
    {
        return lastError = NODE_FAIL;
    }

    if ( oldCommand )
    {
        ret = UppercaseToLowercase(oldCommand);
        ERR_CHECK(ret);

        ret = UppercaseToLowercase(newCommand);
        ERR_CHECK(ret);

        // 寻找目标命令节点
        CmdNode = FindCommand(oldCommand, NULL);
        if ( CmdNode == NULL )
        {
            printf("<%s>not find\n", oldCommand);
            return lastError = NODE_NOT_FIND_CMD;
        }

        // 判断是否已经存在新命令节点
        if ( FindCommand(newCommand, NULL) != NULL )
        {
            printf("The same command<%s> already exists, \
it has reverted to the old command<%s> name.\n", newCommand, oldCommand);
            return lastError = NODE_REPEATING;
        }
        strcpy(CmdNode->command, newCommand);
        return lastError = NODE_OK;
    }

    if ( oldCommandW )
    {
        ret = UppercaseToLowercaseW(oldCommandW);
        ERR_CHECK(ret);

        ret = UppercaseToLowercaseW(newCommandW);
        ERR_CHECK(ret);

        // 寻找目标命令节点
        CmdNode = FindCommand(NULL, oldCommandW);
        if ( CmdNode == NULL )
        {
            wprintf(L"<%ls>not find\n", oldCommandW);
            return lastError;
        }

        // 判断是否已经存在新命令节点
        if ( FindCommand(NULL, oldCommandW) )
        {
            wprintf(L"The same command<%ls> already exists, \
it has reverted to the old command<%s> name.\n", newCommandW, oldCommandW);
            return lastError = NODE_REPEATING;
        }
        wcscpy(CmdNode->command_w, newCommandW);
        return lastError = NODE_OK;
    }
    return lastError = NODE_FAIL;
}

/**
 * @brief 取消注册全部命令
 * @param
 * @return OK: NODE_OK
 * @return ERROR: NODE_FAIL, NODE_NOT_YET_INIT
 */
int unRegisterAllCommand(void)
{
    command_node* CmdNode = FristNode;
    command_node* nextNode = NULL;
    if ( CmdNode == NULL )
    {
        printf("Command not yet created\n");
        return lastError = NODE_NOT_YET_INIT;
    }

    do
    {
        lastError = unRegisterAllParameters(CmdNode);
        if ( lastError != NODE_OK )
        {
            FristNode = CmdNode;
            printf("The delete command encountered an unknown failure\n");
            return lastError;
        }
        (CmdNode->isWch)
            ? wprintf(L"<%ls>(isWch) deleted\n", CmdNode->command_w)
            : printf("<%s> deleted\n", CmdNode->command);

        nextNode = CmdNode->next;// 先保存 next 节点地址
        CmdNode->prev = NULL;// 断开 prev 节点
        CmdNode->next = NULL;// 断开 next 节点
        free(CmdNode);       // 释放当前节点
        CmdNode = nextNode;  // 节点移动
    } while ( CmdNode != FristNode );


    FristNode = NULL;
    printf("All commands deleted successfully\n");
    return lastError;
}

/**
 * @brief 分配参数节点的字符串
 * @param node 当前参数节点
 * @param isWch 命令节点的 isWch
 * @param param 参数, 取决于命令是否使用了宽字符, 若未使用宽字符, 另一个则填 NULL
 * @param paramW 参数( 宽字符 )
 * @return OK: NODE_OK
 * @return ERROR: NODE_ARG_ERR, NODE_PARAM_TOO_LONG
 */
static int AssignParameterNodeStr(parameter_node* node,
                                  const bool isWch,
                                  const char* param,
                                  const wchar_t* paramW)
{
    const char* widChStrWarning = "This parameter must use wide characters,\
 as it is commanded to use wide characters\n";
    const char* ChStrWarning = "This parameter must use characters\
 because it is commanded using the characters\n";

    if ( node == NULL )
        return lastError = NODE_ARG_ERR;

    if ( isWch )
    {
        if ( paramW == NULL )
        {
            printf("%s", widChStrWarning);
            return lastError = NODE_ARG_ERR;
        }

        if ( wcslen(paramW) > (MAX_PARAMETER - 1) )
            return lastError = NODE_PARAM_TOO_LONG;
        else
        {
            for ( size_t i = 0; i < wcslen(paramW); i++ )
            {
                // 检查参数是否有非法字符
                if ( passableChParamW(*(paramW + i)) )
                    return lastError = NODE_ARG_ERR;
            }
        }
        wcscpy(node->parameter_w, paramW);
    }
    else
    {
        if ( param == NULL )
        {
            printf("%s", ChStrWarning);
            return lastError = NODE_ARG_ERR;
        }

        if ( strlen(param) > (MAX_PARAMETER - 1) )
            return lastError = NODE_PARAM_TOO_LONG;
        else
        {
            for ( size_t i = 0; i < strlen(param); i++ )
            {
                if ( passableChParam(*(param + i)) )
                    return lastError = NODE_ARG_ERR;
            }
        }
        strcpy(node->parameter, param);
    }
    return lastError = NODE_OK;
}

/**
 * @brief 注册参数
 * @param node 注册参数的命令
 * @param hook 参数的处理函数
 * @param param 参数
 * @param paramW 参数( 宽字符 )
 * @return OK: NODE_OK
 * @return ERROR: NODE_CMD_NODE_NULL, NODE_ARG_ERR, NODE_PARAM_TOO_LONG,
 * NODE_FAIL, NODE_REPEATING
 *
 */
int RegisterParameter(command_node* node,
                      ParameterHandler hook,
                      const bool isRaw,
                      const char* param,
                      const wchar_t* paramW)
{
    parameter_node* paramNode = NULL, * tmp = NULL;
    const char* warningStr = "Not the same configuration(isWch)\
 as the command node...\n";
    if ( node == NULL )// 若传入的 node 为 NULL 则会访问错误的地址, 所以先判断
        return lastError = NODE_CMD_NODE_NULL;

    if ( param == NULL && paramW == NULL )
    {
        printf("The parameter is null, exit func:<%s>", __func__);
        return lastError = NODE_ARG_ERR;
    }


    paramNode = node->ParameterNode_head;
    if ( node->isWch )
    {
        if ( paramW == NULL )
        {
            printf("%s", warningStr);
            return lastError = NODE_ARG_ERR;
        }
    }
    else
    {
        if ( param == NULL )
        {
            printf("%s", warningStr);
            return lastError = NODE_ARG_ERR;
        }
    }

    if ( paramNode == NULL )
    {
        paramNode = (parameter_node*)malloc(sizeof(parameter_node));
        CHECK_BUF(paramNode);
        if ( AssignParameterNodeStr(paramNode, node->isWch, param, paramW) != NODE_OK )
        {
            free(paramNode);
            return lastError;
        }
        paramNode->handler = hook;
        paramNode->isRawStr = isRaw;
        paramNode->prev = node;
        paramNode->next = paramNode;
        node->ParameterNode_head = paramNode;
        (node->isWch)
            ? wprintf(L"<%ls><%ls> %ls\n", node->command_w, paramW, successStrW)
            : printf("<%s><%s> %s\n", node->command, param, successStr);
        return lastError = NODE_OK;
    }
    else
    {
        paramNode = FindParameter(node, param, paramW);
        if ( paramNode != NULL )// 参数名称重复检查
        {
            printf("The same parameter already exists...\n");
            return lastError = NODE_REPEATING;
        }
        paramNode = (parameter_node*)malloc(sizeof(parameter_node));
        CHECK_BUF(paramNode);
        if ( AssignParameterNodeStr(paramNode, node->isWch, param, paramW) != NODE_OK )
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
        (node->isWch)
            ? wprintf(L"<%ls><%ls> %ls\n", node->command_w, paramW, successStrW)
            : printf("<%s><%s> %s\n", node->command, param, successStr);
        return lastError = NODE_OK;
    }
    return lastError = NODE_FAIL;
}

/**
 * @brief 取消注册参数
 * @param node 要被删除参数的命令
 * @param param 参数
 * @param paramW 参数( 宽字符 )
 * @return OK: NODE_OK
 * @return ERROR: NODE_ARG_ERR, NODE_NOT_FIND_PARAM, NODE_FAIL, NODE_PARAM_TOO_LONG
 */
int unRegisterParameter(command_node* node,
                        const char* param,
                        const wchar_t* paramW)
{
    parameter_node* paramNode = NULL, * tmp = NULL;
    if ( node == NULL )
        return lastError = NODE_ARG_ERR;

    if ( param == NULL && paramW == NULL )
        return lastError = NODE_ARG_ERR;

    paramNode = FindParameter(node, param, paramW);
    if ( paramNode ) // 通过 parameter 寻找 ParameterNode
    {
        tmp = ParameterFinalNode(node);
        if ( paramNode == node->ParameterNode_head )
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

        (node->isWch)
            ? wprintf(L"<%ls> %ls, parameterNode unregister finish.\n",
                      node->command_w, paramNode->parameter_w)
            : printf("<%s> %s, parameterNode unregister finish.\n",
                     node->command, paramNode->parameter);

        free(paramNode);
        return lastError = NODE_OK;
    }
    else
    {
        (node->isWch)
            ? wprintf(L"<%s>, not find parameterNode\n", node->command_w)
            : printf("<%s>, not find parameterNode\n", node->command);
        return lastError;
    }
    return lastError = NODE_FAIL;
}

/**
 * @brief 更新参数
 * @param CmdNode 要被更改参数的命令
 * @param hook 参数处理函数
 * @param oldParam 旧参数
 * @param oldParamW 旧参数( 宽字符 )
 * @param newParam 新参数
 * @param newParamW 新参数( 宽字符 )
 * @return OK: NODE_OK
 * @return ERROR: NODE_ARG_ERR, NODE_REPEATING, NODE_NOT_FIND_PARAM
 */
int updateParameter(const command_node* CmdNode, ParameterHandler hook,
                    const bool isRaw,
                    const char* oldParam, const wchar_t* oldParamW,
                    const char* newParam, const wchar_t* newParamW)
{
    parameter_node* ParamNode = NULL;
    size_t len = 0;
    if ( CmdNode == NULL )
        return lastError = NODE_ARG_ERR;

    if ( oldParam )
    {
        if ( newParam == NULL )
            return lastError = NODE_ARG_ERR;
    }
    else if ( oldParamW )
    {
        if ( newParamW == NULL )
            return lastError = NODE_ARG_ERR;
    }
    else
    {
        return lastError = NODE_FAIL;
    }

    // 寻找旧参数
    ParamNode = FindParameter(CmdNode, oldParam, oldParamW);
    if ( ParamNode == NULL )
    {
        (CmdNode->isWch)
            ? wprintf(L"<%ls> <%ls>not find\n", CmdNode->command_w, oldParamW)
            : printf("<%s> <%s>not find\n", CmdNode->command, oldParam);
        return lastError = NODE_NOT_FIND_PARAM;
    }

    // 检查新参数是否已经存在
    if ( FindParameter(CmdNode, newParam, newParamW) != NULL )
    {
        (CmdNode->isWch)
            ? wprintf(L"The same param<%ls> already exists,\
 it has reverted to the old param<%ls> name.\n", newParamW, oldParamW)
            : printf("The same param<%s> already exists,\
 it has reverted to the old param<%s> name.\n", newParam, oldParam);
        return lastError = NODE_REPEATING;
    }

    if ( lastError == NODE_PARAM_TOO_LONG )
    {
        printf("func:<%s>, 'param' is too long\n", __func__);
        return lastError;
    }

    // 合法字符检查
    switch ( CmdNode->isWch )
    {
        case false:
        {
            if ( newParam == NULL )
                return lastError = NODE_ARG_ERR;

            len = strlen(newParam);
            for ( size_t i = 0; i < len; i++ )
            {
                if ( passableChParam(*(newParam + i)) )
                {
                    printf("Illegal characters in a string...\n");
                    return lastError = NODE_ARG_ERR;
                }
            }
            strcpy(ParamNode->parameter, newParam);
            ParamNode->isRawStr = isRaw;
        }
        break;
        default:
        {
            if ( newParamW == NULL )
                return lastError = NODE_ARG_ERR;

            len = wcslen(newParamW);
            for ( size_t i = 0; i < len; i++ )
            {
                if ( passableChParamW(*(newParamW + i)) )
                {
                    printf("Illegal characters in a string...\n");
                    return lastError = NODE_ARG_ERR;
                }
            }
            wcscpy(ParamNode->parameter_w, newParamW);
            ParamNode->isRawStr = isRaw;
        }
        break;
    }

    ParamNode->handler = hook;
    return lastError = NODE_OK;
}

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

    if ( current == NULL )
    {
        printf("<%s>list null...\n", __func__);
        return 0;
    }

    do
    {
        len++;
        tmp = *map;// 先保存 map , 预防 realloc 失败导致内存泄露
        *map = (command_info*)realloc(tmp, len * sizeof(command_info));
        if ( *map == NULL )
        {
            lastError = NODE_ALLOC_ERR;
            free(tmp);
            return 0;
        }

        (*map + len - 1)->command = (current->isWch)
            ? (void*)(current->command_w)
            : (void*)(current->command); // 根据节点是否使用宽字符指向不同的数组
        (*map + len - 1)->node = (void*)current;
        current = current->next;
    } while ( current != FristNode );
    return len;
}

/**
 * @brief 解析空格中混含的用户参数
 * @param userParam 用户参数
 * @return 保存用户参数的首地址
 */
static void* ParseSpace(const char* userParam)
{
    const char space = ' ';
    size_t passLen = 0; // 已经处理的长度
    const size_t passConstant =
        COMMAND_SIZE - MAX_COMMAND - MAX_PARAMETER - userDataPassSpace;
    userString* tmp = NULL;
    char* str = userParam, * str2 = NULL;
    if ( userParam == NULL )
        return NULL;

    passLen = passConstant;
    do
    {
        // 在指令支持的最大长度内跳过所空格, 先检查是否还有有效字符
        for ( ; passableChParam(*str) && str < userParam + passConstant; str++ );
        if ( str >= userParam + passConstant )
            return userData; // 超出可解析的长度


        userDataCnt++;
        passLen += str - userParam;
        userData = (userString*)realloc(tmp, userDataCnt * sizeof(userString));
#if 0
        if ( userData == tmp )
        {
            free(userData);
            return userData = NULL;
        }
#endif
        if ( userData == NULL )
        {
            printf("<%s>alloc memory fail\n", __func__);
            free(tmp);
            return (void*)userData;
        }

        (userData + userDataCnt - 1)->strHead = (void*)str;// 字符串头

        str2 = str;
        for ( ; *str2 != space &&
             str2 < userParam + passConstant &&
             *str2 != '\0'; str2++ ); // 略过非空格
        if ( str2 > (passConstant + userParam) )
        {
            // 超出了限定的长度
            printf("<%s>userParam is too long( %u, %p )...\
 the end string has no end\n",
                   __func__, COMMAND_SIZE,
                   (void*)(str2 - (passConstant + userParam)));
            (userData + userDataCnt - 1)->len =
                (size_t)((passConstant + userParam) - str);
            return (void*)userData;
        }

        (userData + userDataCnt - 1)->len = (size_t)(str2 - str);
        str += (userData + userDataCnt - 1)->len;

        tmp = userData;
#if NODE_DEBUG
        printf("<%s>len:%llu\n", __func__, (userData + userDataCnt - 1)->len);
#endif
    } while ( str <= userParam + passConstant || *str2 == '\0' );
    return (void*)userData;
}

/**
 * @brief 解析空格中混含的用户参数( 宽字符 )
 * @param userParam 用户参数
 * @return 保存用户参数的首地址
 */
static void* ParseSpaceW(const wchar_t* userParam)
{
    const wchar_t space = L' ';
    size_t passLen = 0; // 已经处理的长度
    const size_t passConstant = (COMMAND_SIZE * 2U) - (MAX_COMMAND * 2U) -
        (MAX_PARAMETER * 2U) - userDataPassSpace;
    userString* tmp = NULL;
    wchar_t* str = userParam, * str2 = NULL;
    if ( userParam == NULL )
        return NULL;

    passLen = passConstant;
    do
    {
        // 在指令支持的最大长度内跳过所空格, 先检查是否还有有效字符
        for ( ; passableChParamW(*str) && str < userParam + passConstant; str++ );
        if ( str >= userParam + passConstant )
            return userData; // 超出可解析的长度


        userDataCnt++;
        passLen += str - userParam;
        userData = (userString*)realloc(tmp, userDataCnt * sizeof(userString));
#if 0
        if ( userData == tmp )
        {
            free(userData);
            return userData = NULL;
        }
#endif
        if ( userData == NULL )
        {
            printf("<%s>alloc memory fail\n", __func__);
            free(tmp);
            return (void*)userData;
        }

        (userData + userDataCnt - 1)->strHead = (void*)str;// 字符串头

        str2 = str;
        for ( ; *str2 != space &&
             str2 < userParam + passConstant &&
             *str2 != L'\0'; str2++ ); // 略过非空格
        if ( str2 > (passConstant + userParam) )
        {
            // 超出了限定的长度
            printf("<%s>userParam is too long( %u, %p )...\
 the end string has no end\n",
                   __func__, COMMAND_SIZE,
                   (void*)(str2 - (passConstant + userParam)));
            (userData + userDataCnt - 1)->len =
                (size_t)((passConstant + userParam) - str);
            return (void*)userData;
        }

        (userData + userDataCnt - 1)->len = (size_t)(str2 - str);
        str += (userData + userDataCnt - 1)->len;

        tmp = userData;
#if NODE_DEBUG
        printf("<%s>len:%llu\n", __func__, (userData + userDataCnt - 1)->len);
#endif
    } while ( str <= userParam + passConstant || *str2 == L'\0' );
    return (void*)userData;
}

/**
 * @brief 获得解析到的用户参数个数
 * @return 
 */
size_t NodeGetUserParamsCnt()
{
    return userDataCnt;
}

/**
 * @brief 命令解析
 * @param commandString 用户输入的字符串
 * @return OK: NODE_OK
 * @return ERROR: NODE_ARG_ERR, NODE_CMD_TOO_LONG, NODE_PARAM_TOO_LONG,
 * NODE_NOT_FIND_CMD, NODE_NOT_FIND_PARAM, NODE_PARSE_ERR
 */
int CommandParse(const char* commandString)
{
    const char* spaceStr = " ";
    command_node* CmdNode = NULL;
    parameter_node* ParamNode = NULL;
    char cmd[MAX_COMMAND] = {0};
    char param[MAX_PARAMETER] = {0};
    char* tmp = NULL, * tmp2 = NULL;
    size_t len = 0;
    if ( commandString == NULL )
        return lastError = NODE_ARG_ERR;

    // 寻找命令与参数的间隔字符
    tmp = strstr(commandString, spaceStr);
    if ( tmp == NULL )
    {
        // 计算长度是否超出
        len = strlen(commandString);
        if ( len >= MAX_COMMAND )
        {
            printf("Entered command is too long...\n");
            return lastError = NODE_CMD_TOO_LONG;
        }

        // 直接把源字符串扔进去寻找目标命令
        CmdNode = FindCommand(commandString, NULL);
        if ( CmdNode == NULL )
        {
            printf("<%s>The command was not found.\n", commandString);
            return lastError;
        }

        printf("<%s>There is no input parameter for this command...\n", commandString);
        showParam(CmdNode);
        return lastError = NODE_PARSE_ERR;
    }

    len = tmp - commandString; // 命令的长度
    memcpy(cmd, commandString, len); // 复制命令
    CmdNode = FindCommand(cmd, NULL); // 寻找命令
    if ( CmdNode == NULL )
    {
        printf("<%s>The command was not found.\n", cmd);
        return lastError;
    }

    tmp2 = tmp;
    // 跳过所有不支持字符的地址
    for ( ; passableChParam(*tmp2) && tmp2 < commandString + COMMAND_SIZE; tmp2++ );
    userDataPassSpace = tmp2 - tmp;
    if ( tmp2 == commandString + COMMAND_SIZE )
    {
        printf("<%s>There is no input parameter for this command...\n", cmd);
        showParam(CmdNode);
        return lastError = NODE_ARG_ERR;
    }
    tmp = strstr(tmp2, spaceStr);
    // 已经没有' '的情况
    if ( tmp == NULL )
    {
        ParamNode = FindParameter(CmdNode, tmp2, NULL);
        if ( ParamNode == NULL )
        {
            printf("<%s><%s>Parameter not found in current command...\n",
                   cmd, tmp2);
            return lastError;
        }
        ParamNode->handlerArg = NULL;
        if ( ParamNode->handler == NULL )
        {
            printf("<%s><%s>No handler function\n", cmd, tmp2);
            return lastError = NODE_OK;
        }

        // printf("last error:%d\n", lastError);
        (ParamNode->handler)(ParamNode->handlerArg); // 调用当前参数处理
        return lastError = NODE_OK;
    }

    // 还有' '的情况
    len = tmp - tmp2; // 参数的长度
    if ( len >= MAX_PARAMETER )
    {
        printf("Entered parameter is too long...\n");
        return lastError = NODE_PARAM_TOO_LONG;
    }
    memcpy(param, tmp2, len); // 复制参数
    ParamNode = FindParameter(CmdNode, param, NULL); // 寻找参数
    if ( ParamNode == NULL )
    {
        printf("<%s><%s>Parameter not found in current command...\n", cmd, param);
        return lastError;
    }

    ParamNode->handlerArg = (ParamNode->isRawStr)
        ? tmp
        : ParseSpace(tmp);
    if ( ParamNode->handler == NULL )
    {
        printf("<%s><%s>No handler function\n", cmd, param);
        return lastError = NODE_OK;
    }
    (ParamNode->handler)(ParamNode->handlerArg); // 调用当前参数处理
    free(userData);// userData 传递给用户处理函数并处理结束后释放

    ParamNode->handlerArg = NULL; // 清空指针
    userData = NULL;// 复位
    userDataCnt = 0;
    return lastError = NODE_OK;
}

#ifdef WCHAR_MIN
#ifdef WCHAR_MAX
/**
 * @brief 命令解析( 宽字符 )
 * @param commandString 用户输入的字符串
 * @return OK: NODE_OK
 * @return ERROR: NODE_ARG_ERR, NODE_CMD_TOO_LONG, NODE_PARAM_TOO_LONG,
 * NODE_NOT_FIND_CMD, NODE_NOT_FIND_PARAM, NODE_PARSE_ERR
 */
int CommandParseW(const wchar_t* commandString)
{
    const wchar_t* spaceStr = L" ";
    command_node* CmdNode = NULL;
    parameter_node* ParamNode = NULL;
    wchar_t cmd[MAX_COMMAND] = {0};
    wchar_t param[MAX_PARAMETER] = {0};
    wchar_t* tmp = NULL, * tmp2 = NULL;
    size_t len = 0;
    if ( commandString == NULL )
        return lastError = NODE_ARG_ERR;

    // 寻找命令与参数的间隔字符
    tmp = wcsstr(commandString, spaceStr);
    if ( tmp == NULL )
    {
        // 计算长度是否超出
        len = wcslen(commandString);
        if ( len >= MAX_COMMAND )
        {
            printf("Entered command is too long...\n");
            return lastError = NODE_CMD_TOO_LONG;
        }

        // 直接把源字符串扔进去寻找目标命令
        CmdNode = FindCommand(NULL, commandString);
        if ( CmdNode == NULL )
        {
            wprintf(L"<%ls>The command was not found.\n", commandString);
            return lastError;
        }
        wprintf(L"<%ls>There is no input parameter for this command...\n", commandString);
        showParam(CmdNode);
        return lastError = NODE_PARSE_ERR;
    }

    len = tmp - commandString; // 命令的长度
    memcpy(cmd, commandString, len); // 复制命令
    CmdNode = FindCommand(NULL, cmd); // 寻找命令
    if ( CmdNode == NULL )
    {
        wprintf(L"<%s>The command was not found.\n", cmd);
        showParam(CmdNode);
        return lastError;
    }

    tmp2 = tmp;
    // 跳过所有不支持字符的地址
    for ( ; passableChParamW(*tmp2) && tmp2 < commandString + COMMAND_SIZE; tmp2++ );
    userDataPassSpace = (tmp2 - tmp);// wchar_t  = 2 * char
    if ( tmp2 == commandString + COMMAND_SIZE )
    {
        wprintf(L"<%ls>There is no input parameter for this command...\n", cmd);
        return lastError = NODE_ARG_ERR;
    }
    tmp = wcsstr(tmp2, spaceStr);
    // 已经没有' '的情况
    if ( tmp == NULL )
    {
        ParamNode = FindParameter(CmdNode, NULL, tmp2);
        if ( ParamNode == NULL )
        {
            wprintf(L"<%ls><%ls>Parameter not found in current command...\n", cmd, tmp2);
            return lastError;
        }
        ParamNode->handlerArg = NULL;
        if ( ParamNode->handler == NULL )
        {
            wprintf(L"<%ls><%ls>No handler function\n", cmd, tmp2);
            return lastError = NODE_OK;
        }
        (ParamNode->handler)(ParamNode->handlerArg); // 调用当前参数处理
        return lastError = NODE_OK;
    }

    // 还有' '的情况
    len = tmp - tmp2; // 参数的长度
    if ( len >= MAX_PARAMETER )
    {
        printf("Entered parameter is too long...\n");
        return lastError = NODE_PARAM_TOO_LONG;
    }
    memcpy(param, tmp2, len); // 复制参数
    ParamNode = FindParameter(CmdNode, NULL, param); // 寻找参数
    if ( ParamNode == NULL )
    {
        wprintf(L"<%ls><%ls>Parameter not found in current command...\n", cmd, param);
        return lastError;
    }

    ParamNode->handlerArg = (ParamNode->isRawStr)
        ? tmp
        : ParseSpaceW(tmp);
    if ( ParamNode->handler == NULL )
    {
        wprintf(L"<%ls><%ls>No handler function\n", cmd, param);
        return lastError = NODE_OK;
}
    (ParamNode->handler)(ParamNode->handlerArg); // 调用当前参数处理
    ParamNode->handlerArg = NULL; // 清空指针

    return lastError = NODE_OK;
}
#endif // WCHAR_MAX


#endif // WCHAR_MIN

/**
 * @brief 获得上次运行链表管理函数的错误
 * @param
 * @return lastError
 */
int NodeGetLastError(void)
{
    switch ( lastError )
    {
        case NODE_OK:
            printf("NODE OK\n");
            break;
        case NODE_FAIL:
            printf("NODE unknow error\n");
            break;
        case NODE_ARG_ERR:
            printf("NODE parameter passing error\n");
            break;
        case NODE_NOT_FIND:
            printf("NODE not find\n");
            break;
        case NODE_NOT_FIND_CMD:
            printf("NODE not found command\n");
            break;
        case NODE_NOT_FIND_PARAM:
            printf("NODE not found parameter\n");
            break;
        case NODE_ALLOC_ERR:
            printf("NODE Alloc error, maybe RAM is not enough...\n");
            break;
        case NODE_CMD_NODE_NULL:
            printf("NODE this command is null\n");
            break;
        case NODE_PARAM_NODE_NULL:
            printf("NODE command has no paramNode\n");
            break;
        case NODE_REPEATING:
            printf("NODE is repeating create\n");
            break;
        case NODE_CMD_TOO_LONG:
            printf("NODE 'command' is too long\n");
            break;
        case NODE_PARAM_TOO_LONG:
            printf("NODE 'parameter' is too long\n");
            break;
        case NODE_PARSE_ERR:
            printf("NODE parsing string error\n");
            break;
        case NODE_NOT_YET_INIT:
            printf("NODEs have not been initialized...\n");
            break;
        default:
            break;
    }
    return lastError;
}

/**
 * @brief 输入指令 注册命令
 * @param arg
 */
static void regCmd(void* arg)
{
    userString* data = (userString*)arg;
    char cmd[MAX_COMMAND] = {0};
    int err = NODE_OK;
    command_node* cmdNode = NULL;
    if ( data == NULL )
    {
        printf("<%s>userParam error\n", __func__);
        return;
    }

    memcpy(cmd, data->strHead, data->len);
    printf("<%s>parse cmd:<%s>\n", __func__, cmd);
    cmdNode = FindCommand(cmd, NULL);
    err = NodeGetLastError();
    if ( cmdNode == NULL && (err == NODE_NOT_YET_INIT || err == NODE_NOT_FIND_CMD) )
    {
        RegisterCommand(0, cmd, NULL);
        NodeGetLastError();
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
    char cmdArr[MAX_COMMAND] = {0},
        paramArr[MAX_PARAMETER] = {0},
        funcArr[32] = {0};
    command_node* cmdNode = NULL;
    size_t paramPos = 0, paramCnt = NodeGetUserParamsCnt();
    if ( data == NULL )
    {
        printf("<%s>userParam error\n", __func__);
        return;
    }

    if ( paramCnt < 2 )
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
    if ( paramCnt > 2 )
    {
        paramPos++;
        memcpy(funcArr, (data + paramPos)->strHead, (data + paramPos)->len);
        funcAdd = strtoull(funcArr, &endptr, 16);
        func = (void*)funcAdd;
        printf("<%s>parse funcAdd:<%llx><%s>\n", __func__, funcAdd, endptr);
    }

    // 寻找目标命令
    cmdNode = FindCommand(cmdArr, NULL);
    if ( cmdNode == NULL )
    {
        NodeGetLastError();
        return;
    }

    if ( strcmp(paramArr, "exit") == 0 )
    {
        RegisterParameter(cmdNode, exit, 1, paramArr, NULL);
        return;
    }

    (paramCnt > 2)
        ? RegisterParameter(cmdNode, func, 0, paramArr, NULL)
        : RegisterParameter(cmdNode, NULL, 0, paramArr, NULL);
    return;
}

/**
 * @brief 输入指令 删除指定的命令
 * @param arg
 */
static void regDelCmd(void* arg)
{
    char cmd[MAX_COMMAND] = {0};
    userString* data = (userString*)arg;
    if ( data == NULL )
    {
        printf("<%s>userParam error\n", __func__);
        return;
    }

    memcpy(cmd, data->strHead, data->len);
    if ( strcmp(cmd, DEFAULT_CMD) == 0 )
    {
        printf("<%s>cannot be deleted command\n", __func__);
        return;
    }

    if ( unRegisterCommand(cmd, NULL) )
    {
        printf("<%s>", __func__);
        NodeGetLastError();
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
    char cmd[MAX_COMMAND] = {0};
    command_node* cmdNode = NULL;
    if ( data == NULL )
    {
        printf("<%s>userParam error\n", __func__);
        return;
    }

    memcpy(cmd, data->strHead, data->len);
    if ( strcmp(cmd, DEFAULT_CMD) == 0 )
    {
        printf("<%s>cannot be deleted command\n", __func__);
        return;
    }

    cmdNode = FindCommand(cmd, NULL);
    if ( cmdNode == NULL )
    {
        printf("<%s>", __func__);
        NodeGetLastError();
        return;
    }

    if ( unRegisterAllParameters(cmdNode) )
    {
        printf("<%s>", __func__);
        NodeGetLastError();
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
    char cmdArr[MAX_COMMAND] = {0},
        paramArr[MAX_PARAMETER] = {0};
    size_t dataCnt = NodeGetUserParamsCnt();
    command_node* cmdNode = NULL;
    if ( data == NULL )
    {
        printf("<%s>userParam error\n", __func__);
        return;
    }

    if ( dataCnt < 2 )
    {
        printf("<%s>Insufficient input parameters\n", __func__);
        return;
    }

    memcpy(cmdArr, data->strHead, data->len);
    printf("<%s>parse cmd:%s\n", __func__, cmdArr);

    memcpy(paramArr, (data + 1)->strHead, (data + 1)->len);
    printf("<%s>parse param:%s\n", __func__, paramArr);

    if ( strcmp(cmdArr, DEFAULT_CMD) == 0 )
    {
        printf("<%s>cannot be deleted command\n", __func__);
        return;
    }

    cmdNode = FindCommand(cmdArr, NULL);
    if ( cmdNode == NULL )
    {
        printf("<%s>", __func__);
        NodeGetLastError();
        return;
    }

    if ( unRegisterParameter(cmdNode, paramArr, NULL) )
    {
        printf("<%s>", __func__);
        NodeGetLastError();
        return;
    }
}

static void regDelAllCmd(void* arg)
{
    int len = 0;
    command_info* Map = NULL;
    len = NodeGetCommandMap(&Map);
    if ( len == 0 || Map == NULL )
    {
        printf("get list map fail, len:%d, map:%p\n", len, Map);
        free(Map);
        return;
    }

    for ( size_t i = 0; i < len; i++ )
    {
        switch ( ((command_node*)((Map + i)->node))->isWch )
        {
            case false:
            {
                if ( strcmp((char*)((Map + i)->command), DEFAULT_CMD) == 0 )
                    continue;
                else
                    unRegisterCommand((char*)(Map + i)->command, NULL);
            }
            break;
            default:
            {
                if ( wcscmp((wchar_t*)((Map + i)->command), DEFAULT_CMD_W) == 0 )
                    continue;
                else
                    unRegisterCommand(NULL, (wchar_t*)(Map + i)->command);
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
    RegisterCommand(0, DEFAULT_CMD, NULL);
    CmdNode = FindCommand(DEFAULT_CMD, NULL);
    if ( CmdNode == NULL )
    {
        printf("<%s>", __func__);
        NodeGetLastError();
        return -1;
    }

    RegisterParameter(CmdNode, regCmd, 0, DEFAULT_PARAM_cmd, NULL);
    RegisterParameter(CmdNode, regParam, 0, DEFAULT_PARAM_param, NULL);
    RegisterParameter(CmdNode, regDelAllParam, 0, DEFUALT_PARAM_DelAllParam, NULL);
    RegisterParameter(CmdNode, regDelCmd, 0, DEFAULT_PARAM_DelCmd, NULL);
    RegisterParameter(CmdNode, regDelParam, 0, DEFUALT_PARAM_DelParam, NULL);
    RegisterParameter(CmdNode, regDelAllCmd, 0, DEFUALT_PARAM_DelAllCmd, NULL);
    RegisterParameter(CmdNode, showList, 0, DEFUALT_PARAM_ls, NULL);

    return 0;
}
