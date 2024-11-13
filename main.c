#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <Windows.h>
#include "CommandParse.h"
#include "main.h"

#define USER_CMD "Rgb"
#define USER_REG_CMD "cmd"
#define USER_DELALL_CMD "DelAllCmd"
#define USER_DELALL_PARAM "DelAllParam"

static wchar_t command_w[COMMAND_SIZE];
static char command[COMMAND_SIZE];

static int print_array(void* array, const size_t memberSize)
{
    void* pos = array;
    printf_s("[%llu]", memberSize);
    switch ( memberSize )
    {
        case sizeof(unsigned char) :
        {
            while ( *((unsigned char*)pos) != '\0' )
                printf_s("%u,", *((unsigned char*)pos)++);
        }
        break;
        case sizeof(unsigned short) :
        {
            while ( *((unsigned short*)pos) != '\0' )
                printf_s("%u,", *((unsigned short*)pos)++);
        }
        break;
        case sizeof(unsigned int) :
        {
            while ( *((unsigned int*)pos) != '\0' )
                printf_s("%lu,", *((unsigned int*)pos)++);
        }
        break;
        case sizeof(unsigned long long) :
        {
            while ( *((unsigned long long*)pos) != '\0' )
                printf_s("%llu,", *((unsigned long long*)pos)++);
        }
        break;

        default:
            break;
    }
    return 0;
}


static int cnt = 0;
static void cmdRet(const int ret)
{
    printf("--------(%d) %s\n", ++cnt, (ret) ? "fail" : "ok");
}

static void myFunc(void* arg)
{
    if ( arg != NULL )
    {
        printf("%s\n", (char*)arg);
    }
    printf("----------");
    printf(" %s %p----------\n", __func__, myFunc);
    printf("----------");
    printf("----------\n");
}

static void myFunc2(void* arg)
{
    if ( arg != NULL )
    {
        printf("%s\n", (char*)arg);
    }
    printf("----------");
    printf(" %s %p----------\n", __func__, myFunc2);
}

static void regCmd(void* arg)
{
    const char* str = (char*)arg;
    char* cmd = NULL, * param = NULL, * tmp = NULL;
    char cmdArr[MAX_COMMAND] = {0};
    command_node* cmdNode = NULL;

    printf("parse cmd:<%s>\n", str);
    cmdNode = FindCommand(str, NULL);
    if ( cmdNode == NULL )
    {
        RegisterCommand(0, str, NULL);
        NodeGetLastError();
        printf("----%s----\n", __func__);
    }
    else
    {
        printf("命令已经创建\n");
    }
    printf("----%s----\n", __func__);
}

static void regParam(void* arg)
{
    const char* str = (char*)arg;
    char* cmd = NULL, * param = NULL;
    void* func = NULL;
    unsigned long long funcAdd = 0;
    char cmdArr[MAX_COMMAND] = {0}, paramArr[MAX_PARAMETER] = {0};
    command_node* cmdNode = NULL;
    parameter_node* paramNode = NULL;

    param = strstr(str, " ");
    cmd = str;
    memcpy(cmdArr, cmd, (size_t)(param - cmd));
    printf("parse cmd:<%s>\n", cmdArr);
    cmdNode = FindCommand(cmdArr, NULL);
    if ( cmdNode == NULL )
    {
        NodeGetLastError();
        printf("----%s----\n", __func__);
        return;
    }

    param += 1;
    if ( param == NULL )
        return;
    cmd = strstr(param, " ");
    memcpy(paramArr, param, (size_t)(cmd - param));
    printf("parse param:<%s>\n", paramArr);

    cmd += 1;
    if ( cmd == NULL )
        return;
    funcAdd = (void*)strtoull(cmd, cmd + strlen(cmd), 16);
    printf("parse funcAdd:<%llu><%llx>\n", funcAdd, funcAdd);
    func = (void*)funcAdd;
    RegisterParameter(cmdNode, func, 0, paramArr, NULL);
    NodeGetLastError();
    printf("----%s----\n", __func__);
}

static void delAllCmd(void* arg)
{
    unRegisterAllCommand();
    printf("----%s----\n", __func__);
}

static void userShowList(void* arg)
{
    int len = 0;
    command_info* map = NULL;
    len = NodeGetCommandMap(&map);
    if ( len == 0 || map == NULL )
    {
        printf("get list map fail, len:%d, map:%p\n", len, map);
        free(map);
        return;
    }

    for ( size_t i = 0; i < len; i++ )
    {
        (((command_node*)((map + i)->node))->isWch)
            ? wprintf(L"command:<%ls>  %u\n",
                      (wchar_t*)((map + i)->command), (unsigned int)i)
            : printf("command:<%s>  %u\n",
                     (char*)((map + i)->command), (unsigned int)i);
    }
    printf("----%s----\n", __func__);
    free(map);
    return;
}


int main(int argc, char* argv[])
{
    int type = EOF, ret, once = 0;
    command_node* node;
    char userScan[COMMAND_SIZE] = {0};
    static const char* errTip = "输入错误...please retry(input 1 or 0):\n";
    printf("Hello World!\n%s\ninput (2 test, 3 register):",
           TSET_STR);
RETRY:
    ret = scanf_s("%d", &type);
    CLEAN_STDIN();
    ERR_GOTO(!ret, errTip, RETRY);
    switch ( type )
    {
        case 2:
        {
            cmdRet(unRegisterAllCommand());

            node = FindCommand("test1", NULL);
            cmdRet(RegisterParameter(node, NULL, 0, "TEST1", NULL));

            cmdRet(RegisterCommand(0, "TEST1", NULL));
            cmdRet(RegisterCommand(0, "TEST2", NULL));
            cmdRet(RegisterCommand(0, "你", NULL));
            cmdRet(RegisterCommand(1, NULL, L"shit"));

            node = FindCommand("test1", NULL);
            cmdRet(RegisterParameter(node, NULL, 0, "param3", NULL));
            cmdRet(RegisterParameter(node, NULL, 0, "param1", NULL));

            cmdRet(RegisterParameter(node, NULL, 0, NULL, L"TEST1"));
            cmdRet(RegisterParameter(node, NULL, 0, "param3", NULL));
            cmdRet(RegisterParameter(node, NULL, 0, "param4", NULL));
            cmdRet(RegisterParameter(node, NULL, 0, "Param4", NULL));
            cmdRet(RegisterParameter(node, myFunc, 0, "param5", NULL));

            cmdRet(unRegisterParameter(node, "param1", NULL));
            cmdRet(unRegisterParameter(node, NULL, L"param1"));
            cmdRet(unRegisterParameter(node, "param3", NULL));

            cmdRet(updateParameter(node, NULL, 0, "Param4", NULL, "4Param", NULL));
            cmdRet(updateParameter(node, NULL, 0, "4Param", NULL, "param5", NULL));
            cmdRet(updateParameter(node, NULL, 0, "Param4", NULL, NULL, NULL));
            cmdRet(updateParameter(node, NULL, 0, NULL, NULL, "Param4", NULL));
            cmdRet(updateParameter(node, NULL, 0, NULL, L"Param4", NULL, L"Param4"));

            node = FindCommand("test6", NULL);
            cmdRet(RegisterParameter(node, 0, NULL, "TEST1", NULL));

            node = FindCommand(NULL, L"shit");
            cmdRet(RegisterParameter(node, myFunc2, 1, NULL, L"shitParam"));

            node = FindCommand("你", NULL);
            cmdRet(RegisterParameter(node, myFunc2, 1, "MB参数", NULL));
            cmdRet(updateParameter(node, myFunc2, 1, "MB参数", NULL, "参数MB", NULL));

            cmdRet(updateCommand("test1", NULL, "test3", NULL));
            cmdRet(updateCommand("test2", NULL, "test3", NULL));
            cmdRet(updateCommand("test1", NULL, "test3", NULL));
            showList();

            CommandParse("test1");
            CommandParse("test2");
            CommandParse("test3 param5");
            CommandParse("test3 4Param");
            CommandParse("test3 param5 What's this?");
            CommandParse("你 MB参数 你他MBd再说一遍？?？");
            CommandParse("你 参数MB 你他MBd再说一遍！!！");


            cmdRet(unRegisterAllCommand());
        }
        break;
        case 3:
            defaultRegCmd_init();

            RegisterCommand(0, USER_CMD, NULL);
            NodeGetLastError();

            node = FindCommand(USER_CMD, NULL);
            RegisterParameter(node, showList, 1, "list", NULL);
            RegisterParameter(node, myFunc2, 1, "!?", NULL);
            RegisterParameter(node, myFunc, 1, "??!", NULL);
            RegisterParameter(node, exit, 1, "exit", NULL);
            RegisterParameter(node, regCmd, 1, "cmd", NULL);
            RegisterParameter(node, regParam, 1, "param", NULL);
            RegisterParameter(node, delAllCmd, 1, "delAllCmd", NULL);
            RegisterParameter(node, userShowList, 1, "ls", NULL);

            RegisterCommand(0, "USER_CMD", NULL);
            RegisterCommand(1, NULL, L"NULL");
            RegisterCommand(1, NULL, L"USER_CMD");

            while ( 1 )
            {
#if 0
                memset(userScan, 0, COMMAND_SIZE);
                scanf_s("%s", userScan, COMMAND_SIZE);
                printf_s("%s\n", userScan);
                CLEAN_STDIN();
                CommandParse(userScan);
                NodeGetLastError();
#endif
                memset(userScan, 0, sizeof(userScan));
                if ( fgets(userScan, COMMAND_SIZE, stdin) != NULL )
                {
                    // 去除换行符
                    userScan[strcspn(userScan, "\n")] = '\0';

                    CommandParse(userScan);
                    NodeGetLastError();

                }
            }
            break;
        default:
            ERR_GOTO(type, errTip, RETRY);
            break;
    }

    system("pause");
    return 0;
}
