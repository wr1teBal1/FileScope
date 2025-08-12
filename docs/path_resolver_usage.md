# 路径解析模块使用说明

## 概述

路径解析模块是 FileScope 文件管理器的核心组件，负责处理各种格式的路径输入，确保文件系统操作的一致性和可靠性。

## 主要功能

### 1. 路径规范化
将不同格式的路径统一为标准格式，处理多余的斜杠、大小写转换等。

**示例：**
```c
// 输入: "C:\\Dir\\Sub\\\\File"
// 输出: "c:\dir\sub\file" (Windows, 不区分大小写)

char *normalized = path_normalize("C:\\Dir\\Sub\\\\File");
printf("规范化路径: %s\n", normalized);
free(normalized);
```

### 2. 相对路径转绝对路径
将相对路径转换为绝对路径，结合当前工作目录进行解析。

**示例：**
```c
// 当前目录: C:\Users\Name
// 输入: ".\Downloads\file.txt"
// 输出: "c:\users\name\downloads\file.txt"

char *absolute = path_to_absolute(".\\Downloads\\file.txt", NULL);
printf("绝对路径: %s\n", absolute);
free(absolute);
```

### 3. 路径合法性验证
检查路径是否包含非法字符，验证路径长度是否超出系统限制。

**示例：**
```c
PathError error;
bool is_valid = path_validate("C:\\Test\\file.txt", &error);
if (!is_valid) {
    printf("路径无效: %s\n", path_get_error_string(error));
}
```

### 4. 特殊路径处理
- **环境变量展开**: `%USERPROFILE%\Documents` → `C:\Users\Name\Documents`
- **快捷方式解析**: 解析 `.lnk` 文件指向的实际路径
- **虚拟路径解析**: `桌面\文件.txt` → `C:\Users\Name\Desktop\文件.txt`

## 基本使用流程

### 1. 初始化模块

```c
#include "path_resolver.h"

// 使用默认配置初始化
if (!path_resolver_init(NULL)) {
    printf("路径解析器初始化失败\n");
    return -1;
}

// 或使用自定义配置
PathResolverConfig config = {
    .case_sensitive = false,
    .max_path_length = 260,
    .resolve_shortcuts = true,
    .expand_environment = true,
    .resolve_virtual = true
};
path_resolver_init(&config);
```

### 2. 解析路径

```c
// 综合路径解析
PathInfo *info = path_resolve("桌面\\文档.txt", "C:\\Users\\Name");
if (info && info->is_valid) {
    printf("规范化路径: %s\n", info->normalized_path);
    printf("绝对路径: %s\n", info->absolute_path);
    printf("相对路径: %s\n", info->relative_path);
    printf("路径类型: %d\n", info->type);
} else {
    printf("路径解析失败: %s\n", path_get_error_string(info->error));
}

// 释放资源
path_info_free(info);
```

### 3. 清理资源

```c
// 程序结束时清理
path_resolver_cleanup();
```

## 高级功能

### 环境变量处理

```c
// 展开环境变量
char *expanded = path_expand_environment("%USERPROFILE%\\Downloads");
printf("展开后: %s\n", expanded);
free(expanded);
```

### 快捷方式解析

```c
// 解析快捷方式
char *target = path_resolve_shortcut("C:\\Users\\Name\\Desktop\\快捷方式.lnk");
printf("目标路径: %s\n", target);
free(target);
```

### 虚拟路径解析

```c
// 解析虚拟路径
char *real_path = path_resolve_virtual("桌面\\文件.txt");
printf("真实路径: %s\n", real_path);
free(real_path);
```

## 路径操作工具函数

### 路径分解

```c
char *path = "C:\\Users\\Name\\Documents\\file.txt";

// 获取目录部分
char *dir = path_get_directory(path);
printf("目录: %s\n", dir);  // 输出: C:\Users\Name\Documents
free(dir);

// 获取文件名
char *filename = path_get_filename(path);
printf("文件名: %s\n", filename);  // 输出: file.txt
free(filename);

// 获取扩展名
char *ext = path_get_extension(path);
printf("扩展名: %s\n", ext);  // 输出: .txt
free(ext);

// 获取基础名（不含扩展名）
char *basename = path_get_basename(path);
printf("基础名: %s\n", basename);  // 输出: file
free(basename);
```

### 路径组合

```c
// 组合路径
char *combined = path_combine("C:\\Users\\Name", "Documents\\file.txt");
printf("组合路径: %s\n", combined);  // 输出: C:\Users\Name\Documents\file.txt
free(combined);
```

## 错误处理

```c
PathError error;
const char *path = "C:\\Invalid<Path>";

if (!path_validate(path, &error)) {
    switch (error) {
        case PATH_ERROR_INVALID_CHARACTERS:
            printf("路径包含非法字符\n");
            break;
        case PATH_ERROR_TOO_LONG:
            printf("路径过长\n");
            break;
        case PATH_ERROR_INVALID_FORMAT:
            printf("路径格式无效\n");
            break;
        default:
            printf("未知错误\n");
            break;
    }
}
```

## 配置选项

### PathResolverConfig 结构体

```c
typedef struct {
    bool case_sensitive;     // 是否区分大小写（Windows通常为false）
    size_t max_path_length;  // 最大路径长度
    bool resolve_shortcuts;  // 是否解析快捷方式
    bool expand_environment; // 是否展开环境变量
    bool resolve_virtual;    // 是否解析虚拟路径
} PathResolverConfig;
```

### 配置示例

```c
// 严格模式配置
PathResolverConfig strict_config = {
    .case_sensitive = true,      // 区分大小写
    .max_path_length = 260,      // 标准Windows路径长度
    .resolve_shortcuts = false,  // 不解析快捷方式
    .expand_environment = false, // 不展开环境变量
    .resolve_virtual = false     // 不解析虚拟路径
};

// 宽松模式配置
PathResolverConfig lenient_config = {
    .case_sensitive = false,     // 不区分大小写
    .max_path_length = 32767,    // 长路径支持
    .resolve_shortcuts = true,   // 解析快捷方式
    .expand_environment = true,  // 展开环境变量
    .resolve_virtual = true      // 解析虚拟路径
};
```

## 实际应用场景

### 1. 文件资源管理器地址栏

```c
// 用户输入: "桌面\文档"
PathInfo *info = path_resolve("桌面\\文档", NULL);
if (info && info->is_valid) {
    // 显示规范化路径给用户
    printf("当前路径: %s\n", info->normalized_path);
    
    // 使用绝对路径进行文件操作
    if (fs_path_exists(info->absolute_path)) {
        // 执行文件操作
    }
}
path_info_free(info);
```

### 2. 拖拽文件处理

```c
// 处理拖拽的文件路径
const char *dropped_path = "C:\\Users\\Name\\Desktop\\file.txt";
char *normalized = path_normalize(dropped_path);
if (normalized) {
    // 验证路径合法性
    PathError error;
    if (path_validate(normalized, &error)) {
        // 执行文件操作
        printf("处理文件: %s\n", normalized);
    }
    free(normalized);
}
```

### 3. 快捷方式处理

```c
// 处理快捷方式文件
const char *shortcut = "C:\\Users\\Name\\Desktop\\快捷方式.lnk";
char *target = path_resolve_shortcut(shortcut);
if (target && strcmp(target, shortcut) != 0) {
    printf("快捷方式指向: %s\n", target);
    // 使用目标路径进行后续操作
}
free(target);
```

## 注意事项

1. **内存管理**: 所有返回的字符串都需要手动释放
2. **线程安全**: 当前实现不是线程安全的，多线程使用时需要加锁
3. **编码问题**: 路径中的中文字符需要确保编码一致性
4. **权限问题**: 某些路径可能需要管理员权限才能访问

## 性能考虑

- 路径规范化操作的时间复杂度为 O(n)
- 环境变量展开的时间复杂度为 O(n*m)，其中 m 是环境变量数量
- 快捷方式解析需要文件系统访问，相对较慢
- 建议对频繁访问的路径进行缓存

## 调试技巧

```c
// 启用详细调试信息
PathInfo *info = path_resolve("测试路径", NULL);
if (info) {
    printf("路径类型: %d\n", info->type);
    printf("是否有效: %s\n", info->is_valid ? "是" : "否");
    printf("错误码: %d (%s)\n", info->error, path_get_error_string(info->error));
    path_info_free(info);
}
```

这个路径解析模块为 FileScope 提供了强大而灵活的路径处理能力，确保文件操作的可靠性和用户体验的一致性。 