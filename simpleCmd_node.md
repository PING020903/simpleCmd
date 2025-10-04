# 简易终端-节点

## 功能介绍

- 注册命令`cmdNode_RegisterCommand`

- 取消注册命令`cmdNode_unRegisterCommand`

- 取消注册全部命令 `cmdNode_unRegisterAllCommand`

- 寻找命令节点`cmdNode_FindCommand`

- 更新命令`cmdNode_updateCommand`

- 注册参数`cmdNode_RegisterParameter`

- 取消注册单个命令下的所有参数`cmdNode_unRegisterAllParameters`

- 取消注册参数`cmdNode_unRegisterParameter`

- 更新参数`cmdNode_updateParameter`

- 命令解析`cmdNode_CommandParse`

- 命令解析( 宽字符 )`cmdNode_CommandParseW`

- 获得上次运行链表管理函数的错误`cmdNode_GetLastError`

- 打印当前已经注册的命令和参数`cmdNode_showList`

- 打印指定命令已经注册的参数`cmdNode_showParam`

------

## *TIPS*:

- 该终端使用`[command] [param] [userParam]...`这样的形式，`userParam`是可选项

- 不能单独调用`command`，因为`command`没有处理函数可注册，被解析的指令必须带有`param`才能正常调用处理函数，

- 若解析的指令后面没有跟随`userParam`则直接调用处理函数，有`userParam`则会作为字符串参数传递给处理函数

- `command`为英文的情况下，将会被强制转换为小写，调试时无论输入大写还是小写均为同样的效果

- `command`首选推荐英文，中文( GB2312, UTF-8 )可能会出现未知情况，但并不是不行

- `param`是否支持宽字符，是继承`command`是否为宽字符这个属性，若后面带有`userParam`也应为宽字符格式

- `param`不支持输入字符`(space)`

- 若传入到用户注册函数的是源字符串，请用户不要妄图修改源字符串，因为它实际上是`const`属性

---------

### *transplantation*：

##### INCLUDE:

- stdio.h

- string.h

- wchar.h

- stdlib.h

- cmdUserStringParse.h

##### FUNCTIONS USED:

- malloc & free

- printf & wprintf

- strlen & wcslen

- strcpy & wcscpy

- strcmp & wcscmp

- memcpy

----

### 注册命令`cmdNode_RegisterCommand`

```c
int cmdNode_RegisterCommand(const bool isWch, const void* cmdStr);
```

`isWch`: 是否使用宽字符支持

`cmdStr`: 命令的字符串

#### RETURN:

- NODE_OK
- NODE_ARG_ERR
- NODE_CMD_TOO_LONG
- NODE_REPEATING
- NODE_FAIL

##### note:

        若输入英文命令，所有大写的字符都会被转换为小写

-----

### 取消注册命令`cmdNode_unRegisterCommand`

```c
int cmdNode_unRegisterCommand(const char* command, const wchar_t* commandW);
```

`command`: 非宽字符的命令

`commandW`: 宽字符的命令

#### RETURN:

- NODE_OK
- NODE_ARG_ERR
- NODE_CMD_TOO_LONG
- NODE_NOT_YET_INIT
- NODE_NOT_FIND_CMD
- NODE_FAIL

##### note:

        若输入英文命令，所有大写的字符都会被转换为小写

----

### 取消注册全部命令 `cmdNode_unRegisterAllCommand`

```c
int cmdNode_unRegisterAllCommand(void);
```

#### RETURN:

- NODE_OK
- NODE_FAIL
- NODE_NOT_YET_INIT
  
  -------

### 寻找命令节点`cmdNode_FindCommand`

```c
command_node* cmdNode_FindCommand(const char* command, const wchar_t* commandW);
```

`command`: 非宽字符的命令

`commandW`: 宽字符的命令

#### RETURN:

- target node
- NULL
  
  ----

### 更新命令`cmdNode_updateCommand`

```c
int cmdNode_updateCommand(char* oldCommand, wchar_t* oldCommandW,
    char* newCommand, wchar_t* newCommandW);
```

`oldCommand`: 旧命令

`oldCommandW`: 旧命令( 宽字符 )

`newCommand`: 新命令

`newCommandW`: 新命令( 宽字符 )

#### RETURN:

- NODE_OK
- NODE_ARG_ERR
- NODE_NOT_FIND_CMD
- NODE_REPEATING
- NODE_NOT_YET_INIT
- NODE_FAIL

##### note:

        若输入英文命令，所有大写的字符都会被转换为小写

----

### 注册参数`cmdNode_RegisterParameter`

```c
int cmdNode_RegisterParameter(command_node* node,
    ParameterHandler hook,
    const bool isRaw,
    const void* paramStr);
```

`node`: 注册参数的命令

`hook`: 参数的处理函数

`isRaw`: 是否给 `hook` 传递源字符串

`param`: 参数

#### RETURN:

- NODE_OK
- NODE_ARG_ERR
- NODE_CMD_NODE_NULL
- NODE_PARAM_TOO_LONG
- NODE_REPEATING
- NODE_FAIL

##### note:

          相较于`command`，`parameter`的定义就比较自由，除了不能输入字符`(space)`

  --------

### 取消注册单个命令下的所有参数`cmdNode_unRegisterAllParameters`

```c
int cmdNode_unRegisterAllParameters(command_node* node);
```



  `node`: 命令节点

#### RETURN:

- NODE_OK
- NODE_ARG_ERR
- NODE_PARAM_NODE_NULL
  
  ------

### 取消注册参数`cmdNode_unRegisterParameter`

```c
int cmdNode_unRegisterParameter(command_node* node,
    const void* paramStr);
```

  `node`: 要被删除参数的命令
  `paraStr`: 参数

#### RETURN:

- NODE_OK
- NODE_ARG_ERR
- NODE_NOT_FIND_PARAM
- NODE_PARAM_TOO_LONG
- NODE_FAIL
  
  --------

### 更新参数`cmdNode_updateParameter`

```c
int cmdNode_updateParameter(const command_node* CmdNode, ParameterHandler hook,
    const bool isRaw, const void* oldParam, const void* newParam);
```

  `CmdNode`: 要被更改参数的命令
  `hook`: 参数处理函数
  `isRaw`: 是否给 `hook` 传递源字符串
  `oldParam`: 旧参数( 宽字符 )
  `newParam`: 新参数

#### RETURN:

- NODE_OK
- NODE_ARG_ERR
- NODE_REPEATING
- NODE_NOT_FIND_PARAM
  
  --------

### 命令解析`cmdNode_CommandParse`

```c
int cmdNode_CommandParse(const char* commandString);
```



  `commandString`: 用户输入的字符串

#### RETURN:

- NODE_OK
- NODE_ARG_ERR
- NODE_CMD_TOO_LONG
- NODE_PARAM_TOO_LONG
- NODE_NOT_FIND_CMD
- NODE_NOT_FIND_PARAM
- NODE_PARSE_ERR
- NODE_NOT_YET_INIT
  
  

##### note:

        `command`和`parameter`直接的空格都会被忽略，`parameter`和`userParam`之间也是如此

----

### 命令解析( 宽字符 )`cmdNode_CommandParseW`

```c
int cmdNode_CommandParseW(const wchar_t* commandString);
```



  `commandString`: 用户输入的字符串

#### RETURN:

- NODE_OK
- NODE_ARG_ERR
- NODE_CMD_TOO_LONG
- NODE_PARAM_TOO_LONG
- NODE_NOT_FIND_CMD
- NODE_NOT_FIND_PARAM
- NODE_PARSE_ERR
- NODE_NOT_YET_INIT
  
  

##### note:

        `command`和`parameter`直接的空格都会被忽略，`parameter`和`userParam`之间也是如此

----

### 获得上次运行链表管理函数的错误`cmdNode_GetLastError`

```c
int cmdNode_GetLastError(void);
```

#### RETURN:

        lastError

--------

### 打印当前已经注册的命令和参数`cmdNode_showList`

```c
void cmdNode_showList(void);
```

----

### 打印指定命令已经注册的参数`cmdNode_showParam`

```c
void cmdNode_showParam(command_node* CmdNode);
```



----



