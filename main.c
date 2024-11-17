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

        RegisterCommand(0, str);
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
    const char* cmd = NULL, * param = NULL, * tmp = NULL;
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
    funcAdd = strtoull(cmd, &tmp, 16);
    printf("parse funcAdd:<%llu><%llx>\n", funcAdd, funcAdd);
    func = (void*)funcAdd;
    RegisterParameter(cmdNode, func, 0, paramArr);
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
            ? wprintf(L"  %u command:<%ls>(isWch)\n",
                      (unsigned int)i, (wchar_t*)((map + i)->command))
            : printf("  %u command:<%s>\n",
                     (unsigned int)i, (char*)((map + i)->command));
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
    printf("Hello World!\n%s\ninput (1 test, 3 register):",
           TSET_STR);
RETRY:
    ret = scanf_s("%d", &type);
    CLEAN_STDIN();
    ERR_GOTO(!ret, errTip, RETRY);
    switch ( type )
    {
        case 1:
        {
            cmdRet(RegisterCommand(0, "command_TEST"));
            cmdRet(RegisterCommand(1, L"WIDTH_cmd"));

            cmdRet(updateCommand("command_TEST", NULL, "TEST_CMD", NULL));

            node = FindCommand("TEST_CMD", NULL);
            cmdRet(RegisterParameter(node, NULL, 0, "SHIT"));
            cmdRet(RegisterParameter(node, myFunc2, 1, "shit"));
            cmdRet(RegisterParameter(node, userShowList, 0, "list"));
#if 0
            cmdRet(RegisterCommand(0, "WIDTH"));
            node = FindCommand("WIDTH", NULL);
            // 注册char模式的命令下注册宽字符参数只截取了首个字符's',
            // 参数的字符解析模式跟随命令注册时的"是否使用宽字符"这个选项
            cmdRet(RegisterParameter(node, NULL, 0, L"shit"));
#endif
            cmdRet(updateParameter(node, NULL, 0, "SHIT", "MY_SHIT"));
            cmdRet(unRegisterParameter(node, "MY_SHIT"));

            node = FindCommand(NULL, L"WIDTH_cmd");
            cmdRet(RegisterParameter(node, NULL, 0, L"SHIT"));
            cmdRet(RegisterParameter(node, myFunc2, 1, L"shit"));
            cmdRet(RegisterParameter(node, userShowList, 0, L"list"));

            cmdRet(unRegisterAllParameters(node));

            CommandParse("test_cmd      shit    123");
            CommandParse("  test_cmd      shit    ");
            CommandParse("  test_cmd      shit");
            CommandParse("   test_cmd    SHIT  ");
            CommandParse("    test_cmd     list  ");

            cmdRet(unRegisterCommand(NULL, L"width_cmd"));

            cmdRet(unRegisterAllCommand());
        }break;
        case 3:
            defaultRegCmd_init();

            RegisterCommand(0, USER_CMD);
            NodeGetLastError();

            node = FindCommand(USER_CMD, NULL);
            RegisterParameter(node, showList, 1, "list");
            RegisterParameter(node, myFunc2, 1, "!?");
            RegisterParameter(node, myFunc, 1, "??!");
            RegisterParameter(node, exit, 1, "exit");
            RegisterParameter(node, regCmd, 1, "cmd");
            RegisterParameter(node, regParam, 1, "param");
            RegisterParameter(node, delAllCmd, 1, "delAllCmd");
            RegisterParameter(node, userShowList, 1, "ls");

            RegisterCommand(0, "USER_CMD");
            RegisterCommand(1, L"NULL");
            RegisterCommand(1, L"USER_CMD");

            while ( 1 )
            {
                memset(userScan, 0, sizeof(userScan));

                if ( fgets(userScan, COMMAND_SIZE, stdin) != NULL )
                {
                    // 去除换行符
                    userScan[strcspn(userScan, "\n")] = '\0';

                    CommandParse(userScan);
                    NodeGetLastError();

                }
                NodeGetLastError();
            }
            break;
        default:
            ERR_GOTO(type, errTip, RETRY);
            break;
    }

    system("pause");
    return 0;
}
