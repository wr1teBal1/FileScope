# 文件监控模块使用说明

## 概述

文件监控模块已经成功集成到FileScope项目中，提供实时文件系统监控功能。该模块基于Windows API的`ReadDirectoryChangesW`函数实现，支持检测文件的创建、删除、修改、重命名等事件。

## 功能特性

- ✅ **实时监控** - 自动检测目录变化
- ✅ **事件过滤** - 智能排除临时文件和系统文件
- ✅ **防抖机制** - 避免短时间内重复事件
- ✅ **异步处理** - 不阻塞主程序执行
- ✅ **线程安全** - 使用临界区保护共享资源
- ✅ **自动清理** - 程序结束时自动释放资源

## 集成状态

### 1. 主程序集成 ✅
文件监控功能已经集成到主程序中：
- 程序启动时自动监控当前目录
- 主循环中自动处理文件监控事件
- 程序结束时自动清理资源

### 2. 文件结构
```
FileScope/
├── include/
│   ├── file_watcher.h          # 核心文件监控头文件
│   └── file_monitor.h          # 文件监控管理器头文件
├── engine/filesystem/
│   ├── file_watcher.c          # 核心文件监控实现
│   └── file_monitor.c          # 文件监控管理器实现
├── app/
│   └── app.c                   # 主程序（已集成文件监控）
└── test_file_monitor.c         # 测试程序
```

## 使用方法

### 1. 基本使用

```c
#include "file_monitor.h"

// 创建文件监控管理器
FileMonitor *monitor = file_monitor_create();

// 设置自定义回调函数（可选）
file_monitor_set_callback(monitor, my_callback_function);

// 开始监控指定目录
file_monitor_start(monitor, "/path/to/directory");

// 在主循环中处理事件
while (running) {
    file_monitor_process_events(monitor);
    Sleep(100);
}

// 清理资源
file_monitor_destroy(monitor);
```

### 2. 自定义回调函数

```c
void my_callback_function(const FileEvent *event, void *user_data) {
    // user_data 可以用于传递额外的上下文信息
    // 例如：FileMonitor 实例、用户数据等
    
    switch (event->type) {
        case FILE_EVENT_CREATED:
            printf("新文件创建: %s\n", event->file_path);
            break;
        case FILE_EVENT_DELETED:
            printf("文件删除: %s\n", event->file_path);
            break;
        case FILE_EVENT_MODIFIED:
            printf("文件修改: %s\n", event->file_path);
            break;
        case FILE_EVENT_RENAMED:
            printf("文件重命名: %s\n", event->file_path);
            break;
    }
}
```

### 3. 监控状态管理

```c
// 检查监控状态
if (file_monitor_is_active(monitor)) {
    printf("文件监控正在运行\n");
}

// 获取监控目录
const char *dir = file_monitor_get_directory(monitor);
printf("当前监控目录: %s\n", dir);

// 手动刷新
file_monitor_refresh(monitor);

// 获取详细状态
file_monitor_get_status(monitor);
```

## 配置选项

### 1. 默认配置
- 递归监控子目录：启用
- 监控属性变化：启用
- 监控大小变化：启用
- 监控访问时间：禁用
- 防抖时间：150ms

### 2. 排除模式
自动排除以下文件类型：
- `*.tmp`, `*.temp`, `*.bak` - 临时文件
- `Thumbs.db`, `.DS_Store` - 系统文件
- `.git`, `.svn` - 版本控制目录

### 3. 添加自定义排除模式

```c
// 添加排除模式
file_monitor_add_exclude_pattern(monitor, "*.log");
file_monitor_add_exclude_pattern(monitor, "*.cache");
```

## 测试验证

### 1. 运行测试程序
```bash
cd FileScope
gcc -I. -o test_file_monitor test_file_monitor.c engine/filesystem/*.c
./test_file_monitor
```

### 2. 测试功能
测试程序会自动执行以下操作：
- 创建测试文件
- 修改文件内容
- 删除文件
- 手动刷新目录

### 3. 验证输出
正常运行时应该看到：
- 文件监控启动成功信息
- 文件变化事件通知
- 事件类型和文件路径信息

## 性能考虑

### 1. 防抖设置
- 默认防抖时间：150ms
- 可根据需要调整：频繁变化的目录建议增加防抖时间

### 2. 监控范围
- 递归监控会增加系统资源消耗
- 建议只监控必要的目录

### 3. 事件处理
- 避免在回调函数中执行耗时操作
- 使用事件队列机制处理复杂业务逻辑

## 常见问题

### 1. 监控器无法启动
- 检查目录路径是否存在
- 确认有足够的访问权限
- 检查是否已经启动了监控器

### 2. 事件回调不触发
- 确认在主循环中调用了`file_monitor_process_events()`
- 检查回调函数是否正确设置
- 确认监控器正在运行

### 3. 性能问题
- 增加防抖时间
- 使用排除模式过滤文件
- 减少监控的事件类型

## 扩展功能

### 1. 多目录监控
可以创建多个FileMonitor实例监控不同目录：

```c
FileMonitor *monitor1 = file_monitor_create();
FileMonitor *monitor2 = file_monitor_create();

file_monitor_start(monitor1, "/path1");
file_monitor_start(monitor2, "/path2");
```

### 2. 动态配置
可以在运行时动态调整监控配置：

```c
// 停止当前监控
file_monitor_stop(monitor);

// 重新配置并启动
file_monitor_start(monitor, "/new/path");
```

### 3. 事件统计
监控器会自动统计事件数量，可用于性能分析：

```c
file_monitor_get_status(monitor);
// 输出包含事件计数信息
```

## 总结

文件监控模块已经成功集成到FileScope项目中，提供了：

1. **开箱即用** - 程序启动时自动开始监控
2. **零配置** - 使用合理的默认配置
3. **高性能** - 异步处理，不阻塞主程序
4. **易扩展** - 支持自定义回调和配置
5. **自动管理** - 程序结束时自动清理资源

该模块为FileScope提供了强大的实时文件系统监控能力，可以实现文件列表的自动更新、实时备份、开发环境监控等功能。 