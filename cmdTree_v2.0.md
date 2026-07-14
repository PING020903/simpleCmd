# cmdTree v2.0 — 树形命令解析器

> 替代旧版 `CommandParseNode`（链表式）和 `CommandParseTable`（hash 表），
> 统一为 Token-by-Token 树形路由解析。

---

## 一、为什么需要 v2.0

| 旧版痛点 | v2.0 解法 |
|---------|----------|
| 两级分发 `[cmd] [param]` 后只能手动 switch | 任意深度树形路由，每层自动匹配 |
| BLE 数据回调需要独立的 `cmdBind_register` 体系 | `dataHandler` 挂在节点上，解析时自动绑定 |
| 子系统命令每次都要重复注册父命令名 | 一次注册父节点，子节点继承路由 |
| 两种解析器（Node / Table）二选一 | 双模式统一 API，切换只需一个宏 |

`[cmdTree 改进方向文档](./改进方向.md)` 记录了从旧版到 v2.0 的完整设计演进过程。

---

## 二、核心概念

### 2.1 树形路由

```
输入: "wait dat names"
路由: root -> "wait" -> "dat" -> "names" (handler)
                                     |--> 剩余参数透传

输入: "test hardware 0"
路由: root -> "test" (handler, 但更深层有 hardware)
            -> "hardware" (handler, 最深命中) → "0" 透传
```

**规则**：逐 token 下行，取最深命中 handler 的节点。未命中 → `CMDTREE_ERR_NOT_FOUND`。纯路由节点（无 handler）不参与匹配。

### 2.2 FNV Hash 匹配

每个 token 用 FNV-1a（prime=`0x01000193`, offset=`0x811C9DC5`）做 hash，节点间 hash 比对。token 先由分词器切割为 `userString`（指针 + 长度），hash 只算 token 内容，不包含引号。

### 2.3 双模式（同一套 API）

| | 静态模式 | 动态模式 |
|---|---|---|
| 存储 | `cmdTreeNode_t[CMDTREE_STATIC_MAX_NODES]` 数组 | 每个节点 `cmd_MemoryAlloc` 分配 |
| 节点引用 | `CMDTREE_STATIC_INDEX_TYPE`（默认 `short`，可配） | `cmdTreeNode_t*`（指针） |
| 子节点遍历 | `first_child → next_sibling` 链 | `ll_t` 双向链表 |
| 深度限制 | `CMDTREE_STATIC_MAX_DEPTH` 硬限制 | 无限制（仅受内存约束） |
| 节点上限 | `CMDTREE_STATIC_MAX_NODES` 硬限制 | 无限制（仅受内存约束） |
| 适用场景 | 嵌入式 MCU（无 malloc / SRAM 紧张） | 桌面 / 资源充足 |

切换模式只改 `cmdTreeCfg.h`：

```c
#define CMDTREE_MODE_STATIC   1   // 嵌入式
#define CMDTREE_MODE_DYNAMIC  0
```

---

## 三、配置

所有功能开关集中在 [`cmdTreeCfg.h`](include/cmdTreeCfg.h)：

| 宏 | 默认值 | 说明 |
|----|-------|------|
| `CMDTREE_MODE_STATIC` | 0 | 静态数组模式 |
| `CMDTREE_MODE_DYNAMIC` | 1 | 动态分配模式 |
| `CMDTREE_STATIC_MAX_NODES` | 32 | 静态模式最大节点数（含根） |
| `CMDTREE_STATIC_MAX_DEPTH` | 12 | 静态模式最大路由深度 |
| `CMDTREE_STATIC_INDEX_TYPE` | `short` | 静态模式索引类型（须有符号：`int8_t` / `short` / `int`） |
| `CMDTREE_ENABLE_DATA_HANDLER` | 1 | BLE 数据回调开关（0=禁用，节省节点内存） |
| `ENABLE_WCHAR` | 1 | 宽字符分词 `ParseSpaceW` |
| `CMDTREE_ENABLE_HELP` | 1 | 内置 "help" 指令（显示所有注册命令及层级关系），0=禁用（节省节点内存）<br>**注意**：token 直接存 `const char*` 指针不做拷贝，仅支持编译期字面量注册的命令，运行时动态构造的字符串无法通过 `help` 正确显示 |
| `PARSE_SIZE` | 128 | 输入缓冲区大小 |

---

## 四、API 参考

### 4.1 类型定义

```c
// 命令 handler — arg 为 userString*（剩余参数数组），无参数时为 NULL
typedef void (*handler_fn_t)(void *arg);

// BLE 数据回调 — pBuff 为二进制数据，len 为长度，返回 0=成功
typedef int (*data_handler_fn_t)(const void *pBuff, const int len);

// 节点引用（类型随模式变化：静态 = CMDTREE_STATIC_INDEX_TYPE，动态 = 指针）
typedef /* ... */ cmdTreeNodeRef;

#define CMDTREE_NULL  /* -1 或 NULL */
#define CMDTREE_ROOT  (cmdTree_getRoot())
```

### 4.2 错误码

| 值 | 名称 | 含义 |
|----|------|------|
| 0 | `CMDTREE_OK` | 成功 |
| -1 | `CMDTREE_ERR_NOT_FOUND` | 未匹配到任何 handler |
| -2 | `CMDTREE_ERR_NULL_PARENT` | parent 无效 |
| -3 | `CMDTREE_ERR_MEM` | 内存不足（动态模式）或表满（静态模式） |
| -4 | `CMDTREE_ERR_NOT_INIT` | 未调用 `cmdTree_init` |
| -5 | `CMDTREE_ERR_BUSY` | 上次解析未释放 |

### 4.3 函数

#### `void cmdTree_init(void)`

初始化树。动态模式分配根节点，静态模式初始化数组索引 0。重复调用无副作用。

#### `cmdTreeNodeRef cmdTree_Register(cmdTreeNodeRef parent, const char *token, handler_fn_t handler, data_handler_fn_t dataHandler)`

注册一个命令节点。

| 参数 | 说明 |
|------|------|
| `parent` | 父节点引用，根节点的子命令传入 `CMDTREE_ROOT` |
| `token` | 命令字符串（如 `"device"`、`"reset"`） |
| `handler` | 命令回调，`NULL` 表示纯路由节点 |
| `dataHandler` | BLE 数据回调，`NULL` 表示无 |

返回新节点引用，失败返回 `CMDTREE_NULL`。

#### `int cmdTree_CommandParse(const char *commandString)`

解析命令字符串并调用对应 handler。

- 先调用 `ParseSpace()` 分词（支持双引号 raw 数据）
- 逐 token 沿树下行，取最深命中 handler
- 自动记录 `dataHandler` 供后续 BLE 数据回调
- 剩余参数以 `userString*` 形式透传给 handler
- 返回 `CMDTREE_OK` 或错误码

#### `void cmdTree_reset(void)`

释放/清空所有节点。动态模式递归 free，静态模式清零数组。

#### `cmdTreeNodeRef cmdTree_RegisterHelp(cmdTreeNodeRef parent)`

注册内置 `help` 指令。应在所有其他命令注册完成后最后调用。

> **限制**：`help` 显示的命令字符串来自注册时传入的 `token` 指针，不做拷贝。仅支持编译期字面量（如 `"device"`）或全局持久缓冲区。运行时在栈上构造的临时字符串无法正确显示（指针失效或内容被覆盖）。

#### `void cmdTree_showHelp(void)`

显示所有已注册命令及其层级关系（需启用 `CMDTREE_ENABLE_HELP`）。遍历树结构，通过节点中存储的 `token` 指针打印命令字符串。带有 handler 的节点标注 `<- [cmd]`。

#### `cmdTree_err_t cmdTree_GetLastError(void)`

获取最近一次操作的错误码。

#### `void cmdTree_show(void)`

打印树结构到 stdout（调试用）。

```
cmdTree (dynamic):
[00000000]
  [d07076f3] H          ← "device"，H=有 handler
    [650d33c0] H        ←   "reset"
    [d218352d] H        ←   "ota"
  [892e4ca0]            ← "wait"，纯路由
    [d74f0d86]          ←   "dat"
      [e6e5718f] H D    ←     "names"，D=有 dataHandler
```

#### `handler_fn_t cmdTree_getActiveHandler(void)`

获取最近一次解析命中的 handler。

#### `data_handler_fn_t cmdTree_getActiveDataHandler(void)`

获取最近一次解析命中的 `dataHandler`。用于 BLE 场景：解析命令后，取此指针，BLE 数据到达时调用。

---

## 五、Token 分词器

`ParseSpace(const char *input)` 将输入字符串切割为 `userString` 数组，存储在内部缓冲区。

### 双引号支持

空格分隔 token，英文双引号 `"` 包裹的内容视为一个 token：

| 输入 | 分词结果 |
|------|---------|
| `cmd param` | `["cmd", "param"]` |
| `cmd "hello world"` | `["cmd", "hello world"]` |
| `"raw data" cmd` | `["raw data", "cmd"]` |
| `"unclosed` | `["unclosed"]` |
| `""` | `[""]` |

### 宽字符

当 `ENABLE_WCHAR=1` 时，`ParseSpaceW(const wchar_t *)` 可用，规则与窄字符一致。

---

## 六、BLE 数据回调集成

v1.0 需要两套独立的注册系统，v2.0 只需在注册时传入 `dataHandler`：

```c
// 注册时就把 dataHandler 挂在节点上
cmdTreeNodeRef names = cmdTree_Register(dat, "names", h_names, d_names);

// 解析命令 → 自动设置 active dataHandler
cmdTree_CommandParse("wait dat names");

// BLE 收到数据 → 取出回调直接调用
data_handler_fn_t dh = cmdTree_getActiveDataHandler();
if (dh) dh(bleRecvBuf, len);
```

不再需要 `cmdBind_register`、`RECORD_CMD_CALL`、`cmdBind_findByHandler` 等旧 API。

---

## 七、使用示例

### 基本两级路由

```c
cmdTree_init();

cmdTreeNodeRef dev = cmdTree_Register(CMDTREE_ROOT, "device", h_device, NULL);
cmdTree_Register(dev, "reset", h_reset, NULL);
cmdTree_Register(dev, "ota",   h_ota,   NULL);

cmdTree_CommandParse("device");        // → h_device
cmdTree_CommandParse("device reset");  // → h_reset
cmdTree_CommandParse("device ota");    // → h_ota

cmdTree_reset();
```

### 多级路由 + 中间 handler

```c
cmdTreeNodeRef test = cmdTree_Register(CMDTREE_ROOT, "test", h_test, NULL);
cmdTree_Register(test, "hardware", h_hardware, NULL);
cmdTree_Register(test, "flash",    h_flash,    NULL);

cmdTree_CommandParse("test");            // → h_test（一级命中）
cmdTree_CommandParse("test hardware");   // → h_hardware（更深级命中）
cmdTree_CommandParse("test hardware 0"); // → h_hardware，透传 "0"
```

### 纯路由节点（只做路由，不响应）

```c
// "wait" 和 "dat" 无 handler，只是把路铺到 "names" / "font"
cmdTreeNodeRef wait = cmdTree_Register(CMDTREE_ROOT, "wait", NULL, NULL);
cmdTreeNodeRef dat  = cmdTree_Register(wait, "dat", NULL, NULL);
cmdTree_Register(dat, "names", h_names, d_names);
cmdTree_Register(dat, "font",  h_font,  d_font);

cmdTree_CommandParse("wait");         // NOT_FOUND（纯路由无 handler）
cmdTree_CommandParse("wait dat");     // NOT_FOUND（纯路由无 handler）
cmdTree_CommandParse("wait dat names"); // → h_names
```

### 交互式 REPL

```c
char input[PARSE_SIZE];
cmdTree_init();
// ... 注册命令 ...

while (1) {
    printf("> ");
    if (!fgets(input, PARSE_SIZE, stdin)) break;
    input[strcspn(input, "\n")] = '\0';

    int ret = cmdTree_CommandParse(input);
    if (ret == CMDTREE_ERR_NOT_FOUND)
        printf("unknown: '%s'\n", input);
}
cmdTree_reset();
```

完整可运行的 demo 见 [`main.c`](sources/main.c)。

---

## 八、从 v1.x 迁移

| v1.x | v2.0 |
|------|------|
| `cmdTable_RegisterCMD("cmd", len, "param", len, handler)` | `cmdTree_Register(CMD, "cmd", NULL, NULL); cmdTree_Register(cmd, "param", handler, NULL)` |
| `cmdTable_CommandParse(str)` | `cmdTree_CommandParse(str)` |
| `cmdNode_RegisterCommand(0, "cmd")` | `cmdTree_Register(CMDTREE_ROOT, "cmd", handler, dataHandler)` |
| `cmdNode_RegisterParameter(node, handler, 0, "param")` | `cmdTree_Register(node, "param", handler, NULL)` |
| `cmdNode_showList()` | `cmdTree_show()` |
| `cmdBind_register(...)` | 已在 `cmdTree_Register` 的 `dataHandler` 参数中完成 |
| `RECORD_CMD_CALL(...)` | 解析时自动设置，取 `cmdTree_getActiveDataHandler()` |

---

## 九、文件索引

| 文件 | 说明 |
|------|------|
| [`include/cmdTreeCfg.h`](include/cmdTreeCfg.h) | 功能配置（模式切换） |
| [`include/CommandParseTree.h`](include/CommandParseTree.h) | 头文件（类型定义 + API） |
| [`sources/CommandParseTree.c`](sources/CommandParseTree.c) | 实现（双模式） |
| [`include/cmdUserStringParse.h`](include/cmdUserStringParse.h) | 分词器头文件 |
| [`sources/cmdUserStringParser.c`](sources/cmdUserStringParser.c) | 分词器实现（双引号 + 宽字符） |
| [`sources/main.c`](sources/main.c) | 演示程序（4 个 case + REPL） |
| [`test_cmdTree.c`](test_cmdTree.c) | 测试用例（11 项） |
| [`改进方向.md`](改进方向.md) | 设计文档 |
