#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "CommandParseTree.h"
#include "cmdUserStringParse.h"

/* ====== 内存分配实现（桌面用 malloc/free） ====== */
void* cmd_MemoryAlloc(size_t bytes) { return malloc(bytes); }
void cmd_MemoryFree(void* mem) { free(mem); }

/* ====== 测试框架 ====== */
static int testCnt  = 0;
static int failCnt  = 0;

#define TEST_ASSERT(cond, fmt, ...) do {                                      \
    if (!(cond)) {                                                             \
        printf("  FAIL: " fmt "\n", ##__VA_ARGS__);                           \
        failCnt++;                                                             \
    }                                                                          \
} while(0)

/* ====== handler 回调的捕获变量 ====== */
static const char *g_capturedHandler = NULL;       /* 哪个 handler 被调用了 */
static int         g_capturedParamCnt = 0;         /* handler 收到的参数个数 */
static const char *g_capturedParam0  = NULL;       /* 第一个参数内容 */
static const char *g_capturedParam1  = NULL;
static int         g_dataHandlerCalled = 0;        /* dataHandler 是否被调 */
static int         g_dataLen = 0;

static void resetCapture(void)
{
    g_capturedHandler   = NULL;
    g_capturedParamCnt  = 0;
    g_capturedParam0    = NULL;
    g_capturedParam1    = NULL;
    g_dataHandlerCalled = 0;
    g_dataLen           = 0;
}

/* ====== 测试用 handler ====== */
static void h_device(void *arg)
{
    g_capturedHandler = "device";
    if (arg) {
        userString *ud = (userString *)arg;
        g_capturedParamCnt = userParse_GetUserParamCnt();
        if (g_capturedParamCnt > 0) g_capturedParam0 = (const char *)ud[0].strHead;
        if (g_capturedParamCnt > 1) g_capturedParam1 = (const char *)ud[1].strHead;
    }
}

static void h_reset(void *arg)
{
    g_capturedHandler = "reset";
    if (arg) {
        userString *ud = (userString *)arg;
        g_capturedParamCnt = userParse_GetUserParamCnt();
        if (g_capturedParamCnt > 0) g_capturedParam0 = (const char *)ud[0].strHead;
    }
}

static void h_ota(void *arg)
{
    g_capturedHandler = "ota";
}

static void h_test(void *arg)
{
    g_capturedHandler = "test";
    if (arg) {
        userString *ud = (userString *)arg;
        g_capturedParamCnt = userParse_GetUserParamCnt();
        if (g_capturedParamCnt > 0) g_capturedParam0 = (const char *)ud[0].strHead;
    }
}

static void h_hardware(void *arg)
{
    g_capturedHandler = "hardware";
    if (arg) {
        userString *ud = (userString *)arg;
        g_capturedParamCnt = userParse_GetUserParamCnt();
        if (g_capturedParamCnt > 0) g_capturedParam0 = (const char *)ud[0].strHead;
    }
}

static void h_waitDat(void *arg)
{
    g_capturedHandler = "waitDat";
}

static void h_names(void *arg)
{
    g_capturedHandler = "names";
    if (arg) {
        userString *ud = (userString *)arg;
        g_capturedParamCnt = userParse_GetUserParamCnt();
        if (g_capturedParamCnt > 0) g_capturedParam0 = (const char *)ud[0].strHead;
    }
}

static void h_font(void *arg)
{
    g_capturedHandler = "font";
}

static void h_level3(void *arg)
{
    g_capturedHandler = "level3";
}

static void h_level5(void *arg)
{
    g_capturedHandler = "level5";
}

static void h_level10(void *arg)
{
    g_capturedHandler = "level10";
    if (arg) {
        userString *ud = (userString *)arg;
        g_capturedParamCnt = userParse_GetUserParamCnt();
        if (g_capturedParamCnt > 0) g_capturedParam0 = (const char *)ud[0].strHead;
    }
}

/* ====== 测试用 dataHandler ====== */
#if CMDTREE_ENABLE_DATA_HANDLER
static int d_names(const void *pBuff, const int len)
{
    g_dataHandlerCalled = 1;
    g_dataLen = len;
    return 0;
}

static int d_font(const void *pBuff, const int len)
{
    g_dataHandlerCalled = 2;
    g_dataLen = len;
    return 0;
}
#endif

/* ==================================================================
 * 测试用例
 * ================================================================== */

static void test_basic_init_reset(void)
{
    testCnt++;
    printf("\n[TEST %d] init / reset lifecycle\n", testCnt);

    cmdTree_init();
#if CMDTREE_MODE_DYNAMIC
    TEST_ASSERT(cmdTree_getRoot() != NULL, "root should not be CMDTREE_NULL");
#elif CMDTREE_MODE_STATIC
    TEST_ASSERT(cmdTree_getRoot() != CMDTREE_NULL, "root should not be CMDTREE_NULL");
#endif

    cmdTree_reset();
#if CMDTREE_MODE_DYNAMIC
    TEST_ASSERT(cmdTree_getRoot() == NULL, "root should be NULL after reset");
#elif CMDTREE_MODE_STATIC
    TEST_ASSERT(cmdTree_getRoot() == CMDTREE_NULL, "root should be CMDTREE_NULL after reset");
#endif
    
}

static void test_simple_two_level(void)
{
    testCnt++;
    printf("\n[TEST %d] simple two-level: device/reset\n", testCnt);

    cmdTree_init();

    cmdTreeNodeRef dev = cmdTree_Register(CMDTREE_ROOT, "device", h_device, NULL);
    TEST_ASSERT(dev != CMDTREE_NULL, "register device failed");

    cmdTreeNodeRef rst = cmdTree_Register(dev, "reset", h_reset, NULL);
    TEST_ASSERT(rst != CMDTREE_NULL, "register reset failed");

    /* "device reset" → 命中 reset handler */
    resetCapture();
    int ret = cmdTree_CommandParse("device reset");
    TEST_ASSERT(ret == CMDTREE_OK, "parse should succeed");
    TEST_ASSERT(g_capturedHandler && !strcmp(g_capturedHandler, "reset"),
                "should call reset handler, got %s", g_capturedHandler);

    /* "device" → 命中 device handler */
    resetCapture();
    ret = cmdTree_CommandParse("device");
    TEST_ASSERT(ret == CMDTREE_OK, "parse device should succeed");
    TEST_ASSERT(g_capturedHandler && !strcmp(g_capturedHandler, "device"),
                "should call device handler");

    /* "device reset 42" → handler 拿到剩余参数 */
    resetCapture();
    ret = cmdTree_CommandParse("device reset 42");
    TEST_ASSERT(ret == CMDTREE_OK, "parse with params should succeed");
    TEST_ASSERT(g_capturedHandler && !strcmp(g_capturedHandler, "reset"),
                "should call reset handler");
    TEST_ASSERT(g_capturedParam0 && !strncmp(g_capturedParam0, "42", 2),
                "should receive param '42'");

    /* "device unknown" → 命中 device handler（下行到 device 后 unknown 不匹配，回退到 device） */
    resetCapture();
    ret = cmdTree_CommandParse("device unknown");
    TEST_ASSERT(ret == CMDTREE_OK, "fallback to device should succeed");
    TEST_ASSERT(g_capturedHandler && !strcmp(g_capturedHandler, "device"),
                "should fallback to device handler");

    cmdTree_reset();
}

static void test_not_found(void)
{
    testCnt++;
    printf("\n[TEST %d] not found\n", testCnt);

    cmdTree_init();
    cmdTree_Register(CMDTREE_ROOT, "device", h_device, NULL);

    /* 完全不存在的命令 */
    int ret = cmdTree_CommandParse("nonexistent command");
    TEST_ASSERT(ret == CMDTREE_ERR_NOT_FOUND, "should be not found");

    /* 空字符串 */
    ret = cmdTree_CommandParse("");
    TEST_ASSERT(ret == CMDTREE_ERR_NOT_FOUND, "empty string should be not found");

    cmdTree_reset();
}

static void test_multi_level_routing(void)
{
    testCnt++;
    printf("\n[TEST %d] multi-level: wait/dat/names\n", testCnt);

    cmdTree_init();

    cmdTreeNodeRef wait = cmdTree_Register(CMDTREE_ROOT, "wait", NULL, NULL);
    TEST_ASSERT(wait != CMDTREE_NULL, "register wait failed");

    cmdTreeNodeRef dat = cmdTree_Register(wait, "dat", NULL, NULL);
    TEST_ASSERT(dat != CMDTREE_NULL, "register dat failed");

    cmdTreeNodeRef names = cmdTree_Register(dat, "names", h_names,
#if CMDTREE_ENABLE_DATA_HANDLER
                                            d_names
#else
                                            NULL
#endif
    );
    TEST_ASSERT(names != CMDTREE_NULL, "register names failed");

    /* "wait dat names" → 3 级路由 */
    resetCapture();
    int ret = cmdTree_CommandParse("wait dat names");
    TEST_ASSERT(ret == CMDTREE_OK, "3-level route should succeed");
    TEST_ASSERT(g_capturedHandler && !strcmp(g_capturedHandler, "names"),
                "should call names handler");

    /* 验证 BLE dataHandler 绑定 */
    TEST_ASSERT(cmdTree_getActiveHandler() == h_names,
                "active handler should be names");
#if CMDTREE_ENABLE_DATA_HANDLER
    TEST_ASSERT(cmdTree_getActiveDataHandler() == d_names,
                "active dataHandler should be d_names");
#else
    TEST_ASSERT(cmdTree_getActiveDataHandler() == NULL,
                "active dataHandler should be NULL when disabled");
#endif

    /* "wait" 是纯路由，无 handler → not found */
    resetCapture();
    ret = cmdTree_CommandParse("wait");
    TEST_ASSERT(ret == CMDTREE_ERR_NOT_FOUND,
                "pure routing node should not match");

    /* "wait dat" 也是纯路由 → not found */
    resetCapture();
    ret = cmdTree_CommandParse("wait dat");
    TEST_ASSERT(ret == CMDTREE_ERR_NOT_FOUND,
                "pure routing node should not match");

    cmdTree_reset();
}

static void test_intermediate_handler(void)
{
    testCnt++;
    printf("\n[TEST %d] intermediate handler: test / test hardware\n", testCnt);

    cmdTree_init();

    /* "test" 自身有 handler（显示帮助），同时有子节点 */
    cmdTreeNodeRef test = cmdTree_Register(CMDTREE_ROOT, "test", h_test, NULL);
    cmdTree_Register(test, "hardware", h_hardware, NULL);

    /* "test" → 命中 test handler */
    resetCapture();
    int ret = cmdTree_CommandParse("test");
    TEST_ASSERT(ret == CMDTREE_OK, "test alone should work");
    TEST_ASSERT(g_capturedHandler && !strcmp(g_capturedHandler, "test"),
                "should call test handler");

    /* "test hardware" → 命中 hardware handler（更深） */
    resetCapture();
    ret = cmdTree_CommandParse("test hardware");
    TEST_ASSERT(ret == CMDTREE_OK, "test hardware should work");
    TEST_ASSERT(g_capturedHandler && !strcmp(g_capturedHandler, "hardware"),
                "should call hardware handler (deeper match)");

    /* "test hardware 0" → hardware handler 拿到 "0" */
    resetCapture();
    ret = cmdTree_CommandParse("test hardware 0");
    TEST_ASSERT(ret == CMDTREE_OK, "test hardware 0 should work");
    TEST_ASSERT(g_capturedHandler && !strcmp(g_capturedHandler, "hardware"),
                "should call hardware handler");
    TEST_ASSERT(g_capturedParam0 && !strncmp(g_capturedParam0, "0", 1),
                "should receive param '0'");

    cmdTree_reset();
}

static void test_deep_nesting(void)
{
    testCnt++;
    printf("\n[TEST %d] deep nesting: 10 levels\n", testCnt);

    cmdTree_init();

    /* 构建 a/b/c/d/e/f/g/h/i/j 10 级，第 10 级有 handler */
    cmdTreeNodeRef cur = CMDTREE_ROOT;
    const char *levels[] = {"a","b","c","d","e","f","g","h","i","j"};
    for (int i = 0; i < 9; i++) {
        cur = cmdTree_Register(cur, levels[i], NULL, NULL);
        TEST_ASSERT(cur != CMDTREE_NULL, "register level %d failed", i);
    }
    cmdTree_Register(cur, "j", h_level10, NULL);

    /* "a b c d e f g h i j" → 10 级路由 */
    resetCapture();
    int ret = cmdTree_CommandParse("a b c d e f g h i j");
    TEST_ASSERT(ret == CMDTREE_OK, "10-level route should succeed");
    TEST_ASSERT(g_capturedHandler && !strcmp(g_capturedHandler, "level10"),
                "should reach level10 handler");

    /* "a b c d e" (只有 5 级，无 handler) → not found */
    resetCapture();
    ret = cmdTree_CommandParse("a b c d e");
    TEST_ASSERT(ret == CMDTREE_ERR_NOT_FOUND,
                "partial match without handler should fail");

    /* "a b c d e f g h i j extraParam" → handler 拿到 extra */
    resetCapture();
    ret = cmdTree_CommandParse("a b c d e f g h i j extraParam");
    TEST_ASSERT(ret == CMDTREE_OK, "10-level with extra param should succeed");
    TEST_ASSERT(g_capturedHandler && !strcmp(g_capturedHandler, "level10"),
                "should call level10 handler");
    TEST_ASSERT(g_capturedParam0 && !strcmp(g_capturedParam0, "extraParam"),
                "should receive extra param");

    cmdTree_reset();
}

static void test_quoted_params(void)
{
    testCnt++;
    printf("\n[TEST %d] quoted params passthrough\n", testCnt);

    cmdTree_init();
    cmdTree_Register(CMDTREE_ROOT, "device", h_device, NULL);

    /* "device \"hello world\"" → handler 拿到 token[1]="hello world" */
    resetCapture();
    int ret = cmdTree_CommandParse("device \"hello world\"");
    TEST_ASSERT(ret == CMDTREE_OK, "quoted param parse should succeed");
    TEST_ASSERT(g_capturedHandler && !strcmp(g_capturedHandler, "device"),
                "should call device handler");
    TEST_ASSERT(g_capturedParam0 && !strncmp(g_capturedParam0, "hello world", 11),
                "should receive quoted param 'hello world'");

    cmdTree_reset();
}

static void test_data_handler_binding(void)
{
    testCnt++;
    printf("\n[TEST %d] BLE data handler binding\n", testCnt);

    cmdTree_init();

    /* wait/dat/names 带 dataHandler */
    cmdTreeNodeRef wait  = cmdTree_Register(CMDTREE_ROOT, "wait", NULL, NULL);
    cmdTreeNodeRef dat   = cmdTree_Register(wait, "dat", NULL, NULL);
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

    /* 解析 "wait dat names" → dataHandler 应为 d_names */
    resetCapture();
    cmdTree_CommandParse("wait dat names");
#if CMDTREE_ENABLE_DATA_HANDLER
    TEST_ASSERT(cmdTree_getActiveDataHandler() == d_names,
                "active dataHandler should be d_names");

    /* 解析 "wait dat font" → dataHandler 应为 d_font */
    resetCapture();
    cmdTree_CommandParse("wait dat font");
    TEST_ASSERT(cmdTree_getActiveDataHandler() == d_font,
                "active dataHandler should be d_font");

    /* 解析 "wait dat names" → 直接调用 dataHandler 测试 */
    resetCapture();
    cmdTree_CommandParse("wait dat names");
    data_handler_fn_t dh = cmdTree_getActiveDataHandler();
    TEST_ASSERT(dh != NULL, "should have active data handler");
    if (dh) {
        dh("testdata", 8);
        TEST_ASSERT(g_dataHandlerCalled == 1, "data handler should be called");
    }
#else
    TEST_ASSERT(cmdTree_getActiveDataHandler() == NULL,
                "active dataHandler should be NULL when disabled");
#endif

    cmdTree_reset();
}

static void test_multiple_siblings(void)
{
    testCnt++;
    printf("\n[TEST %d] multiple siblings under same parent\n", testCnt);

    cmdTree_init();

    cmdTreeNodeRef dev = cmdTree_Register(CMDTREE_ROOT, "device", NULL, NULL);
    cmdTree_Register(dev, "reset",  h_reset,  NULL);
    cmdTree_Register(dev, "ota",    h_ota,    NULL);
    cmdTree_Register(dev, "status", h_device, NULL);

    /* "device reset" */
    resetCapture();
    int ret = cmdTree_CommandParse("device reset");
    TEST_ASSERT(ret == CMDTREE_OK, "device reset should work");
    TEST_ASSERT(g_capturedHandler && !strcmp(g_capturedHandler, "reset"),
                "should call reset");

    /* "device ota" */
    resetCapture();
    ret = cmdTree_CommandParse("device ota");
    TEST_ASSERT(ret == CMDTREE_OK, "device ota should work");
    TEST_ASSERT(g_capturedHandler && !strcmp(g_capturedHandler, "ota"),
                "should call ota");

    /* "device status" */
    resetCapture();
    ret = cmdTree_CommandParse("device status");
    TEST_ASSERT(ret == CMDTREE_OK, "device status should work");
    TEST_ASSERT(g_capturedHandler && !strcmp(g_capturedHandler, "device"),
                "should call device handler");

    cmdTree_reset();
}

static void test_edge_partial_match(void)
{
    testCnt++;
    printf("\n[TEST %d] edge cases: partial match / wrong branch\n", testCnt);

    /* === 场景1: 注册 a→b→c→d(handler), 输入 "a c" === */
    cmdTree_init();
    cmdTreeNodeRef a1  = cmdTree_Register(CMDTREE_ROOT, "a", NULL, NULL);
    cmdTreeNodeRef b1  = cmdTree_Register(a1, "b", NULL, NULL);
    cmdTreeNodeRef c1  = cmdTree_Register(b1, "c", NULL, NULL);
    cmdTree_Register(c1, "d", h_level10, NULL);

    /* "a c": c 不是 a 的直接子节点 → NOT_FOUND */
    resetCapture();
    int ret = cmdTree_CommandParse("a c");
    TEST_ASSERT(ret == CMDTREE_ERR_NOT_FOUND,
                "'a c' with tree a/b/c/d: c not child of a");

    /* "a b": b 是纯路由 → NOT_FOUND */
    resetCapture();
    ret = cmdTree_CommandParse("a b");
    TEST_ASSERT(ret == CMDTREE_ERR_NOT_FOUND,
                "'a b' with tree a/b/c/d: b is routing node");

    /* "a b c d": 完整路径 → OK */
    resetCapture();
    ret = cmdTree_CommandParse("a b c d");
    TEST_ASSERT(ret == CMDTREE_OK, "'a b c d' full path should succeed");
    TEST_ASSERT(g_capturedHandler && !strcmp(g_capturedHandler, "level10"),
                "should call d handler");

    cmdTree_reset();

    /* === 场景2: 注册 a→p→p→c→b(handler), 输入 "a p a a c d e f g" === */
    cmdTree_init();
    cmdTreeNodeRef a2  = cmdTree_Register(CMDTREE_ROOT, "a", NULL, NULL);
    cmdTreeNodeRef p1  = cmdTree_Register(a2, "p", NULL, NULL);
    cmdTreeNodeRef p2  = cmdTree_Register(p1, "p", NULL, NULL);
    cmdTreeNodeRef c2  = cmdTree_Register(p2, "c", NULL, NULL);
    cmdTree_Register(c2, "b", h_level10, NULL);

    /* "a p a a c d e f g": token[2]="a" 不在第一个 p 的子节点中 → NOT_FOUND */
    resetCapture();
    ret = cmdTree_CommandParse("a p a a c d e f g");
    TEST_ASSERT(ret == CMDTREE_ERR_NOT_FOUND,
                "'a p a a c d e f g' should break at token[2]='a'");

    /* "a p p c b": 完整路径 → OK */
    resetCapture();
    ret = cmdTree_CommandParse("a p p c b");
    TEST_ASSERT(ret == CMDTREE_OK, "'a p p c b' should succeed");
    TEST_ASSERT(g_capturedHandler && !strcmp(g_capturedHandler, "level10"),
                "should call b handler");

    /* "a p p": 第三个 p 是纯路由 → NOT_FOUND */
    resetCapture();
    ret = cmdTree_CommandParse("a p p");
    TEST_ASSERT(ret == CMDTREE_ERR_NOT_FOUND,
                "'a p p': p is routing node, no handler");

    /* "a p p c": c 是纯路由 → NOT_FOUND */
    resetCapture();
    ret = cmdTree_CommandParse("a p p c");
    TEST_ASSERT(ret == CMDTREE_ERR_NOT_FOUND,
                "'a p p c': c is routing node, no handler");

    /* "a p p c b extra": 完整路径 + 额外参数 → OK */
    resetCapture();
    ret = cmdTree_CommandParse("a p p c b extra");
    TEST_ASSERT(ret == CMDTREE_OK, "'a p p c b extra' should succeed");
    TEST_ASSERT(g_capturedHandler && !strcmp(g_capturedHandler, "level10"),
                "should call b handler");
    TEST_ASSERT(g_capturedParam0 && !strcmp(g_capturedParam0, "extra"),
                "should receive 'extra' as param");

    cmdTree_reset();

    /* === 场景3: 同名 token 在不同层级 === */
    cmdTree_init();
    cmdTree_Register(CMDTREE_ROOT, "x", h_device, NULL);
    cmdTreeNodeRef ax = cmdTree_Register(CMDTREE_ROOT, "a", NULL, NULL);
    cmdTree_Register(ax, "x", h_reset, NULL);

    /* "x" → root/x */
    resetCapture();
    ret = cmdTree_CommandParse("x");
    TEST_ASSERT(ret == CMDTREE_OK, "'x' should match root/x");
    TEST_ASSERT(g_capturedHandler && !strcmp(g_capturedHandler, "device"),
                "should call root/x handler");

    /* "a x" → root/a/x */
    resetCapture();
    ret = cmdTree_CommandParse("a x");
    TEST_ASSERT(ret == CMDTREE_OK, "'a x' should match root/a/x");
    TEST_ASSERT(g_capturedHandler && !strcmp(g_capturedHandler, "reset"),
                "should call root/a/x handler");

    cmdTree_reset();
}

static void test_show(void)
{
    testCnt++;
    printf("\n[TEST %d] cmdTree_show (visual check)\n", testCnt);

    cmdTree_init();

    cmdTreeNodeRef dev = cmdTree_Register(CMDTREE_ROOT, "device", h_device, NULL);
    cmdTree_Register(dev, "reset", h_reset, NULL);
    cmdTree_Register(dev, "ota", h_ota, NULL);

    cmdTreeNodeRef test = cmdTree_Register(CMDTREE_ROOT, "test", h_test, NULL);
    cmdTree_Register(test, "hardware", h_hardware, NULL);

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
    printf("  (visual check above — should show 3 top-level branches)\n");

    cmdTree_reset();
}

/* ==================================================================
 * main
 * ================================================================== */
int main(void)
{
    test_basic_init_reset();
    test_simple_two_level();
    test_not_found();
    test_multi_level_routing();
    test_intermediate_handler();
    test_deep_nesting();
    test_quoted_params();
    test_data_handler_binding();
    test_multiple_siblings();
    test_edge_partial_match();
    test_show();

    printf("\n=== cmdTree Results: %d/%d passed ===\n",
           testCnt - failCnt, testCnt);
    return failCnt ? 1 : 0;
}
