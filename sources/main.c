#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <Windows.h>

#include "CommandParse.h"

#define CLEAN_STDIN() while ( getchar() != '\n' )
#define ERR_GOTO(err, errString, goto_tab) do{\
                                              if(err){\
                                                      printf_s("%s", errString);\
                                                      goto goto_tab;\
                                                      }\
                                              }while(0)

#define USER_CMD "Rgb"
#define USER_REG_CMD "cmd"
#define USER_DELALL_CMD "DelAllCmd"
#define USER_DELALL_PARAM "DelAllParam"

#define INFO_CHIP "chip_model"
#define INFO_FLASH "flash_size"
#define INFO_FREQ "frequency"
#define INFO_WIFI "wifi"
#define INFO_BT "bluetooth"

static wchar_t command_w[COMMAND_SIZE];
static char command[COMMAND_SIZE];

static int print_array(void* array, const size_t memberSize)
{
    void* pos = array;
    printf_s("[%llu]", memberSize);
    switch (memberSize)
    {
        case sizeof(unsigned char) :
        {
            while (*((unsigned char*)pos) != '\0')
                printf_s("%u,", *((unsigned char*)pos)++);
        }
        break;
        case sizeof(unsigned short) :
        {
            while (*((unsigned short*)pos) != '\0')
                printf_s("%u,", *((unsigned short*)pos)++);
        }
        break;
        case sizeof(unsigned int) :
        {
            while (*((unsigned int*)pos) != '\0')
                printf_s("%lu,", *((unsigned int*)pos)++);
        }
        break;
        case sizeof(unsigned long long) :
        {
            while (*((unsigned long long*)pos) != '\0')
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
    userString* pdata = arg;
    if (arg != NULL)
    {
        printf("argCnt:[%d], arg:{%s}\n", simpleCmd_GetUserParamsCnt(),
            (char*)(pdata->strHead));
    }
    printf("----------");
    printf(" %s %p----------\n", __func__, myFunc);
    printf("----------");
    printf("----------\n");
}

static void myFunc2(void* arg)
{
    char* pdata = arg;
    if (arg != NULL)
    {
        printf("arg:{%s}\n", pdata);
    }
    printf("----------");
    printf(" %s %p----------\n", __func__, myFunc2);
}

static void myFuncFreeList(void* arg)
{

}


static void regCmd(void* arg)
{
    const char* str = (char*)arg;
    char* cmd = NULL, * param = NULL, * tmp = NULL;
    char cmdArr[MAX_COMMAND] = { 0 };
    command_node* cmdNode = NULL;

    printf("parse cmd:<%s>\n", str);
    cmdNode = cmdNode_FindCommand(str, NULL);
    if (cmdNode == NULL)
    {

        cmdNode_RegisterCommand(0, str);
        cmdNode_GetLastError();
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
    char cmdArr[MAX_COMMAND] = { 0 }, paramArr[MAX_PARAMETER] = { 0 };
    command_node* cmdNode = NULL;
    parameter_node* paramNode = NULL;

    param = strstr(str, " ");
    cmd = str;
    memcpy(cmdArr, cmd, (size_t)(param - cmd));
    printf("parse cmd:<%s>\n", cmdArr);
    cmdNode = cmdNode_FindCommand(cmdArr, NULL);
    if (cmdNode == NULL)
    {
        cmdNode_GetLastError();
        printf("----%s----\n", __func__);
        return;
    }

    param += 1;
    if (param == NULL)
        return;
    cmd = strstr(param, " ");
    memcpy(paramArr, param, (size_t)(cmd - param));
    printf("parse param:<%s>\n", paramArr);

    cmd += 1;
    if (cmd == NULL)
        return;
    funcAdd = strtoull(cmd, &tmp, 16);
    printf("parse funcAdd:<%llu><%llx>\n", funcAdd, funcAdd);
    func = (void*)funcAdd;
    cmdNode_RegisterParameter(cmdNode, func, 0, paramArr);
    cmdNode_GetLastError();
    printf("----%s----\n", __func__);
}

static void delAllCmd(void* arg)
{
    cmdNode_unRegisterAllCommand();
    printf("----%s----\n", __func__);
}

static void userShowList(void* arg)
{
    int len = 0;
    command_info* map = NULL;
    len = NodeGetCommandMap(&map);
    if (len == 0 || map == NULL)
    {
        printf("get list map fail, len:%d, map:%p\n", len, map);
        free(map);
        return;
    }

    for (size_t i = 0; i < len; i++)
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




static unsigned char CharacterToNum(const char ch)
{
    switch (ch)
    {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        return ch - 48;
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
        return ch - 55;
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
        return ch - 87;
    default:
        break;
    }
    return (unsigned char)0xff;
}

static unsigned int StringToDevId(const char* DevStr)
{
    unsigned int DevId = 0x0000, cnt;
    unsigned char temp = 0x00;

    for (cnt = 0; cnt < 4; cnt++)
    {
        temp = CharacterToNum(DevStr[cnt]);
        if (temp == 0xff) // 解析错误
        {
            printf("Dev Id parse error |%c|", DevStr[cnt]);
            return DevId;
        }
        DevId = (DevId << 4) | temp;
    }
    return DevId;
}



int main(int argc, char* argv[])
{
    int type = EOF, ret, once = 0;
    command_node* node;
    char userScan[COMMAND_SIZE] = { 0 };
    static const char* errTip = "input error...     please retry(input 1 or 0):\n";
    printf("Hello World!\ninput ( 0 ~ 11 )\n(1&3 test cmdNode)\n(11 test cmdTable)\n:");
    printf("");
_RETRY:
    ret = scanf_s("%d", &type);
    CLEAN_STDIN();
    ERR_GOTO(!ret, errTip, _RETRY);
    switch (type)
    {
    case 0:
    {

    }break;
    case 1:
    {
        cmdRet(cmdNode_RegisterCommand(0, "command_TEST"));
        cmdRet(cmdNode_RegisterCommand(1, L"WIDTH_cmd"));

        cmdRet(cmdNode_updateCommand("command_TEST", NULL, "TEST_CMD", NULL));

        node = cmdNode_FindCommand("TEST_CMD", NULL);
        cmdRet(cmdNode_RegisterParameter(node, NULL, 0, "SHIT"));
        cmdRet(cmdNode_RegisterParameter(node, myFunc2, 1, "shit"));
        cmdRet(cmdNode_RegisterParameter(node, userShowList, 0, "list"));

        cmdRet(cmdNode_updateParameter(node, NULL, 0, "SHIT", "MY_SHIT"));
        cmdRet(cmdNode_unRegisterParameter(node, "MY_SHIT"));

        node = cmdNode_FindCommand(NULL, L"WIDTH_cmd");
        cmdRet(cmdNode_RegisterParameter(node, NULL, 0, L"SHIT"));
        cmdRet(cmdNode_RegisterParameter(node, myFunc2, 1, L"shit"));
        cmdRet(cmdNode_RegisterParameter(node, userShowList, 0, L"list"));

        cmdRet(cmdNode_unRegisterAllParameters(node));

        char teststring[32] = { 0 };
        strcpy(teststring, "  test_cmd      shit    ");
        cmdNode_CommandParse(teststring);

        memset(teststring, 0, sizeof(teststring));
        strcpy(teststring, "test_cmd      shit    123");
        cmdNode_CommandParse(teststring);

        memset(teststring, 0, sizeof(teststring));
        strcpy(teststring, "  test_cmd      shit");
        cmdNode_CommandParse(teststring);
        cmdNode_CommandParse("   test_cmd    SHIT  ");
        cmdNode_CommandParse("    test_cmd     list  ");

        cmdRet(cmdNode_unRegisterCommand(NULL, L"width_cmd"));

        cmdRet(cmdNode_unRegisterAllCommand());
    }break;
    case 2:
    {

    }break;
    case 3:
    {
        defaultRegCmd_init();

        cmdNode_RegisterCommand(0, USER_CMD);
        cmdNode_GetLastError();

        node = cmdNode_FindCommand(USER_CMD, NULL);
        cmdNode_RegisterParameter(node, cmdNode_showList, true, "list");
        cmdNode_RegisterParameter(node, myFunc2, true, "!?");
        cmdNode_RegisterParameter(node, myFunc, true, "??!");
        cmdNode_RegisterParameter(node, exit, true, "exit");
        cmdNode_RegisterParameter(node, regCmd, true, "cmd");
        cmdNode_RegisterParameter(node, regParam, true, "param");
        cmdNode_RegisterParameter(node, delAllCmd, true, "delAllCmd");
        cmdNode_RegisterParameter(node, userShowList, true, "ls");

        cmdNode_RegisterCommand(0, "USER_CMD");
        cmdNode_RegisterCommand(1, L"NULL");
        cmdNode_RegisterCommand(1, L"USER_CMD");

        while (1)
        {
            memset(userScan, 0, sizeof(userScan));

            if (fgets(userScan, COMMAND_SIZE, stdin) != NULL)
            {
                // 去除换行符
                userScan[strcspn(userScan, "\n")] = '\0';

                cmdNode_CommandParse(userScan);
                cmdNode_GetLastError();

            }
        }
    }break;
    case 4:
    {

    }break;
    case 5:
    {

    }break;
    case 7:
    {

    }break;
    case 8:
    {

    }break;
    case 9:
    {

    }break;
    case 10: {

        static const char* uCMD_1 = "check";
        static const char* uCMD_2 = "ch eck";
        static const char* uCMD_3 = "CHECK";
        static const char* uCMD_4 = "deinit";
        static const char* uPARAM_1 = "test";
        static const char* uPARAM_2 = "test 2";
        static const char* uPARAM_3 = "SimpleCMD";


        while (1)
        {
            printf("into commandSimple test~~\n");
            memset(userScan, 0, sizeof(userScan));

            if (fgets(userScan, COMMAND_SIZE, stdin) != NULL)
            {
                // 去除换行符
                userScan[strcspn(userScan, "\n")] = '\0';


                cmdNode_GetLastError();

            }
            cmdNode_GetLastError();
        }
    }break;
    case 11: {
#define STR_1 "device"
#define STR_2 "reset"
#define STR_3 "test"
#define STR_4 "device "

#define uCMD_1 "deinit"
#define uCMD_2 "test"
#define uCMD_3 "check"
        //#define uCMD_4 "device"
#define uPARAM_1 "SimpleCMD"
#define uPARAM_5 "changeADDR"
#define uPARAM_6 "server"
//#define uPARAM_7 "reset"
#define uPARAM_8 "alloc"
#define uPARAM_9 "ADstruct"
#define uPARAM_10 "bonding"
#define uPARAM_11 "RWdata"
#define uPARAM_13 "disconnect"

        static const char* func0 = "func";
        static const char* func1 = "func1";
        static const char* func2 = "func2";
        static const char* func3 = "func3";
        static const char* func4 = "func4";
        static const char* func5 = "func5";
        static const char* func6 = "func6";
        static const char* func7 = "func7";
        static const char* uCMD_4 = "device";
        static const char* uPARAM_7 = "reset";
#define uTEST_FUNC_0 "func"
#define uTEST_FUNC_1 "func1"
#define uTEST_FUNC_2 "func2"
#define uTEST_FUNC_3 "func3"
#define uTEST_FUNC_4 "func4"
#define uTEST_FUNC_5 "func5"
#define uTEST_FUNC_6 "func6"
#define uTEST_FUNC_7 "func7"

#define uFUNC(X) func##X
        printf("\"%s\" hash:[%x]\n", STR_1, cmdTable_CmdToHash(STR_1, strlen(STR_1)));
        printf("\"%s\" hash:[%x]\n", STR_4, cmdTable_CmdToHash(STR_4, strlen(STR_4)));
        printf("\"%s\" hash:[%x]\n", STR_4, cmdTable_CmdToHash(STR_4, strlen(STR_4) - 1));
        printf("\"%s\" hash:[%x]\n", STR_2, cmdTable_CmdToHash(STR_2, strlen(STR_2)));
        printf("\"%s\" hash:[%x]\n", STR_3, cmdTable_CmdToHash(STR_3, strlen(STR_3)));

        ret = cmdTable_RegisterCMD(uCMD_2, strlen(uCMD_2),
            uPARAM_1, strlen(uPARAM_1), myFunc);

        ret = cmdTable_RegisterCMD(uCMD_1, strlen(uCMD_1),
            uPARAM_1, strlen(uPARAM_1), myFunc);

        ret = cmdTable_RegisterCMD((void*)uCMD_4, strlen(uCMD_4),
            uPARAM_5, strlen(uPARAM_5), myFunc);

        ret = cmdTable_RegisterCMD(uCMD_3, strlen(uCMD_3),
            uPARAM_6, strlen(uPARAM_6), myFunc);

        ret = cmdTable_RegisterCMD((void*)uCMD_4, strlen(uCMD_4),
            (void*)uPARAM_7, strlen(uPARAM_7), myFunc);

        ret = cmdTable_RegisterCMD(uCMD_2, strlen(uCMD_2),
            uPARAM_8, strlen(uPARAM_8), myFunc);

        ret = cmdTable_RegisterCMD(uCMD_2, strlen(uCMD_2),
            uPARAM_9, strlen(uPARAM_9), myFunc);

        ret = cmdTable_RegisterCMD(uCMD_3, strlen(uCMD_3),
            uPARAM_10, strlen(uPARAM_10), myFunc);

        ret = cmdTable_RegisterCMD(uCMD_2, strlen(uCMD_2),
            uPARAM_11, strlen(uPARAM_11), myFunc);

        ret = cmdTable_RegisterCMD(uCMD_2, strlen(uCMD_2),
            (void*)uFUNC(0), strlen(uFUNC(0)), myFunc);

        ret = cmdTable_RegisterCMD((void*)uCMD_4, strlen(uCMD_4),
            uPARAM_13, strlen(uPARAM_13), myFunc);

        char* tempString[COMMAND_SIZE / 4] = { 0 };
        strcpy(tempString, "test SimpleCMD 123 456 789\0");
        cmdTable_CommandParse(tempString);

        cmdHash_node _old = {
            .command = cmdTable_CmdToHash(uCMD_2, strlen(uCMD_2)),
            .parameter = cmdTable_CmdToHash(uPARAM_1, strlen(uPARAM_1)),
            .handler = NULL
        };

        cmdHash_node _new = {
                .command = cmdTable_CmdToHash(uCMD_2, strlen(uCMD_2)),
                .parameter = cmdTable_CmdToHash("simplecmd", strlen("simplecmd")),
                .handler = NULL
        };

        cmdTable_updataCMDarg(&_old, &_new);
        cmdTable_CommandParse(tempString);

        strcpy(tempString, "test simplecmd 123 456 789\0");
        cmdTable_CommandParse(tempString);
        cmdTable_resetTable();


    }break;
    default:
        ERR_GOTO(type, errTip, _RETRY);
        break;
    }

    system("pause");
    return 0;
}
