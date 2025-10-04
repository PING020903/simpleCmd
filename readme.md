### 作者吐槽1

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

-----

创建于`2024-11-11 20:44`

修改时间`2024-11-18 6:37`

-----


### 作者吐槽2

        原本在`ESP32`上使用这个链表命令解析器用得还挺好的，直到更换了`SRAM`更低的MCU，比如我最近用的这个`WCH-571F`，只要一旦启用了`BLE`功能，动不动就会出现堆栈踩踏的现象，导致原本申请的堆空间被破坏导致程序跑飞了。

        无奈之下只好再搞个`HASH`版本的命令解析器，解决了字符串占用大量堆空间的问题，也简化了节点的结构复杂度，进一步减少对`SRAM`的占用。

        后面发现`WCH-571F`的`SRAM`还是太有限(18KB)了，想要遍历BLE服务都做不了，而且即便我用了这个版本的命令解析，依旧会有部分命令被覆盖，又转而用`WCH-582M`(32KB)才缓解了这个问题。

修改时间`2025-10-02 18:34`

-----
