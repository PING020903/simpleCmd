# 简易终端

## 功能介绍

- 注册命令`RegisterCommand`

- 取消注册命令`unRegisterCommand`

- 取消注册全部命令 `unRegisterAllCommand`

- 寻找命令节点`FindCommand`

- 更新命令`updateCommand`

- 注册参数`RegisterParameter`

- 取消注册单个命令下的所有参数`unRegisterAllParameters`

- 取消注册参数`unRegisterParameter`

- 更新参数`updateParameter`

- 命令解析`CommandParse`

- 命令解析( 宽字符 )`CommandParseW`

- 获得上次运行链表管理函数的错误`NodeGetLastError`

- 打印当前已经注册的命令和参数`showList`

- 打印指定命令已经注册的参数`showParam`

------

## *TIPS*:

- 该终端使用`[command] [param] [userParam]`这样的形式，`userParam`是可选项

- 不能单独调用`command`，因为`command`没有处理函数可注册，被解析的指令必须带有`param`才能正常调用处理函数，

- 若解析的指令后面没有跟随`userParam`则直接调用处理函数，有`userParam`则会作为字符串参数传递给处理函数

- `command`为英文的情况下，将会被强制转换为小写，调试时无论输入大写还是小写均为同样的效果

- `command`首选推荐英文，中文( GB2312, UTF-8 )可能会出现未知情况，但并不是不行

- `param`是否支持宽字符，是继承`command`是否为宽字符这个属性，若后面带有`userParam`也应为宽字符格式

- `param`不支持输入字符`(space)`

---------

### *transplantation*：

##### INCLUDE:

- stdio.h
- string.h
- wchar.h
- stdlib.h
- stdbool.h

##### FUNCTIONS USED:

- malloc & free
- printf & wprintf
- strlen & wcslen
- strcpy & wcscpy
- strcmp & wcscmp
- memcpy

----

### 注册命令`RegisterCommand`

```c
int RegisterCommand(const bool isWch,
                    const char* command,
                    const wchar_t* commandW);
```

`isWch`: 是否使用宽字符支持

`command`: 非宽字符的命令

`commandW`: 宽字符的命令

#### RETURN:

- NODE_OK
- NODE_ARG_ERR
- NODE_CMD_TOO_LONG
- NODE_REPEATING
- NODE_FAIL

##### note:

        若输入英文命令，所有大写的字符都会被转换为小写

-----

### 取消注册命令`unRegisterCommand`

```c
int unRegisterCommand(char* command, wchar_t* commandW);
```

`command`: 非宽字符的命令

`commandW`: 宽字符的命令

#### RETURN:

* NODE_OK
* NODE_ARG_ERR
* NODE_CMD_TOO_LONG
* NODE_NOT_YET_INIT
* NODE_NOT_FIND_CMD
* NODE_FAIL

##### note:

        若输入英文命令，所有大写的字符都会被转换为小写

----

### 取消注册全部命令 `unRegisterAllCommand`

```c
int unRegisterAllCommand(void);
```

#### RETURN:

- NODE_OK
- NODE_FAIL
- NODE_NOT_YET_INIT
  
  -------

### 寻找命令节点`FindCommand`

```c
command_node* FindCommand(const char* command,
                          const wchar_t* commandW);
```

`command`: 非宽字符的命令

`commandW`: 宽字符的命令

#### RETURN:

- target node

- NULL
  
  ----

### 更新命令`updateCommand`

```c
int updateCommand(char* oldCommand, wchar_t* oldCommandW,
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

### 注册参数`RegisterParameter`

```c
int RegisterParameter(command_node* node,
                      ParameterHandler hook,
                      const char* param,
                      const wchar_t* paramW);
```

`node`: 注册参数的命令

`hook`: 参数的处理函数

`param`: 参数

`paramW`: 参数( 宽字符 )

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

### 取消注册单个命令下的所有参数`unRegisterAllParameters`

```c
int unRegisterAllParameters(command_node* node);
```



  `node`: 命令节点

#### RETURN:

- NODE_OK
- NODE_ARG_ERR
- NODE_PARAM_NODE_NULL
  
  ------

### 取消注册参数`unRegisterParameter`

```c
int unRegisterParameter(command_node* node,
                        const char* param,
                        const wchar_t* paramW);
```

  `node`: 要被删除参数的命令
  `para`: 参数
  `paraW`: 参数( 宽字符 )

#### RETURN:

- NODE_OK
- NODE_ARG_ERR
- NODE_NOT_FIND_PARAM
- NODE_PARAM_TOO_LONG
- NODE_FAIL
  
  --------

### 更新参数`updateParameter`

```c
int updateParameter(const command_node* CmdNode, ParameterHandler hook,
                    const char* oldParam, const wchar_t* oldParamW,
                    const char* newParam, const wchar_t* newParamW);
```

  `CmdNode`: 要被更改参数的命令
  `hook`: 参数处理函数
  `oldParam`: 旧参数
  `oldParamW`: 旧参数( 宽字符 )
  `newParam`: 新参数
  `newParamW`: 新参数( 宽字符 )

#### RETURN:

- NODE_OK
- NODE_ARG_ERR
- NODE_REPEATING
- NODE_NOT_FIND_PARAM
  
  --------

### 命令解析`CommandParse`

```c
int CommandParse(const char* commandString);
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

### 命令解析( 宽字符 )`CommandParseW`

```c
int CommandParseW(const wchar_t* commandString);
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

### 获得上次运行链表管理函数的错误`NodeGetLastError`

```c
int NodeGetLastError(void);
```

#### RETURN:

        lastError

--------

### 打印当前已经注册的命令和参数`showList`

```c
void showList(void);
```

----

### 打印指定命令已经注册的参数`showParam`

```c
void showParam(command_node* CmdNode);
```



----



### 作者吐槽

        在这里，我先 **DISS** 一下这篇文章`https://zhuanlan.zhihu.com/p/141409031`还有这份代码`https://github.com/jiejieTop/cmd-parser`。

        为什么 **DISS** 这份文章和代码呢？

        缘由是这样的，我需要个如同终端这样的字符解释器，根据我输入的字符来调用不同的函数，对板子进行调试，搜索了一些词条，但都是关于`Power Shell`亦或者是`Unix shell`的命令表格，跟我的需求都不搭边，唯一符合的就只有以上的这个知乎的文章链接。

        一开始我看见了这个符合我预期的东西还是挺开心的，但是点进*GitHub*仓库一看，好家伙，几个*issue*都没有处理，关键是这几个*issue*都是关于代码移植以后各种报错、跑不起来，还有几个*pull request*都没有处理，这完全是没人管的一个东西。再细看代码，发现在`line:67`处有这么一句，

```c
 printf("%s -->%s\n",index->cmd,index->desc);
```

而这个`index`的定义是这样的

```c
typedef void (*cmd_handler)(void);

typedef struct cmd {
    const char*     cmd;
    const char*     cmd_mess;
    unsigned int    hash;
    cmd_handler     handler;
} cmd_t;


static void _list(void)
{
    cmd_t *index;
    for (index = _cmd_begin; index < _cmd_end; index = _get_next_cmd(index)) {
        printf("%s -->%s\n",index->cmd,index->desc);
    }
}
```

于是我自己写了这个能满足我需求的终端。

2024.11.14
