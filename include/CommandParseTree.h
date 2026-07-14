#pragma once
#ifndef _COMMANDPARSETREE_H_
#define _COMMANDPARSETREE_H_

#include <stdint.h>
#include <stddef.h>
#include "cmdTreeCfg.h"

#if CMDTREE_MODE_DYNAMIC
#include "ll.h"
#endif

/* ============================================================
 * cmdTree — 树形命令解析器 v2.0
 *
 * 两种模式：
 *   CMDTREE_MODE_DYNAMIC=1 → 动态分配（本文件实现）
 *   CMDTREE_MODE_STATIC=1  → 静态数组（待实现）
 *
 * 两种模式共享完全相同的 API，调用方无感。
 * ============================================================ */

/* ====== 错误码 ====== */
typedef enum {
    CMDTREE_OK              =  0,
    CMDTREE_ERR_NOT_FOUND   = -1,   // 未匹配到任何 handler
    CMDTREE_ERR_NULL_PARENT = -2,   // parent 为 NULL
    CMDTREE_ERR_MEM         = -3,   // 内存分配失败
    CMDTREE_ERR_NOT_INIT    = -4,   // 未初始化
    CMDTREE_ERR_BUSY        = -5,   // 上次解析未释放
} cmdTree_err_t;

/* ====== handler 类型 ====== */

/** 命令 handler：arg 为 userString 数组指针（剩余参数），NULL 表示无参数 */
typedef void (*handler_fn_t)(void *arg);

/** BLE 数据回调：pBuff=接收到的数据, len=数据长度, 返回 0=成功 */
#if CMDTREE_ENABLE_DATA_HANDLER
typedef int (*data_handler_fn_t)(const void *pBuff, const int len);
#else
typedef void* data_handler_fn_t;   /* 禁用时退化为 void*，保持 API 签名兼容 */
#endif

/* ============================================================
 * 动态模式：外部内存分配 + c-linked-list
 * ============================================================ */
#if CMDTREE_MODE_DYNAMIC

/** 树节点 */
typedef struct cmdTreeNode {
    unsigned int            hash;           // 当前 token 的 FNV hash
    handler_fn_t            handler;        // NULL = 纯路由节点
#if CMDTREE_ENABLE_DATA_HANDLER
    data_handler_fn_t       dataHandler;    // NULL = 无 BLE 数据回调
#endif
    struct cmdTreeNode     *parent;         // 父节点（root->parent = NULL）

    ll_t                    sibling_node;   // 挂在 parent->children_head 链表上
    ll_t                    children_head;  // 本节点的子节点链表头
} cmdTreeNode_t;

/** 节点引用 = 指针 */
typedef cmdTreeNode_t *cmdTreeNodeRef;

#define CMDTREE_NULL  NULL
#define CMDTREE_ROOT  (cmdTree_getRoot())

#endif /* CMDTREE_MODE_DYNAMIC */

/* ============================================================
 * 静态模式：编译期确定大小的数组 + 索引引用
 * ============================================================ */
#if CMDTREE_MODE_STATIC

/** 树节点（数组元素） */
typedef struct cmdTreeNode {
    unsigned int        hash;           // 当前 token 的 FNV hash
    handler_fn_t        handler;        // NULL = 纯路由节点
#if CMDTREE_ENABLE_DATA_HANDLER
    data_handler_fn_t   dataHandler;    // NULL = 无 BLE 数据回调
#endif
    CMDTREE_STATIC_INDEX_TYPE  parent_idx;     // 父节点索引（根节点的 parent_idx = -1）
    CMDTREE_STATIC_INDEX_TYPE  first_child;    // 第一个子节点索引（-1 = 无子节点）
    CMDTREE_STATIC_INDEX_TYPE  next_sibling;   // 下一个兄弟节点索引（-1 = 无）
    unsigned char       used;           // 0 = 空闲槽位, 1 = 已占用
} cmdTreeNode_t;

/** 节点引用 = 数组索引 */
typedef CMDTREE_STATIC_INDEX_TYPE cmdTreeNodeRef;

#define CMDTREE_NULL  ((cmdTreeNodeRef)(-1))
#define CMDTREE_ROOT  (cmdTree_getRoot())   /* 始终为 0 */

#endif /* CMDTREE_MODE_STATIC */

/* ====== 统一 API ====== */

/** 初始化树（动态模式：创建根节点；静态模式：初始化数组） */
void cmdTree_init(void);

/**
 * @brief 注册一个命令节点
 * @param parent      父节点引用（根节点用 CMDTREE_ROOT）
 * @param token       当前层级的命令字符串（如 "device"、"reset"）
 * @param handler     命中后回调（NULL = 纯路由节点）
 * @param dataHandler BLE 数据回调（NULL = 无）
 * @return 新节点引用，失败返回 CMDTREE_NULL
 */
cmdTreeNodeRef cmdTree_Register(cmdTreeNodeRef parent,
                                const char *token,
                                handler_fn_t handler,
                                data_handler_fn_t dataHandler);

/**
 * @brief 解析命令字符串
 *
 * 逐 token 沿树下行，取最深命中 handler 节点，剩余参数透传给 handler。
 * 同时自动记录 dataHandler 供后续 BLE 数据回调使用。
 *
 * @param commandString  输入命令字符串（如 "device reset"、"wait dat font"）
 * @return CMDTREE_OK 或错误码
 */
int cmdTree_CommandParse(const char *commandString);

/** 递归释放所有节点 */
void cmdTree_reset(void);

/** 获取最近一次错误码 */
cmdTree_err_t cmdTree_GetLastError(void);

/** 调试：打印树结构 */
void cmdTree_show(void);

/** 获取当前激活的 handler（解析后由 tree 自动设置） */
handler_fn_t cmdTree_getActiveHandler(void);

/** 获取当前激活的 BLE dataHandler（解析后由 tree 自动设置） */
data_handler_fn_t cmdTree_getActiveDataHandler(void);

/** 内部：获取根节点指针（供 CMDTREE_ROOT 宏使用） */
cmdTreeNodeRef cmdTree_getRoot(void);

#endif /* _COMMANDPARSETREE_H_ */
