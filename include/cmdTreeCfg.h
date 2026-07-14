#pragma once
#ifndef _CMDTREECFG_H_
#define _CMDTREECFG_H_

/* ============================================================
 * cmdTree 功能配置头文件
 *
 * 通过修改此文件中的宏来启用/禁用各项功能。
 * 所有模块（分词器、树解析器）均通过 include 此文件获取配置。
 * ============================================================ */

/* ====== 模式选择（二选一） ====== */
#define CMDTREE_MODE_STATIC   1   // 静态数组模式（嵌入式 MCU，编译期确定内存）
#define CMDTREE_MODE_DYNAMIC  0   // 动态分配模式（桌面 / 资源充足，运行时分配）

#if CMDTREE_MODE_STATIC && CMDTREE_MODE_DYNAMIC
#error "CMDTREE_MODE_STATIC 和 CMDTREE_MODE_DYNAMIC 不能同时为 1"
#endif
#if !CMDTREE_MODE_STATIC && !CMDTREE_MODE_DYNAMIC
#error "必须启用 CMDTREE_MODE_STATIC 或 CMDTREE_MODE_DYNAMIC 之一"
#endif

/* ====== 静态模式参数 ====== */
#if CMDTREE_MODE_STATIC
#define CMDTREE_STATIC_INDEX_TYPE  short   // 节点索引类型（须有符号：short / int / int8_t 等），影响节点上限和内存
#define CMDTREE_STATIC_MAX_NODES   32     // 最大节点数（含根节点），超出则注册失败
#define CMDTREE_STATIC_MAX_DEPTH   12     // 最大路由深度（token 级数），编译期硬限制

/* C99 编译期校验：idx_t 必须是有符号类型（C99 兼容：利用负数组长度） */
typedef char CMDTREE_assert_idx_signed[(CMDTREE_STATIC_INDEX_TYPE)(-1) < (CMDTREE_STATIC_INDEX_TYPE)0 ? 1 : -1];
#endif

/* ====== 宽字符支持（桌面调试用） ====== */
#define ENABLE_WCHAR  1   // 1=启用 ParseSpaceW 宽字符分词，0=禁用（节省嵌入式空间）

/* ====== BLE 数据回调 ====== */
#define CMDTREE_ENABLE_DATA_HANDLER  1   // 1=启用 dataHandler（BLE 数据回调），0=禁用（节省节点内存）

/* ====== 内置 help 指令 ====== */
#define CMDTREE_ENABLE_HELP  1   // 1=启用内置 "help" 指令，0=禁用（节省节点内存）
// 注意：token 字符串直接存 const char* 指针，不做拷贝。
// 这意味着 help 指令只能正确显示用编译期字面量（如 "device"）注册的命令，
// 运行时在栈上构造的临时字符串（如 snprintf 拼出的 buffer）无法通过 help 显示。
// 调用方传入 cmdTree_Register 的 token 必须是编译期字面量或全局持久字符串。

/* ====== 输入缓冲区大小 ====== */
#define PARSE_SIZE  128   // 单次解析的输入最大字符数

#endif // !_CMDTREECFG_H_
