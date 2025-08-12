# 路径解析模块集成指南

## 概述

本指南说明如何将路径解析模块集成到 FileScope 项目中，替换或增强现有的路径处理功能。

## 集成步骤

### 1. 包含头文件

在需要使用路径解析功能的源文件中添加：

```c
#include "path_resolver.h"
```

### 2. 初始化模块

在应用程序启动时初始化路径解析器：

```c
// 在 main() 函数或应用程序初始化函数中
bool init_path_resolver(void) {
    // 使用默认配置
    if (!path_resolver_init(NULL)) {
        printf("路径解析器初始化失败\n");
        return false;
    }
    
    // 或使用自定义配置
    PathResolverConfig config = {
        .case_sensitive = false,
        .max_path_length = 260,
        .resolve_shortcuts = true,
        .expand_environment = true,
        .resolve_virtual = true
    };
    
    if (!path_resolver_init(&config)) {
        printf("路径解析器初始化失败\n");
        return false;
    }
    
    return true;
}
```

### 3. 替换现有路径处理

#### 替换文件系统操作中的路径处理

**原来的代码：**
```c
// 直接使用用户输入的路径
if (fs_path_exists(user_input_path)) {
    // 执行操作
}
```

**集成后的代码：**
```c
// 使用路径解析器处理路径
PathInfo *info = path_resolve(user_input_path, current_directory);
if (info && info->is_valid) {
    if (fs_path_exists(info->absolute_path)) {
        // 使用规范化后的绝对路径执行操作
        perform_file_operation(info->absolute_path);
    }
} else {
    // 处理路径错误
    show_error_message(path_get_error_string(info->error));
}
path_info_free(info);
```

#### 替换地址栏路径处理

**原来的代码：**
```c
// 直接显示用户输入的路径
update_address_bar(user_input_path);
```

**集成后的代码：**
```c
// 显示规范化后的路径
PathInfo *info = path_resolve(user_input_path, current_directory);
if (info && info->is_valid) {
    update_address_bar(info->normalized_path);
    set_current_directory(info->absolute_path);
} else {
    show_error_message("无效路径");
}
path_info_free(info);
```

### 4. 增强拖拽功能

```c
void handle_file_drop(const char *dropped_path) {
    // 规范化拖拽的路径
    char *normalized = path_normalize(dropped_path);
    if (!normalized) {
        show_error_message("路径处理失败");
        return;
    }
    
    // 验证路径
    PathError error;
    if (!path_validate(normalized, &error)) {
        show_error_message(path_get_error_string(error));
        free(normalized);
        return;
    }
    
    // 解析路径（处理快捷方式等）
    PathInfo *info = path_resolve(normalized, current_directory);
    free(normalized);
    
    if (info && info->is_valid) {
        // 处理文件
        if (fs_is_directory(info->absolute_path)) {
            navigate_to_directory(info->absolute_path);
        } else if (fs_is_file(info->absolute_path)) {
            open_file(info->absolute_path);
        }
    }
    
    path_info_free(info);
}
```

### 5. 增强搜索功能

```c
void search_files(const char *search_path) {
    // 解析搜索路径
    PathInfo *info = path_resolve(search_path, current_directory);
    if (!info || !info->is_valid) {
        show_error_message("搜索路径无效");
        path_info_free(info);
        return;
    }
    
    // 使用绝对路径进行搜索
    perform_file_search(info->absolute_path);
    path_info_free(info);
}
```

### 6. 清理资源

在应用程序退出时清理路径解析器：

```c
void cleanup_path_resolver(void) {
    path_resolver_cleanup();
}

// 在 main() 函数末尾或应用程序清理函数中调用
cleanup_path_resolver();
```

## 与现有模块的集成

### 与 file_system.c 集成

修改 `file_system.c` 中的路径处理函数：

```c
// 在文件开头包含头文件
#include "path_resolver.h"

// 修改 fs_get_absolute_path 函数
char* fs_get_absolute_path(const char *path) {
    return path_to_absolute(path, fs_get_current_directory());
}

// 修改 fs_combine_path 函数
char* fs_combine_path(const char *path1, const char *path2) {
    return path_combine(path1, path2);
}

// 修改 fs_get_directory 函数
const char* fs_get_directory(const char *path) {
    char *dir = path_get_directory(path);
    // 注意：这里需要处理内存管理
    // 可以考虑使用静态缓冲区或修改接口设计
    return dir;
}
```

### 与 file_list.c 集成

```c
// 在文件列表处理中使用路径解析
void load_directory_contents(const char *path) {
    PathInfo *info = path_resolve(path, current_directory);
    if (info && info->is_valid) {
        // 使用绝对路径加载目录内容
        DIR *dir = fs_open_directory(info->absolute_path);
        if (dir) {
            // 处理目录内容
            struct dirent *entry;
            while ((entry = fs_read_directory(dir)) != NULL) {
                // 处理每个文件/目录项
                process_file_item(entry, info->absolute_path);
            }
            fs_close_directory(dir);
        }
    }
    path_info_free(info);
}
```

### 与 sidebar.c 集成

```c
// 在侧边栏导航中使用路径解析
void navigate_to_path(const char *path) {
    PathInfo *info = path_resolve(path, current_directory);
    if (info && info->is_valid) {
        // 更新侧边栏显示
        update_sidebar_display(info->normalized_path);
        
        // 更新当前目录
        set_current_directory(info->absolute_path);
        
        // 加载目录内容
        load_directory_contents(info->absolute_path);
    }
    path_info_free(info);
}
```

## 配置选项

### 开发环境配置

```c
PathResolverConfig dev_config = {
    .case_sensitive = false,     // Windows 开发环境
    .max_path_length = 260,      // 标准路径长度
    .resolve_shortcuts = true,   // 解析快捷方式
    .expand_environment = true,  // 展开环境变量
    .resolve_virtual = true      // 解析虚拟路径
};
```

### 生产环境配置

```c
PathResolverConfig prod_config = {
    .case_sensitive = false,
    .max_path_length = 32767,    // 长路径支持
    .resolve_shortcuts = true,
    .expand_environment = true,
    .resolve_virtual = true
};
```

### 调试配置

```c
PathResolverConfig debug_config = {
    .case_sensitive = true,      // 严格模式
    .max_path_length = 260,
    .resolve_shortcuts = false,  // 禁用快捷方式解析
    .expand_environment = false, // 禁用环境变量展开
    .resolve_virtual = false     // 禁用虚拟路径解析
};
```

## 错误处理

### 全局错误处理

```c
void handle_path_error(PathError error, const char *path) {
    switch (error) {
        case PATH_ERROR_INVALID_CHARACTERS:
            show_error_dialog("路径包含非法字符", path);
            break;
        case PATH_ERROR_TOO_LONG:
            show_error_dialog("路径过长", path);
            break;
        case PATH_ERROR_INVALID_FORMAT:
            show_error_dialog("路径格式无效", path);
            break;
        case PATH_ERROR_ENVIRONMENT_VARIABLE_NOT_FOUND:
            show_warning_dialog("环境变量未找到", path);
            break;
        case PATH_ERROR_SHORTCUT_RESOLUTION_FAILED:
            show_warning_dialog("快捷方式解析失败", path);
            break;
        default:
            show_error_dialog("路径处理错误", path);
            break;
    }
}
```

### 路径验证包装函数

```c
bool validate_and_resolve_path(const char *input_path, char **output_path) {
    PathInfo *info = path_resolve(input_path, current_directory);
    if (!info || !info->is_valid) {
        if (info) {
            handle_path_error(info->error, input_path);
            path_info_free(info);
        }
        return false;
    }
    
    *output_path = strdup(info->absolute_path);
    path_info_free(info);
    return true;
}
```

## 性能优化

### 路径缓存

```c
// 简单的路径缓存实现
typedef struct {
    char *key;
    char *value;
    time_t timestamp;
} PathCacheEntry;

static PathCacheEntry path_cache[100];
static int cache_index = 0;

char* get_cached_path(const char *path) {
    // 查找缓存
    for (int i = 0; i < 100; i++) {
        if (path_cache[i].key && strcmp(path_cache[i].key, path) == 0) {
            // 检查缓存是否过期（5分钟）
            if (time(NULL) - path_cache[i].timestamp < 300) {
                return strdup(path_cache[i].value);
            }
        }
    }
    return NULL;
}

void cache_path(const char *path, const char *resolved_path) {
    // 释放旧的缓存项
    if (path_cache[cache_index].key) {
        free(path_cache[cache_index].key);
        free(path_cache[cache_index].value);
    }
    
    // 添加新的缓存项
    path_cache[cache_index].key = strdup(path);
    path_cache[cache_index].value = strdup(resolved_path);
    path_cache[cache_index].timestamp = time(NULL);
    
    cache_index = (cache_index + 1) % 100;
}
```

### 优化的路径解析

```c
char* optimized_path_resolve(const char *path) {
    // 首先检查缓存
    char *cached = get_cached_path(path);
    if (cached) {
        return cached;
    }
    
    // 解析路径
    PathInfo *info = path_resolve(path, current_directory);
    if (info && info->is_valid) {
        char *result = strdup(info->absolute_path);
        
        // 缓存结果
        cache_path(path, result);
        
        path_info_free(info);
        return result;
    }
    
    path_info_free(info);
    return NULL;
}
```

## 测试集成

### 单元测试

```c
void test_path_resolver_integration(void) {
    // 测试基本路径解析
    PathInfo *info = path_resolve("桌面\\文件.txt", "C:\\Users\\Test");
    assert(info != NULL);
    assert(info->is_valid);
    assert(info->absolute_path != NULL);
    path_info_free(info);
    
    // 测试路径验证
    PathError error;
    assert(path_validate("C:\\Valid\\Path", &error));
    assert(!path_validate("C:\\Invalid<Path>", &error));
    
    // 测试路径规范化
    char *normalized = path_normalize("C:\\Dir\\\\Sub\\File");
    assert(normalized != NULL);
    assert(strcmp(normalized, "c:\\dir\\sub\\file") == 0);
    free(normalized);
}
```

### 集成测试

```c
void test_file_operations_with_path_resolver(void) {
    // 测试文件操作与路径解析器的集成
    const char *test_paths[] = {
        "桌面\\测试文件.txt",
        "%USERPROFILE%\\Documents\\file.txt",
        ".\\Downloads\\file.txt"
    };
    
    for (int i = 0; i < 3; i++) {
        PathInfo *info = path_resolve(test_paths[i], current_directory);
        if (info && info->is_valid) {
            // 测试文件系统操作
            bool exists = fs_path_exists(info->absolute_path);
            printf("路径 %s 存在: %s\n", test_paths[i], exists ? "是" : "否");
        }
        path_info_free(info);
    }
}
```

通过以上集成步骤，路径解析模块可以无缝地集成到 FileScope 项目中，提供强大而可靠的路径处理能力。 