#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CommandParseTree.h"
#include "cmdUserStringParse.h"

/* ====== 内存分配器（桌面用 malloc/free） ====== */
void* cmd_MemoryAlloc(size_t bytes) { return malloc(bytes); }
void cmd_MemoryFree(void* mem) { free(mem); }

/* ====== 辅助宏 ====== */
#define CLEAN_STDIN() while (getchar() != '\n')

/* ====== 测试 handler ====== */
static void h_myFunc(void* arg)
{
    userString* pdata = (userString*)arg;
    if (arg != NULL) {
        int n = userParse_GetUserParamCnt();
        printf("  params: %d, first: <%.*s>\n", n,
               (int)pdata->len, (const char*)pdata->strHead);
    }
    printf("  [handler: myFunc]\n");
}

static void h_device(void* arg)
{
    printf("  [handler: device] — show device info\n");
    if (arg) {
        userString* pdata = (userString*)arg;
        printf("  extra arg: <%.*s>\n", (int)pdata->len, (const char*)pdata->strHead);
    }
}

static void h_reset(void* arg)  { printf("  [handler: reset] — rebooting...\n"); }
static void h_ota(void* arg)    { printf("  [handler: ota] — entering OTA mode\n"); }
static void h_flash(void* arg)  { printf("  [handler: flash] — running flash test\n"); }

static void h_test(void* arg)
{
    printf("  [handler: test] — try 'test hardware' or 'test flash'\n");
}

static void h_hardware(void* arg)
{
    printf("  [handler: hardware] — running hardware self-test\n");
    if (arg) {
        userString* pdata = (userString*)arg;
        printf("  test item: <%.*s>\n", (int)pdata->len, (const char*)pdata->strHead);
    }
}

static void h_names(void* arg)
{
    printf("  [handler: names] — waiting for BLE names data...\n");
}

static void h_font(void* arg)
{
    printf("  [handler: font] — waiting for BLE font data...\n");
}

static void h_showTree(void* arg) { cmdTree_show(); }

static void h_exit(void* arg)
{
    printf("bye.\n");
    exit(0);
}

/* BLE data callbacks */
#if CMDTREE_ENABLE_DATA_HANDLER
static int d_names(const void* pBuff, const int len)
{
    printf("  [data: names] received %d bytes\n", len);
    return 0;
}

static int d_font(const void* pBuff, const int len)
{
    printf("  [data: font] received %d bytes\n", len);
    return 0;
}
#endif

/* ==================================================================
 * case 1: 基本两级路由
 * ================================================================== */
static void demo_basic_two_level(void)
{
    cmdTree_init();

    cmdTreeNodeRef dev = cmdTree_Register(CMDTREE_ROOT, "device", h_device, NULL);
    cmdTree_Register(dev, "reset", h_reset, NULL);
    cmdTree_Register(dev, "ota",   h_ota,   NULL);

    cmdTree_show();

    printf("\n--- 解析 'device' ---\n");       cmdTree_CommandParse("device");
    printf("\n--- 解析 'device reset' ---\n");  cmdTree_CommandParse("device reset");
    printf("\n--- 解析 'device ota' ---\n");    cmdTree_CommandParse("device ota");

    printf("\n--- 解析 'device bootloader' (不存在) ---\n");
    int ret = cmdTree_CommandParse("device bootloader");
    printf("  ret=%d (expected -1 NOT_FOUND)\n", ret);

    cmdTree_reset();
}

/* ==================================================================
 * case 2: 多级路由 + BLE dataHandler
 * ================================================================== */
static void demo_multi_level(void)
{
    cmdTree_init();

    cmdTreeNodeRef test = cmdTree_Register(CMDTREE_ROOT, "test", h_test, NULL);
    cmdTree_Register(test, "hardware", h_hardware, NULL);
    cmdTree_Register(test, "flash",    h_flash,    NULL);

    cmdTreeNodeRef wait = cmdTree_Register(CMDTREE_ROOT, "wait", NULL, NULL);
    cmdTreeNodeRef dat  = cmdTree_Register(wait, "dat", NULL, NULL);
    cmdTree_Register(dat, "names", h_names,
#if CMDTREE_ENABLE_DATA_HANDLER
                     d_names
#else
                     NULL
#endif
    );
    cmdTree_Register(dat, "font",  h_font,
#if CMDTREE_ENABLE_DATA_HANDLER
                     d_font
#else
                     NULL
#endif
    );

    cmdTree_show();

    printf("\n--- 'test' ---\n");               cmdTree_CommandParse("test");
    printf("\n--- 'test hardware' ---\n");       cmdTree_CommandParse("test hardware");
    printf("\n--- 'test hardware 0' (透传) ---\n"); cmdTree_CommandParse("test hardware 0");
    printf("\n--- 'wait dat names' ---\n");      cmdTree_CommandParse("wait dat names");
    printf("  active dataHandler: %p\n", (void*)cmdTree_getActiveDataHandler());
    printf("\n--- 'wait dat font' ---\n");       cmdTree_CommandParse("wait dat font");

    /* 模拟 BLE 数据到达 */
#if CMDTREE_ENABLE_DATA_HANDLER
    printf("\n--- 模拟 BLE 数据回调 ---\n");
    cmdTree_CommandParse("wait dat names");
    data_handler_fn_t dh = cmdTree_getActiveDataHandler();
    const char* data = "fake_ble_data";
    if (dh) dh(data, strlen(data));
#endif

    cmdTree_reset();
}

/* ==================================================================
 * case 3: 交互式 REPL（主演示）
 * ================================================================== */
static void demo_repl(void)
{
    char input[PARSE_SIZE];

    cmdTree_init();

    cmdTreeNodeRef dev  = cmdTree_Register(CMDTREE_ROOT, "device", h_device, NULL);
    cmdTree_Register(dev, "reset", h_reset, NULL);
    cmdTree_Register(dev, "ota",   h_ota,   NULL);

    cmdTreeNodeRef test = cmdTree_Register(CMDTREE_ROOT, "test", h_test, NULL);
    cmdTree_Register(test, "hardware", h_hardware, NULL);
    cmdTree_Register(test, "flash",    h_flash,    NULL);

    cmdTreeNodeRef wait = cmdTree_Register(CMDTREE_ROOT, "wait", NULL, NULL);
    cmdTreeNodeRef dat  = cmdTree_Register(wait, "dat", NULL, NULL);
    cmdTree_Register(dat, "names", h_names,
#if CMDTREE_ENABLE_DATA_HANDLER
                     d_names
#else
                     NULL
#endif
    );
    cmdTree_Register(dat, "font",  h_font,
#if CMDTREE_ENABLE_DATA_HANDLER
                     d_font
#else
                     NULL
#endif
    );

    cmdTree_Register(CMDTREE_ROOT, "list", h_showTree, NULL);
    cmdTree_Register(CMDTREE_ROOT, "exit", h_exit,    NULL);

    printf("=== cmdTree REPL ===\n");
    printf("Commands: device [reset|ota] | test [hardware|flash]\n");
    printf("          wait dat [names|font] | list | exit\n");
    printf("Quoted raw: device \"hello world\"\n");
    printf("====================\n\n");

    while (1) {
        printf("> ");
        if (fgets(input, PARSE_SIZE, stdin) == NULL) break;
        input[strcspn(input, "\n")] = '\0';
        if (input[0] == '\0') continue;

        int ret = cmdTree_CommandParse(input);
        if (ret == CMDTREE_ERR_NOT_FOUND)
            printf("  unknown command: '%s'\n", input);
    }

    cmdTree_reset();
}

/* ==================================================================
 * case 4: 引号 raw 数据
 * ================================================================== */
static void demo_quoted_params(void)
{
    cmdTree_init();
    cmdTree_Register(CMDTREE_ROOT, "device", h_device, NULL);

    printf("--- 'device \"hello world\"' ---\n");
    cmdTree_CommandParse("device \"hello world\"");

    printf("\n--- 'device normal_param' ---\n");
    cmdTree_CommandParse("device normal_param");

    cmdTree_reset();
}

/* ==================================================================
 * main
 * ================================================================== */
int main(void)
{
    int type;

    printf("cmdTree Demo\n");
    printf("  1 — basic two-level routing (device/reset, device/ota)\n");
    printf("  2 — multi-level routing + BLE dataHandler\n");
    printf("  3 — interactive REPL\n");
    printf("  4 — quoted params demo\n");
    printf("> ");

    if (scanf_s("%d", &type) != 1) return 1;
    CLEAN_STDIN();

    switch (type) {
    case 1: demo_basic_two_level();   break;
    case 2: demo_multi_level();       break;
    case 3: demo_repl();              break;
    case 4: demo_quoted_params();     break;
    default:
        printf("unknown option\n");
        break;
    }

    system("pause");
    return 0;
}
