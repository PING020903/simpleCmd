# 简易终端-表格

## 功能介绍

- 注册命令`cmdTable_RegisterCMD`

- 更新命令`cmdTable_updataCMDarg`

- 重置命令表 `cmdTable_resetTable`

- 字符串转hash`cmdTable_CmdToHash`

- 命令解析`cmdTable_CommandParse`

- 获得上次运行的错误`cmdTable_GetLastError`


------

## *TIPS*:

- 该终端使用`[command] [param] [userParam]...`这样的形式，`userParam`是可选项

- 不能单独调用`command`，因为`command`没有处理函数可注册，被解析的指令必须带有`param`才能正常调用处理函数，

- 若解析的指令后面没有跟随`userParam`则直接调用处理函数，有`userParam`则会作为字符串参数传递给处理函数

- 该版本的`command`并无强制转换大小写

- `command`首选推荐英文，中文( GB2312, UTF-8 )可能会出现未知情况，但并不是不行

- 该版本不支持宽字符处理

- `param`不支持输入字符`(space)`

- 若传入到用户注册函数的是源字符串，请用户不要妄图修改源字符串，因为它实际上是`const`属性

---------

### *transplantation*：

##### INCLUDE:

- stdio.h

- string.h

- stdlib.h

- cmdUserStringParse.h

- DBG_macro.h_

##### FUNCTIONS USED:

- memcpy

- memset

----

### 注册命令`cmdTable_RegisterCMD`

```c
int cmdTable_RegisterCMD(void* cmd, int cmd_len,
    void* param, int param_len, ParameterHandler handler);
```

`cmd`: 命令字符串

`cmd_len`: 命令的字符串长度

`param`: 参数的字符串

`param_len`: 参数的字符串长度

`handler`: 处理函数

#### RETURN:

- OK: NODE_OK
- ERROR: others

##### note:

        区分大小写，无强制转换大小写

-----

### 更新命令`cmdTable_updataCMDarg`

```c
typedef struct {
    void* next;
    unsigned int command;
    unsigned int parameter;
    ParameterHandler handler;
} cmdHash_node;

int cmdTable_updataCMDarg(cmdHash_node* _old, cmdHash_node* _new);
```

`_old`: 旧参数（作为索引，命令与参数必填）

`_new`: 新参数（处理函数可填NULL，即为不修改处理函数，沿用旧处理函数）

#### RETURN:

- OK: NODE_OK
- ERROR: others

##### note:

        区分大小写，无强制转换大小写

-----

### 重置命令表 `cmdTable_resetTable`

```c
int cmdTable_resetTable(void);
```

#### RETURN:

- OK: NODE_OK
- ERROR: others

##### note:

-----

### 字符串转hash`cmdTable_CmdToHash`

```c
unsigned int cmdTable_CmdToHash(const char* string, int len);
```

`string`: 字符串

`len`: 字符串长度

#### RETURN:

- hash value

##### note:

        区分大小写，无强制转换大小写

-----

### 命令解析`cmdTable_CommandParse`

```c
int cmdTable_CommandParse(const char* commandString);
```

`commandString`: 命令字符串

#### RETURN:

- OK: NODE_OK
- ERROR: others

##### note:

        区分大小写，无强制转换大小写

-----

### 获得上次运行的错误`cmdTable_GetLastError`

```c
int cmdTable_GetLastError(void);
```

`commandString`: 命令字符串

#### RETURN:

- lastError

##### note:

        区分大小写，无强制转换大小写

-----