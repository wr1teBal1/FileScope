#ifndef PATH_RESOLVER_H
#define PATH_RESOLVER_H

#include "main.h"
#include <stdbool.h>
#include <stddef.h>

// 路径解析错误码
typedef enum {
    PATH_ERROR_NONE,
    PATH_ERROR_INVALID_CHARACTERS,
    PATH_ERROR_TOO_LONG,
    PATH_ERROR_INVALID_FORMAT,
    PATH_ERROR_ENVIRONMENT_VARIABLE_NOT_FOUND,
    PATH_ERROR_SHORTCUT_RESOLUTION_FAILED,
    PATH_ERROR_UNKNOWN
} PathError;

// 路径类型
typedef enum {
    PATH_TYPE_ABSOLUTE,      // 绝对路径
    PATH_TYPE_RELATIVE,      // 相对路径
    PATH_TYPE_ENVIRONMENT,   // 包含环境变量
    PATH_TYPE_SHORTCUT,      // 快捷方式
    PATH_TYPE_VIRTUAL        // 虚拟路径（如桌面、我的电脑）
} PathType;

// 路径信息结构
typedef struct {
    char *normalized_path;   // 规范化后的路径
    char *absolute_path;     // 绝对路径
    char *relative_path;     // 相对路径
    PathType type;           // 路径类型
    bool is_valid;           // 是否有效
    PathError error;         // 错误码
} PathInfo;

// 路径解析配置
typedef struct {
    bool case_sensitive;     // 是否区分大小写（Windows通常为false）
    size_t max_path_length;  // 最大路径长度
    bool resolve_shortcuts;  // 是否解析快捷方式
    bool expand_environment; // 是否展开环境变量
    bool resolve_virtual;    // 是否解析虚拟路径
} PathResolverConfig;

// 初始化路径解析器
bool path_resolver_init(const PathResolverConfig *config);

// 清理路径解析器
void path_resolver_cleanup(void);

// 规范化路径
// 将不同格式的路径统一为标准格式
char* path_normalize(const char *path);

// 将相对路径转换为绝对路径
char* path_to_absolute(const char *path, const char *base_path);

// 验证路径合法性
bool path_validate(const char *path, PathError *error);

// 解析路径（综合处理）
PathInfo* path_resolve(const char *path, const char *base_path);

// 释放路径信息
void path_info_free(PathInfo *info);

// 展开环境变量
char* path_expand_environment(const char *path);

// 解析快捷方式
char* path_resolve_shortcut(const char *shortcut_path);

// 解析虚拟路径
char* path_resolve_virtual(const char *virtual_path);

// 获取路径类型
PathType path_get_type(const char *path);

// 检查路径是否包含非法字符
bool path_has_invalid_characters(const char *path);

// 检查路径长度是否超出限制
bool path_is_too_long(const char *path);

// 获取路径分隔符
char path_get_separator(void);

// 组合路径
char* path_combine(const char *path1, const char *path2);

// 获取路径的目录部分
char* path_get_directory(const char *path);

// 获取路径的文件名部分
char* path_get_filename(const char *path);

// 获取路径的扩展名
char* path_get_extension(const char *path);

// 获取路径的基础名（不含扩展名）
char* path_get_basename(const char *path);

// 获取错误描述
const char* path_get_error_string(PathError error);

// 获取默认配置
PathResolverConfig path_get_default_config(void);

#endif // PATH_RESOLVER_H 