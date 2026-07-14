#include <string.h>
#include <stdio.h>

#include "CommandParseTree.h"
#include "cmdUserStringParse.h"

/* ============================================================
 * FNV-1a hash（与旧 CommandParseTable 一致）
 *   FNV prime  = 0x01000193
 *   FNV offset = 0x811C9DC5
 * ============================================================ */
#define FNV_PRIME   0x01000193
#define FNV_OFFSET  0x811C9DC5

static unsigned int fnv_hash(const char *str, size_t len)
{
    unsigned int hash = FNV_OFFSET;
    for (size_t i = 0; i < len; i++) {
        hash ^= (unsigned char)str[i];
        hash *= FNV_PRIME;
    }
    return hash;
}

/* ============================================================
 * 静态模式实现
 * ============================================================ */
#if CMDTREE_MODE_STATIC

#include <string.h>

/* ====== 静态数组 ====== */
static cmdTreeNode_t  cmdTree_nodes[CMDTREE_STATIC_MAX_NODES];
static cmdTreeNodeRef activeNode   = CMDTREE_NULL;
static cmdTree_err_t   lastError   = CMDTREE_OK;

/* ====== 内部：获取根节点（索引 0，未初始化时返回 CMDTREE_NULL） ====== */
cmdTreeNodeRef cmdTree_getRoot(void)
{
    return (cmdTree_nodes[0].used) ? 0 : CMDTREE_NULL;
}

/* ====== 内部：在 parent 的子节点中查找 hash 匹配的节点 ====== */
static cmdTreeNodeRef find_child(cmdTreeNodeRef parent, unsigned int hash)
{
    for (cmdTreeNodeRef idx = cmdTree_nodes[parent].first_child;
         idx != CMDTREE_NULL;
         idx = cmdTree_nodes[idx].next_sibling) {
        if (cmdTree_nodes[idx].hash == hash)
            return idx;
    }
    return CMDTREE_NULL;
}

/* ==================================================================
 * 公开 API
 * ================================================================== */

#if CMDTREE_ENABLE_HELP
static void h_help(void *arg)
{
    (void)arg;
    cmdTree_showHelp();
}
#endif

void cmdTree_init(void)
{
    memset(cmdTree_nodes, 0, sizeof(cmdTree_nodes));

    /* 索引 0 = 根节点 */
    cmdTree_nodes[0].hash        = 0;
    cmdTree_nodes[0].handler     = NULL;
#if CMDTREE_ENABLE_DATA_HANDLER
    cmdTree_nodes[0].dataHandler = NULL;
#endif
    cmdTree_nodes[0].parent_idx  = -1;
    cmdTree_nodes[0].first_child = -1;
    cmdTree_nodes[0].next_sibling = -1;
    cmdTree_nodes[0].used        = 1;

    activeNode = CMDTREE_NULL;
    lastError  = CMDTREE_OK;
}

cmdTreeNodeRef cmdTree_Register(cmdTreeNodeRef parent,
                                const char *token,
                                handler_fn_t handler,
                                data_handler_fn_t dataHandler)
{
    /* 根节点未初始化（used == 0） → NOT_INIT */
    if (cmdTree_nodes[0].used == 0) {
        lastError = CMDTREE_ERR_NOT_INIT;
        return CMDTREE_NULL;
    }
    if (parent < 0 || parent >= CMDTREE_STATIC_MAX_NODES ||
        cmdTree_nodes[parent].used == 0) {
        lastError = CMDTREE_ERR_NULL_PARENT;
        return CMDTREE_NULL;
    }

    /* 查找空闲槽位（索引 1 开始，0 是根节点） */
    cmdTreeNodeRef slot = CMDTREE_NULL;
    for (cmdTreeNodeRef i = 1; i < CMDTREE_STATIC_MAX_NODES; i++) {
        if (cmdTree_nodes[i].used == 0) {
            slot = i;
            break;
        }
    }
    if (slot == CMDTREE_NULL) {
        lastError = CMDTREE_ERR_MEM;   /* 表满 */
        return CMDTREE_NULL;
    }

    /* 填充节点 */
    cmdTree_nodes[slot].hash         = fnv_hash(token, strlen(token));
    cmdTree_nodes[slot].handler      = handler;
#if CMDTREE_ENABLE_DATA_HANDLER
    cmdTree_nodes[slot].dataHandler  = dataHandler;
#else
    (void)dataHandler;
#endif
#if CMDTREE_ENABLE_HELP
    cmdTree_nodes[slot].token        = token;
#endif
    cmdTree_nodes[slot].parent_idx   = parent;
    cmdTree_nodes[slot].first_child  = -1;
    cmdTree_nodes[slot].next_sibling = -1;
    cmdTree_nodes[slot].used         = 1;

    /* 头插法链入 parent 的子节点链 */
    cmdTree_nodes[slot].next_sibling = cmdTree_nodes[parent].first_child;
    cmdTree_nodes[parent].first_child = slot;

    lastError = CMDTREE_OK;
    return slot;
}

cmdTreeNodeRef cmdTree_RegisterHelp(cmdTreeNodeRef parent)
{
#if CMDTREE_ENABLE_HELP
    return cmdTree_Register(parent, "help", h_help, NULL);
#else
    (void)parent;
    return CMDTREE_NULL;
#endif
}

int cmdTree_CommandParse(const char *commandString)
{
    if (cmdTree_nodes[0].used == 0) {
        lastError = CMDTREE_ERR_NOT_INIT;
        return lastError;
    }
    if (!commandString || commandString[0] == '\0') {
        lastError = CMDTREE_ERR_NOT_FOUND;
        return lastError;
    }

    /* 分词 */
    ParseSpace(commandString);
    int tokenCnt = userParse_GetUserParamCnt();
    userString *tokens = userParse_pUserData();

    cmdTreeNodeRef cur       = 0;   /* root */
    cmdTreeNodeRef bestMatch = CMDTREE_NULL;
    int bestDepth            = -1;

    /* 逐 token 沿树下行 */
    const int maxDepth = CMDTREE_STATIC_MAX_DEPTH;
    for (int i = 0; i < tokenCnt && i < maxDepth; i++) {
        unsigned int h = fnv_hash((const char *)tokens[i].strHead, tokens[i].len);
        cmdTreeNodeRef child = find_child(cur, h);
        if (child == CMDTREE_NULL)
            break;

        cur = child;
        if (cmdTree_nodes[cur].handler) {
            bestMatch = cur;
            bestDepth = i;
        }
    }

    /* 兜底：最后一个匹配节点有 handler */
    if (bestMatch == CMDTREE_NULL && cmdTree_nodes[cur].handler) {
        bestMatch = cur;
        if (cur != 0) {
            int d = -1;
            for (cmdTreeNodeRef p = cur; p != CMDTREE_NULL && p != 0;
                 p = cmdTree_nodes[p].parent_idx)
                d++;
            bestDepth = d;
        }
    }

    if (bestMatch == CMDTREE_NULL) {
#if CMDTREE_ENABLE_HELP
        printf("Unknown command. Type 'help' for available commands.\n");
#endif
        lastError = CMDTREE_ERR_NOT_FOUND;
    } else {
        activeNode = bestMatch;

        int remainingStart = bestDepth + 1;
        if (remainingStart < tokenCnt) {
            cmdTree_nodes[bestMatch].handler(&tokens[remainingStart]);
        } else {
            cmdTree_nodes[bestMatch].handler(NULL);
        }

        lastError = CMDTREE_OK;
    }

    RESET_USERDATA_RECORD();
    return lastError;
}

void cmdTree_reset(void)
{
    /* 全部清零（含根节点），cmdTree_getRoot() → CMDTREE_NULL */
    memset(cmdTree_nodes, 0, sizeof(cmdTree_nodes));
    activeNode = CMDTREE_NULL;
    lastError  = CMDTREE_OK;
}

cmdTree_err_t cmdTree_GetLastError(void)
{
    return lastError;
}

handler_fn_t cmdTree_getActiveHandler(void)
{
    return (activeNode != CMDTREE_NULL) ? cmdTree_nodes[activeNode].handler : NULL;
}

data_handler_fn_t cmdTree_getActiveDataHandler(void)
{
#if CMDTREE_ENABLE_DATA_HANDLER
    return (activeNode != CMDTREE_NULL) ? cmdTree_nodes[activeNode].dataHandler : NULL;
#else
    (void)activeNode;
    return NULL;
#endif
}

/* ====== 调试：打印树 ====== */
static void show_subtree_static(cmdTreeNodeRef node, int depth)
{
    for (int i = 0; i < depth; i++)
        printf("  ");
    printf("[%08x]", cmdTree_nodes[node].hash);
    if (cmdTree_nodes[node].handler)
        printf(" H");
#if CMDTREE_ENABLE_DATA_HANDLER
    if (cmdTree_nodes[node].dataHandler)
        printf(" D");
#endif
    printf("\n");

    for (cmdTreeNodeRef c = cmdTree_nodes[node].first_child;
         c != CMDTREE_NULL;
         c = cmdTree_nodes[c].next_sibling) {
        show_subtree_static(c, depth + 1);
    }
}

void cmdTree_show(void)
{
    if (cmdTree_nodes[0].used == 0) {
        printf("(tree not initialized)\n");
        return;
    }
    printf("cmdTree (static, max=%d nodes):\n", CMDTREE_STATIC_MAX_NODES);
    show_subtree_static(0, 0);
}

/* ====== 帮助：递归显示命令层级 ====== */
#if CMDTREE_ENABLE_HELP
static void showHelp_subtree_static(cmdTreeNodeRef node, int depth)
{
    for (cmdTreeNodeRef c = cmdTree_nodes[node].first_child;
         c != CMDTREE_NULL;
         c = cmdTree_nodes[c].next_sibling) {
        for (int i = 0; i < depth; i++)
            printf("  ");
        printf("%s", cmdTree_nodes[c].token ? cmdTree_nodes[c].token : "(null)");
        if (cmdTree_nodes[c].handler)
            printf("  <- [cmd]");
        printf("\n");

        showHelp_subtree_static(c, depth + 1);
    }
}
#endif

void cmdTree_showHelp(void)
{
#if CMDTREE_ENABLE_HELP
    if (cmdTree_nodes[0].used == 0) {
        printf("(tree not initialized)\n");
        return;
    }
    printf("Available commands:\n");
    showHelp_subtree_static(0, 0);
#else
    printf("help is disabled (CMDTREE_ENABLE_HELP=0)\n");
#endif
}

#endif /* CMDTREE_MODE_STATIC */

/* ============================================================
 * 动态模式实现
 * ============================================================ */
#if CMDTREE_MODE_DYNAMIC

/* ====== 内部状态 ====== */
static cmdTreeNode_t *cmdTree_root = NULL;       // 根节点
static cmdTreeNode_t *activeNode  = NULL;       // 本次解析命中的节点（BLE 绑定用）
static cmdTree_err_t  lastError   = CMDTREE_OK;

/* ====== 内部：获取根节点 ====== */
cmdTreeNodeRef cmdTree_getRoot(void)
{
    return cmdTree_root;
}

/* ====== 内部：在 parent 的子节点中查找 hash 匹配的节点 ====== */
static cmdTreeNode_t *find_child(cmdTreeNode_t *parent, unsigned int hash)
{
    cmdTreeNode_t *child;
    list_for_each_entry(child, &parent->children_head, sibling_node) {
        if (child->hash == hash)
            return child;
    }
    return NULL;
}

/* ====== 内部：递归释放子树（自底向上） ====== */
static void free_subtree(cmdTreeNode_t *node)
{
    if (!node) return;

    while (node->children_head.next != &node->children_head) {
        cmdTreeNode_t *child = list_first_entry(
            &node->children_head, cmdTreeNode_t, sibling_node);
        list_del(&child->sibling_node);
        free_subtree(child);
        cmd_MemoryFree(child);
    }
}

/* ==================================================================
 * 公开 API
 * ================================================================== */

#if CMDTREE_ENABLE_HELP
static void h_help(void *arg)
{
    (void)arg;
    cmdTree_showHelp();
}
#endif

void cmdTree_init(void)
{
    if (cmdTree_root) return;

    cmdTree_root = (cmdTreeNode_t *)cmd_MemoryAlloc(sizeof(cmdTreeNode_t));
    if (!cmdTree_root) {
        lastError = CMDTREE_ERR_MEM;
        return;
    }

    cmdTree_root->hash        = 0;
    cmdTree_root->handler     = NULL;
#if CMDTREE_ENABLE_DATA_HANDLER
    cmdTree_root->dataHandler = NULL;
#endif
    cmdTree_root->parent      = NULL;
    cmdTree_root->children_head = (ll_t)ll_head_INIT(cmdTree_root->children_head);

    activeNode = NULL;
    lastError  = CMDTREE_OK;
}

cmdTreeNodeRef cmdTree_Register(cmdTreeNodeRef parent,
                                const char *token,
                                handler_fn_t handler,
                                data_handler_fn_t dataHandler)
{
    if (!cmdTree_root) {
        lastError = CMDTREE_ERR_NOT_INIT;
        return CMDTREE_NULL;
    }
    if (!parent) {
        lastError = CMDTREE_ERR_NULL_PARENT;
        return CMDTREE_NULL;
    }

    cmdTreeNode_t *n = (cmdTreeNode_t *)cmd_MemoryAlloc(sizeof(cmdTreeNode_t));
    if (!n) {
        lastError = CMDTREE_ERR_MEM;
        return CMDTREE_NULL;
    }

    n->hash        = fnv_hash(token, strlen(token));
    n->handler     = handler;
#if CMDTREE_ENABLE_DATA_HANDLER
    n->dataHandler = dataHandler;
#else
    (void)dataHandler;
#endif
#if CMDTREE_ENABLE_HELP
    n->token       = token;
#endif
    n->parent      = parent;
    n->children_head = (ll_t)ll_head_INIT(n->children_head);

    list_add_tail(&n->sibling_node, &parent->children_head);

    lastError = CMDTREE_OK;
    return n;
}

cmdTreeNodeRef cmdTree_RegisterHelp(cmdTreeNodeRef parent)
{
#if CMDTREE_ENABLE_HELP
    return cmdTree_Register(parent, "help", h_help, NULL);
#else
    (void)parent;
    return CMDTREE_NULL;
#endif
}

int cmdTree_CommandParse(const char *commandString)
{
    if (!cmdTree_root) {
        lastError = CMDTREE_ERR_NOT_INIT;
        return lastError;
    }
    if (!commandString || commandString[0] == '\0') {
        lastError = CMDTREE_ERR_NOT_FOUND;
        return lastError;
    }

    ParseSpace(commandString);
    int tokenCnt = userParse_GetUserParamCnt();
    userString *tokens = userParse_pUserData();

    cmdTreeNode_t *cur       = cmdTree_root;
    cmdTreeNode_t *bestMatch = NULL;
    int bestDepth            = -1;

    const int maxDepth = tokenCnt;
    for (int i = 0; i < tokenCnt && i < maxDepth; i++) {
        unsigned int h = fnv_hash((const char *)tokens[i].strHead, tokens[i].len);
        cmdTreeNode_t *child = find_child(cur, h);
        if (!child)
            break;

        cur = child;
        if (cur->handler) {
            bestMatch = cur;
            bestDepth = i;
        }
    }

    if (bestMatch == NULL && cur->handler) {
        bestMatch = cur;
        if (cur != cmdTree_root) {
            int d = -1;
            for (cmdTreeNode_t *p = cur; p && p != cmdTree_root; p = p->parent)
                d++;
            bestDepth = d;
        }
    }

    if (bestMatch == NULL) {
#if CMDTREE_ENABLE_HELP
        printf("Unknown command. Type 'help' for available commands.\n");
#endif
        lastError = CMDTREE_ERR_NOT_FOUND;
    } else {
        activeNode = bestMatch;

        int remainingStart = bestDepth + 1;
        if (remainingStart < tokenCnt) {
            bestMatch->handler(&tokens[remainingStart]);
        } else {
            bestMatch->handler(NULL);
        }

        lastError = CMDTREE_OK;
    }

    RESET_USERDATA_RECORD();
    return lastError;
}

void cmdTree_reset(void)
{
    if (!cmdTree_root) return;

    free_subtree(cmdTree_root);
    cmd_MemoryFree(cmdTree_root);
    cmdTree_root = NULL;
    activeNode   = NULL;
    lastError    = CMDTREE_OK;
}

cmdTree_err_t cmdTree_GetLastError(void)
{
    return lastError;
}

handler_fn_t cmdTree_getActiveHandler(void)
{
    return activeNode ? activeNode->handler : NULL;
}

data_handler_fn_t cmdTree_getActiveDataHandler(void)
{
#if CMDTREE_ENABLE_DATA_HANDLER
    return activeNode ? activeNode->dataHandler : NULL;
#else
    (void)activeNode;
    return NULL;
#endif
}

/* ====== 调试：打印树 ====== */
static void show_subtree(const cmdTreeNode_t *node, int depth)
{
    for (int i = 0; i < depth; i++)
        printf("  ");
    printf("[%08x]", node->hash);
    if (node->handler)
        printf(" H");
#if CMDTREE_ENABLE_DATA_HANDLER
    if (node->dataHandler)
        printf(" D");
#endif
    printf("\n");

    cmdTreeNode_t *child;
    list_for_each_entry(child, &node->children_head, sibling_node) {
        show_subtree(child, depth + 1);
    }
}

void cmdTree_show(void)
{
    if (!cmdTree_root) {
        printf("(tree not initialized)\n");
        return;
    }
    printf("cmdTree (dynamic):\n");
    show_subtree(cmdTree_root, 0);
}

/* ====== 帮助：递归显示命令层级 ====== */
#if CMDTREE_ENABLE_HELP
static void showHelp_subtree(const cmdTreeNode_t *node, int depth)
{
    cmdTreeNode_t *child;
    list_for_each_entry(child, &node->children_head, sibling_node) {
        for (int i = 0; i < depth; i++)
            printf("  ");
        printf("%s", child->token ? child->token : "(null)");
        if (child->handler)
            printf("  <- [cmd]");
        printf("\n");

        showHelp_subtree(child, depth + 1);
    }
}
#endif

void cmdTree_showHelp(void)
{
#if CMDTREE_ENABLE_HELP
    if (!cmdTree_root) {
        printf("(tree not initialized)\n");
        return;
    }
    printf("Available commands:\n");
    showHelp_subtree(cmdTree_root, 0);
#else
    printf("help is disabled (CMDTREE_ENABLE_HELP=0)\n");
#endif
}

#endif /* CMDTREE_MODE_DYNAMIC */
