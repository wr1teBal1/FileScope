# 文件监控模块使用指南

## 概述

文件监控模块提供了实时监控目录变化的功能，支持检测文件的创建、删除、修改、重命名等事件。该模块基于Windows API的`ReadDirectoryChangesW`函数实现，提供了高效、可靠的目录监控能力。

## 主要功能

### 1. 监控目录变化
- **文件/文件夹创建**：检测新文件或目录的创建
- **文件/文件夹删除**：检测文件或目录的删除
- **文件/文件夹重命名**：检测文件或目录的重命名操作
- **内容修改**：检测文件大小、最后修改时间的变化
- **属性变更**：检测文件属性（如只读、隐藏）的变化

### 2. 事件过滤和防抖
- **文件模式过滤**：支持通配符模式，可排除或包含特定类型的文件
- **防抖机制**：避免短时间内重复事件的干扰
- **自定义配置**：可配置监控的目录深度、事件类型等

### 3. 异步事件处理
- **回调机制**：通过回调函数处理文件变化事件
- **线程安全**：使用临界区保护共享资源
- **非阻塞**：监控在独立线程中运行，不阻塞主程序

## API 接口

### 数据结构

#### FileEventType
```c
typedef enum {
    FILE_EVENT_CREATED,      // 文件/文件夹创建
    FILE_EVENT_DELETED,      // 文件/文件夹删除
    FILE_EVENT_MODIFIED,     // 文件内容修改
    FILE_EVENT_RENAMED,      // 文件/文件夹重命名
    FILE_EVENT_ATTRIBUTES,   // 属性变更
    FILE_EVENT_SIZE,         // 大小变更
    FILE_EVENT_ACCESS,       // 访问时间变更
    FILE_EVENT_UNKNOWN       // 未知事件
} FileEventType;
```

#### FileEvent
```c
typedef struct {
    FileEventType type;      // 事件类型
    char *file_path;         // 文件路径
    char *old_path;          // 重命名时的旧路径
    time_t timestamp;        // 事件时间戳
    size_t file_size;        // 文件大小
    bool is_directory;       // 是否为目录
    bool is_hidden;          // 是否为隐藏文件
} FileEvent;
```

#### FileWatcherConfig
```c
typedef struct {
    bool enabled;                    // 是否启用监控
    bool recursive;                  // 是否递归监控子目录
    bool watch_attributes;           // 是否监控属性变化
    bool watch_size;                 // 是否监控大小变化
    bool watch_access;               // 是否监控访问时间
    unsigned int debounce_ms;        // 防抖时间（毫秒）
    char **exclude_patterns;         // 排除的文件模式
    int exclude_pattern_count;       // 排除模式数量
    char **include_patterns;         // 包含的文件模式
    int include_pattern_count;       // 包含模式数量
} FileWatcherConfig;
```

### 主要函数

#### 创建和销毁
```c
// 创建文件监控器
FileWatcher* file_watcher_create(const char *directory_path, const FileWatcherConfig *config);

// 销毁文件监控器
void file_watcher_destroy(FileWatcher *watcher);
```

#### 启动和停止
```c
// 启动监控
bool file_watcher_start(FileWatcher *watcher);

// 停止监控
void file_watcher_stop(FileWatcher *watcher);

// 检查监控器是否正在运行
bool file_watcher_is_running(const FileWatcher *watcher);
```

#### 回调设置
```c
// 设置回调函数
void file_watcher_set_callback(FileWatcher *watcher, FileWatcherCallback callback, void *user_data);
```

#### 配置管理
```c
// 获取默认配置
FileWatcherConfig file_watcher_get_default_config(void);

// 添加排除模式
bool file_watcher_add_exclude_pattern(FileWatcher *watcher, const char *pattern);

// 添加包含模式
bool file_watcher_add_include_pattern(FileWatcher *watcher, const char *pattern);
```

#### 手动操作
```c
// 手动刷新目录
bool file_watcher_refresh(FileWatcher *watcher);

// 处理监控事件（主循环中调用）
void file_watcher_process_events(FileWatcher *watcher);
```

## 使用示例

### 基本使用

```c
#include "file_watcher.h"
#include <stdio.h>

// 文件变化回调函数
void on_file_changed(const FileEvent *event, void *user_data) {
    printf("文件变化: %s - %s\n", 
           file_event_type_to_string(event->type), 
           event->file_path);
}

int main() {
    // 创建配置
    FileWatcherConfig config = file_watcher_get_default_config();
    config.recursive = true;
    config.debounce_ms = 100;
    
    // 创建监控器
    FileWatcher *watcher = file_watcher_create(".", &config);
    if (!watcher) {
        printf("创建监控器失败\n");
        return 1;
    }
    
    // 设置回调
    file_watcher_set_callback(watcher, on_file_changed, NULL);
    
    // 启动监控
    if (!file_watcher_start(watcher)) {
        printf("启动监控失败\n");
        file_watcher_destroy(watcher);
        return 1;
    }
    
    printf("监控已启动，按 Ctrl+C 退出\n");
    
    // 主循环
    while (1) {
        file_watcher_process_events(watcher);
        Sleep(100);  // Windows
        // usleep(100000);  // Linux/Unix
    }
    
    // 清理
    file_watcher_destroy(watcher);
    return 0;
}
```

### 高级配置

```c
// 创建自定义配置
FileWatcherConfig config = file_watcher_get_default_config();
config.enabled = true;
config.recursive = true;
config.watch_attributes = true;
config.watch_size = true;
config.watch_access = false;
config.debounce_ms = 200;  // 200ms防抖

// 添加排除模式
file_watcher_add_exclude_pattern(&config, "*.tmp");
file_watcher_add_exclude_pattern(&config, "*.log");
file_watcher_add_exclude_pattern(&config, ".git");

// 添加包含模式
file_watcher_add_include_pattern(&config, "*.txt");
file_watcher_add_include_pattern(&config, "*.md");
```

## 性能考虑

### 1. 防抖设置
- 对于频繁变化的文件，建议设置适当的防抖时间（100-500ms）
- 过短的防抖时间可能导致事件重复
- 过长的防抖时间可能错过重要事件

### 2. 目录深度
- 递归监控会增加系统资源消耗
- 对于大型目录树，考虑只监控特定子目录
- 使用排除模式过滤不必要的文件

### 3. 事件类型
- 只监控需要的事件类型以减少CPU使用
- `watch_access` 通常不需要，会生成大量事件
- `watch_attributes` 和 `watch_size` 根据需要启用

## 错误处理

### 常见错误
1. **目录不存在**：确保监控的目录存在且有访问权限
2. **权限不足**：某些系统目录可能需要管理员权限
3. **句柄耗尽**：长时间运行时注意及时释放资源

### 错误检查
```c
FileWatcher *watcher = file_watcher_create(path, &config);
if (!watcher) {
    // 处理创建失败
    return;
}

if (!file_watcher_start(watcher)) {
    // 处理启动失败
    file_watcher_destroy(watcher);
    return;
}
```

## 平台支持

- **Windows**: 使用 `ReadDirectoryChangesW` API
- **Linux/Unix**: 预留接口，可扩展实现
- **跨平台**: 统一的API接口，平台特定实现

## 注意事项

1. **线程安全**：回调函数在监控线程中调用，注意线程安全
2. **资源管理**：及时销毁监控器释放系统资源
3. **事件处理**：避免在回调函数中执行耗时操作
4. **路径编码**：支持UTF-8编码的文件路径
5. **网络驱动器**：网络驱动器断开时可能产生异常事件

## 扩展功能

### 1. 事件队列
可以实现事件队列机制，将事件缓存后批量处理

### 2. 多目录监控
可以创建多个监控器同时监控不同目录

### 3. 事件持久化
可以将文件变化事件记录到日志文件

### 4. 自定义过滤器
可以实现更复杂的文件过滤逻辑 