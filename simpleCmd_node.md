
# 简易终端命令解析系统 - 节点模式

## 目录

- [功能概览](#功能概览)
- [核心概念](#核心概念)
- [API 参考](#api-参考)
- [使用示例](#使用示例)
- [错误码说明](#错误码说明)
- [注意事项](#注意事项)
- [移植指南](#移植指南)

---

## 功能概览

本系统提供基于链表结构的命令行解析功能，支持命令和参数的动态注册与管理。

### 核心功能

- ✅ **注册命令** `cmdNode_RegisterCommand`
- ✅ **取消注册命令** `cmdNode_unRegisterCommand`
- ✅ **取消注册全部命令** `cmdNode_unRegisterAllCommand`
- ✅ **查找命令节点** `cmdNode_FindCommand`
- ✅ **更新命令名称** `cmdNode_updateCommand`
- ✅ **注册参数** `cmdNode_RegisterParameter`
- ✅ **取消注册单个命令下的所有参数** `cmdNode_unRegisterAllParameters`
- ✅ **取消注册参数** `cmdNode_unRegisterParameter`
- ✅ **更新参数** `cmdNode_updateParameter`
- ✅ **命令解析** `cmdNode_CommandParse`
- ✅ **命令解析 (宽字符)** `cmdNode_CommandParseW`
- ✅ **获取上次错误** `cmdNode_GetLastError`
- ✅ **打印已注册的命令和参数** `cmdNode_showList`
- ✅ **打印指定命令的参数** `cmdNode_showParam`
- ✅ **获取命令映射表** `NodeGetCommandMap` (仅调试/注册模式)
- ✅ **初始化默认注册命令** `defaultRegCmd_init` (仅调试/注册模式)

### 附加功能（字符串解析）

- ✅ **获取用户参数个数** `userParse_GetUserParamCnt`
- ✅ **获取用户数据指针** `userParse_pUserData`
- ✅ **解析空格分隔的参数** `ParseSpace` / `ParseSpaceW`
- ✅ **重置用户数据记录** `RESET_USERDATA_RECORD`

---

## 核心概念

### 指令格式

系统使用 `[command] [param] [userParam]...` 格式：

- **command**: 命令词（必须是已注册的命令）
- **param**: 参数词（必须是该命令下已注册的参数）
- **userParam**: 用户自定义参数（可选，传递给处理函数的字符串）

### 重要规则

⚠️ **TIPS**:

1. **command 必须带 param**: 不能单独调用 `command`，因为 `command` 没有直接的处理函数，必须带有 `param` 才能正常调用处理函数
2. **userParam 可选**: 若解析的指令后面没有跟随 `userParam` 则直接调用处理函数（参数为 NULL），有 `userParam` 则会作为字符串参数传递给处理函数
3. **大小写转换**: `command` 为英文时，会被强制转换为小写。调试时无论输入大写还是小写均为同样效果
4. **字符集**: `command` 首选推荐英文，中文 (GB2312, UTF-8) 可能会出现未知情况
5. **param 继承性**: `param` 是否支持宽字符继承自 `command` 的设置
6. **空格限制**: `param` 不支持输入空格字符
7. **源字符串保护**: 若传入到用户注册函数的是源字符串，请不要修改它（实际为 `const` 属性）

### 数据结构

#### 命令节点 (command_node)

```c
typedef struct {
    void* prev;                      // 上一节点
    void* next;                      // 下一节点
    bool isWch : 1;                  // 该命令是否使用宽字符
    wchar_t command_string[MAX_COMMAND];  // 命令字符串
    void* ParameterNode_head;        // 该命令下的参数链表头
} command_node;
```

#### 参数节点 (parameter_node)

```c
typedef struct {
    void* prev;                      // 上一节点
    void* next;                      // 下一节点
    bool isRawStr : 1;               // 传递参数的形式是否为原字符串
    wchar_t parameter_string[MAX_PARAMETER];  // 参数字符串
    void* handlerArg;                // handler 的参数
    ParameterHandler handler;        // 参数处理函数
} parameter_node;

typedef void (*ParameterHandler)(void* arg);  // 命令参数处理函数类型
```

#### 用户字符串 (userString)

```c
typedef struct {
    void* strHead;                   // 字符串起始地址
    size_t len;                      // 字符串长度
} userString;
```

---

## API 参考

### 命令管理

#### 注册命令 `cmdNode_RegisterCommand`

```c
int cmdNode_RegisterCommand(const bool isWch, const void* cmdStr);
```

**参数说明**:
- `isWch`: 是否使用宽字符 (`true`=宽字符，`false`=普通字符)
- `cmdStr`: 命令字符串（`char*` 或 `wchar_t*`，取决于 `isWch`）

**返回值**:
- `NODE_OK` (0): 注册成功
- `NODE_ARG_ERR` (-2): 参数错误（cmdStr 为 NULL）
- `NODE_CMD_TOO_LONG` (-10): 命令长度超过 `MAX_COMMAND` (16)
- `NODE_REPEATING` (-9): 命令已存在
- `NODE_FAIL` (-1): 注册失败（通常为内存分配失败）

**示例**:
```c
// 注册普通字符命令
cmdNode_RegisterCommand(false, "help");

// 注册宽字符命令（需 ENABLE_WCHAR=1）
cmdNode_RegisterCommand(true, L"帮助");
```

**注意**: 若输入英文命令，所有大写字符都会被转换为小写

---

#### 取消注册命令 `cmdNode_unRegisterCommand`

```c
int cmdNode_unRegisterCommand(const char* command, const wchar_t* commandW);
```

**参数说明**:
- `command`: 非宽字符命令（若使用宽字符则传 NULL）
- `commandW`: 宽字符命令（若使用普通字符则传 NULL）

**返回值**:
- `NODE_OK`: 删除成功
- `NODE_ARG_ERR`: 参数错误（两个参数都为 NULL）
- `NODE_CMD_TOO_LONG`: 命令过长
- `NODE_NOT_YET_INIT` (-13): 命令链表尚未初始化
- `NODE_NOT_FIND_CMD` (-4): 未找到该命令
- `NODE_FAIL`: 删除失败

**示例**:
```c
// 删除普通字符命令
cmdNode_unRegisterCommand("help", NULL);

// 删除宽字符命令
cmdNode_unRegisterCommand(NULL, L"帮助");
```

---

#### 取消注册全部命令 `cmdNode_unRegisterAllCommand`

```c
int cmdNode_unRegisterAllCommand(void);
```

**返回值**:
- `NODE_OK`: 清空成功
- `NODE_FAIL`: 清空失败
- `NODE_NOT_YET_INIT`: 命令链表尚未初始化

---

#### 查找命令节点 `cmdNode_FindCommand`

```c
command_node* cmdNode_FindCommand(const char* command, const wchar_t* commandW);
```

**参数说明**:
- `command`: 非宽字符命令
- `commandW`: 宽字符命令

**返回值**:
- `command_node*`: 找到的命令节点指针
- `NULL`: 未找到或参数错误

**示例**:
```c
command_node* node = cmdNode_FindCommand("help", NULL);
if (node == NULL) {
    printf("Command not found\n");
}
```

---

#### 更新命令 `cmdNode_updateCommand`

```c
int cmdNode_updateCommand(
    char* oldCommand, 
    wchar_t* oldCommandW,
    char* newCommand, 
    wchar_t* newCommandW
);
```

**参数说明**:
- `oldCommand` / `oldCommandW`: 旧命令名（二选一，另一个传 NULL）
- `newCommand` / `newCommandW`: 新命令名（二选一，另一个传 NULL）

**返回值**:
- `NODE_OK`: 更新成功
- `NODE_ARG_ERR`: 参数错误
- `NODE_NOT_FIND_CMD` (-4): 未找到旧命令
- `NODE_REPEATING` (-9): 新命令名已存在
- `NODE_NOT_YET_INIT`: 链表未初始化
- `NODE_FAIL`: 更新失败

**示例**:
```c
// 将 "help" 重命名为 "assistance"
cmdNode_updateCommand("help", NULL, "assistance", NULL);
```

---

### 参数管理

#### 注册参数 `cmdNode_RegisterParameter`

```c
int cmdNode_RegisterParameter(
    command_node* node,
    ParameterHandler hook,
    const bool isRaw,
    const void* paramStr
);
```

**参数说明**:
- `node`: 要注册参数的命令节点
- `hook`: 参数处理函数指针
- `isRaw`: 是否给 `hook` 传递原始字符串（`true`=传递 `userString*`，`false`=传递解析后的参数）
- `paramStr`: 参数字符串

**返回值**:
- `NODE_OK`: 注册成功
- `NODE_ARG_ERR`: 参数错误（node 为 NULL、hook 为 NULL 或 paramStr 为 NULL）
- `NODE_CMD_NODE_NULL` (-7): 命令节点为空
- `NODE_PARAM_TOO_LONG` (-11): 参数长度超过 `MAX_PARAMETER` (16)
- `NODE_REPEATING`: 参数已存在
- `NODE_FAIL`: 注册失败

**示例**:
```c
void helpHandler(void* arg) {
    printf("Help command executed\n");
}

command_node* cmdNode = cmdNode_FindCommand("help", NULL);
cmdNode_RegisterParameter(cmdNode, helpHandler, false, "all");
```

**注意**: 相较于 `command`，`parameter` 的定义更自由，除了不能输入空格外

---

#### 取消注册单个命令下的所有参数 `cmdNode_unRegisterAllParameters`

```c
int cmdNode_unRegisterAllParameters(command_node* node);
```

**参数说明**:
- `node`: 命令节点

**返回值**:
- `NODE_OK`: 删除成功
- `NODE_ARG_ERR`: 参数错误（node 为 NULL）
- `NODE_PARAM_NODE_NULL` (-8): 参数节点为空（本身就没有参数）

---

#### 取消注册参数 `cmdNode_unRegisterParameter`

```c
int cmdNode_unRegisterParameter(
    command_node* node,
    const void* paramStr
);
```

**参数说明**:
- `node`: 要被删除参数的命令节点
- `paramStr`: 要删除的参数名

**返回值**:
- `NODE_OK`: 删除成功
- `NODE_ARG_ERR`: 参数错误
- `NODE_NOT_FIND_PARAM` (-5): 未找到该参数
- `NODE_PARAM_TOO_LONG`: 参数过长
- `NODE_FAIL`: 删除失败

**示例**:
```c
cmdNode_unRegisterParameter(cmdNode, "all");
```

---

#### 更新参数 `cmdNode_updateParameter`

```c
int cmdNode_updateParameter(
    const command_node* CmdNode, 
    ParameterHandler hook,
    const bool isRaw, 
    const void* oldParam, 
    const void* newParam
);
```

**参数说明**:
- `CmdNode`: 要被更改参数的命令节点
- `hook`: 新的参数处理函数
- `isRaw`: 是否传递原始字符串
- `oldParam`: 旧参数名
- `newParam`: 新参数名

**返回值**:
- `NODE_OK`: 更新成功
- `NODE_ARG_ERR`: 参数错误
- `NODE_REPEATING`: 新参数名已存在
- `NODE_NOT_FIND_PARAM`: 未找到旧参数

**示例**:
```c
void newHandler(void* arg);
cmdNode_updateParameter(cmdNode, newHandler, false, "all", "full");
```

---

### 命令解析

#### 命令解析 `cmdNode_CommandParse`

```c
int cmdNode_CommandParse(const char* commandString);
```

**参数说明**:
- `commandString`: 用户输入的完整命令字符串

**返回值**:
- `NODE_OK`: 解析并执行成功
- `NODE_ARG_ERR`: 参数错误（输入为 NULL）
- `NODE_CMD_TOO_LONG`: 命令部分过长
- `NODE_PARAM_TOO_LONG`: 参数部分过长
- `NODE_NOT_FIND_CMD`: 未找到命令
- `NODE_NOT_FIND_PARAM`: 未找到参数
- `NODE_PARSE_ERR` (-12): 字符串解析错误
- `NODE_NOT_YET_INIT`: 链表未初始化
- `NODE_NO_HANDLER` (-15): 参数没有注册处理函数

**示例**:
```c
// 解析命令 "help all"
int ret = cmdNode_CommandParse("help all");
if (ret != NODE_OK) {
    cmdNode_GetLastError();  // 打印错误信息
}

// 解析带用户参数的命令 "reg cmd mycommand"
cmdNode_CommandParse("reg cmd mycommand");
```

**注意**: `command` 和 `parameter` 之间的空格都会被忽略，`parameter` 和 `userParam` 之间也是如此

---

#### 命令解析 (宽字符) `cmdNode_CommandParseW`

```c
int cmdNode_CommandParseW(const wchar_t* commandString);
```

**参数说明**:
- `commandString`: 用户输入的宽字符命令字符串

**返回值**: 同 `cmdNode_CommandParse`

**注意**: 需要 `ENABLE_WCHAR=1` 且系统支持宽字符

---

### 辅助功能

#### 获取上次错误 `cmdNode_GetLastError`

```c
int cmdNode_GetLastError(void);
```

**返回值**: 上一次链表管理函数的错误码

**示例**:
```c
int err = cmdNode_GetLastError();
// 会自动打印错误详情到调试输出
```

---

#### 打印命令列表 `cmdNode_showList`

```c
void cmdNode_showList(void);
```

**功能**: 打印当前已注册的所有命令及其参数

**示例输出**:
```
1:command  <help>     
  parameters:
        all
        version

2:command  <reg>     
  parameters:
        cmd
        param
        DelCmd
```

---

#### 打印指定命令的参数 `cmdNode_showParam`

```c
void cmdNode_showParam(command_node* CmdNode);
```

**参数说明**:
- `CmdNode`: 要查看的命令节点

**示例**:
```c
command_node* node = cmdNode_FindCommand("help", NULL);
cmdNode_showParam(node);
```

---

#### 获取用户参数个数 `simpleCmd_GetUserParamsCnt`

```c
#define simpleCmd_GetUserParamsCnt() userParse_GetUserParamCnt()
```

**返回值**: 当前解析到的用户参数个数

---

### 高级功能（仅 ENABLE_REG 模式）

#### 获取命令映射表 `NodeGetCommandMap`

```c
int NodeGetCommandMap(command_info** map);
```

**参数说明**:
- `map`: 输出的命令信息数组指针

**返回值**: 命令数量

**用途**: 用于遍历所有已注册的命令

---

#### 初始化默认注册命令 `defaultRegCmd_init`

```c
int defaultRegCmd_init(void);
```

**功能**: 初始化默认的 "reg" 命令及其参数，用于运行时动态注册

**注册的默认命令**:
- `reg cmd [command]`: 注册新命令
- `reg param [command] [param] [handler_addr]`: 注册参数
- `reg DelCmd [command]`: 删除命令
- `reg DelAllParam [command]`: 删除命令的所有参数
- `reg DelParam [command] [param]`: 删除指定参数
- `reg DelAllCmd`: 删除所有命令（"reg" 除外）
- `reg ls`: 显示命令列表

---

## 错误码说明

| 错误码 | 值 | 说明 |
|--------|-----|------|
| `NODE_OK` | 0 | 成功 |
| `NODE_FAIL` | -1 | 通用失败 |
| `NODE_ARG_ERR` | -2 | 函数参数错误 |
| `NODE_NOT_FIND` | -3 | 未找到（通用） |
| `NODE_NOT_FIND_CMD` | -4 | 未找到命令 |
| `NODE_NOT_FIND_PARAM` | -5 | 未找到参数 |
| `NODE_ALLOC_ERR` | -6 | 内存分配失败 |
| `NODE_CMD_NODE_NULL` | -7 | 命令节点为空 |
| `NODE_PARAM_NODE_NULL` | -8 | 参数节点为空 |
| `NODE_REPEATING` | -9 | 节点重复 |
| `NODE_CMD_TOO_LONG` | -10 | 命令过长（>16） |
| `NODE_PARAM_TOO_LONG` | -11 | 参数过长（>16） |
| `NODE_PARSE_ERR` | -12 | 字符串解析错误 |
| `NODE_NOT_YET_INIT` | -13 | 尚未初始化 |
| `NODE_UNSUPPORT` | -14 | 不支持的操作 |
| `NODE_NO_HANDLER` | -15 | 没有处理函数 |

---

## 使用示例

### 完整示例

```c
#include "CommandParse.h"
#include <stdio.h>

// 参数处理函数示例
void printVersion(void* arg) {
    printf("Version 1.0.0\n");
}

void showHelp(void* arg) {
    printf("Available commands:\n");
    printf("  help     - Show this help\n");
    printf("  version  - Show version\n");
}

int main() {
    // 1. 注册命令
    cmdNode_RegisterCommand(false, "help");
    cmdNode_RegisterCommand(false, "version");
    
    // 2. 查找命令节点
    command_node* helpNode = cmdNode_FindCommand("help", NULL);
    command_node* versionNode = cmdNode_FindCommand("version", NULL);
    
    // 3. 注册参数
    if (helpNode) {
        cmdNode_RegisterParameter(helpNode, showHelp, false, "all");
    }
    
    if (versionNode) {
        cmdNode_RegisterParameter(versionNode, printVersion, false, "now");
    }
    
    // 4. 显示已注册的命令
    cmdNode_showList();
    
    // 5. 解析用户输入
    const char* userInput = "help all";
    int result = cmdNode_CommandParse(userInput);
    
    if (result != NODE_OK) {
        cmdNode_GetLastError();
    }
    
    // 6. 清理资源
    cmdNode_unRegisterAllCommand();
    
    return 0;
}
```

### 带用户参数的示例

```c
#include "CommandParse.h"
#include <string.h>

// 处理函数 - 接收原始字符串
void echoHandler(void* arg) {
    userString* userData = (userString*)arg;
    if (userData) {
        char buffer[PARSE_SIZE] = {0};
        memcpy(buffer, userData->strHead, userData->len);
        printf("Echo: %s\n", buffer);
    }
}

int main() {
    // 注册命令和参数
    cmdNode_RegisterCommand(false, "echo");
    command_node* echoNode = cmdNode_FindCommand("echo", NULL);
    
    // 注册参数，设置 isRaw=true 以接收原始字符串
    cmdNode_RegisterParameter(echoNode, echoHandler, true, "text");
    
    // 解析带用户参数的命令
    cmdNode_CommandParse("echo text Hello World");
    // 输出：Echo: Hello World
    
    return 0;
}
```

### 使用默认注册命令（运行时动态注册）

```c
#include "CommandParse.h"

int main() {
    // 初始化默认的 reg 命令
    defaultRegCmd_init();
    
    // 现在可以通过命令行注册新命令
    // 例如：reg cmd mycmd
    //       reg param mycmd myparam 0x12345678
    cmdNode_CommandParse("reg cmd mycmd");
    cmdNode_CommandParse("reg param mycmd myparam 0x00000000");
    
    // 查看所有命令
    cmdNode_CommandParse("reg ls");
    
    return 0;
}
```

---

## 注意事项

### 内存管理

1. **自动释放**: 取消注册命令时会自动释放关联的参数节点
2. **手动清理**: 程序退出前建议调用 `cmdNode_unRegisterAllCommand()` 释放所有资源
3. **用户字符串**: `ParseSpace` 函数会动态分配内存，每次解析后会自动释放

### 字符限制

1. **命令长度**: 最大 16 个字符（`MAX_COMMAND`）
2. **参数长度**: 最大 16 个字符（`MAX_PARAMETER`）
3. **缓冲区大小**: `PARSE_SIZE` 定义为 128，用户输入不应超过此长度
4. **非法字符**: 命令和参数中不能包含空格

### 宽字符支持

1. **编译选项**: 需要设置 `ENABLE_WCHAR=1`
2. **一致性**: 命令和参数的字符类型必须一致
3. **平台支持**: 需要系统支持 `wchar.h`

### 线程安全

⚠️ **警告**: 当前实现**不是线程安全的**，多线程环境下需要自行加锁

### 最佳实践

1. **始终检查返回值**: 所有 API 都返回状态码，应该检查是否成功
2. **使用 GetLastError**: 出错时调用 `cmdNode_GetLastError()` 获取详细信息
3. **合理的命令结构**: 设计清晰的命令层次结构，便于维护
4. **参数验证**: 在处理函数中验证用户传入的参数

---

## 移植指南

### 必需的头文件

```c
#include <stdio.h>      // printf, wprintf
#include <string.h>     // strlen, strcpy, strcmp, memcpy
#include <wchar.h>      // wcslen, wcscpy, wcscmp (宽字符支持)
#include <stdlib.h>     // malloc, free
#include <stdbool.h>    // bool 类型
#include "cmdUserStringParse.h"
```

### 配置宏

在 `CommandParse.h` 中配置：

```c
#define CMD_METHOD_NODE 1      // 启用节点模式
#define CMD_METHOD_TABLE 0     // 启用哈希表模式（另一种实现）
#define ENABLE_WCHAR 0         // 启用宽字符支持
#define ENABLE_REG 1           // 启用运行时注册功能
#define MAX_COMMAND 16         // 最大命令长度
#define MAX_PARAMETER 16       // 最大参数长度
#define PARSE_SIZE 128         // 解析缓冲区大小
#define MAX_HASH_LIST 16       // 哈希表大小（仅 TABLE 模式）
```

### 依赖的库函数

- **内存管理**: `malloc`, `free`, `realloc`
- **字符串操作**: `strlen`, `strcpy`, `strcmp`, `memcpy`, `memset`
- **宽字符操作** (可选): `wcslen`, `wcscpy`, `wcscmp`
- **格式化输出**: `printf`, `wprintf`

---

## 与其他模式的对比

### 节点模式 (NODE) vs 哈希表模式 (TABLE)

| 特性 | 节点模式 | 哈希表模式 |
|------|----------|------------|
| 数据结构 | 双向链表 | 哈希表 |
| 查找速度 | O(n) | O(1) 平均 |
| 内存占用 | 较高 | 较低 |
| 动态扩展 | 支持 | 固定大小 |
| 命令更新 | 支持 | 支持 |
| 适用场景 | 命令数量少，频繁增删 | 命令固定，快速查找 |

**选择建议**:
- 命令数量 < 100 且经常变化 → 使用 **节点模式**
- 命令固定且追求性能 → 使用 **哈希表模式**

---

## 版本历史

- **v1.0**: 初始版本，实现基本的命令注册和解析功能
- **v1.1**: 添加宽字符支持
- **v1.2**: 添加运行时动态注册功能
- **v1.3**: 优化内存管理，添加更多错误码

---

## 常见问题 (FAQ)

### Q: 为什么我的命令找不到？
A: 检查以下几点：
1. 命令是否已成功注册
2. 命令长度是否超过 16 个字符
3. 是否有大小写问题（英文命令会被转为小写）
4. 调用 `cmdNode_GetLastError()` 查看详细错误

### Q: 如何处理带空格的参数？
A: 当前版本不支持带空格的参数。如需支持，可以：
1. 使用多个参数拼接
2. 使用特殊字符替代空格
3. 修改 `passableChParam` 函数放宽限制

### Q: 宽字符编译失败怎么办？
A: 确保：
1. 设置 `ENABLE_WCHAR=1`
2. 编译器支持宽字符
3. 使用正确的宽字符字面量（如 `L"中文"`）

### Q: 内存泄漏怎么办？
A: 遵循以下原则：
1. 程序结束前调用 `cmdNode_unRegisterAllCommand()`
2. 不要手动修改节点内部结构
3. 不要修改传递给处理函数的源字符串

---

## 许可证

本项目采用 MIT 许可证，详见 LICENSE 文件。

---

## 联系方式

如有问题或建议，请提交 Issue 或 Pull Request。
```

这个更新后的文档包含了：

1. ✅ **完整的目录结构**，方便查阅
2. ✅ **详细的功能列表**，一目了然
3. ✅ **核心概念解释**，帮助理解系统架构
4. ✅ **完整的 API 文档**，包括参数、返回值、示例
5. ✅ **丰富的使用示例**，从简单到复杂
6. ✅ **错误码表格**，便于调试
7. ✅ **注意事项和最佳实践**，避免常见错误
8. ✅ **移植指南**，方便在其他项目中使用
9. ✅ **FAQ 常见问题解答**
10. ✅ **版本历史记录**

文档已经更新完成，您可以直接使用这个新版本替换原有的 `simpleCmd_node.md` 文件。