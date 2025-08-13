#ifndef FILE_MONITOR_H
#define FILE_MONITOR_H

#include "main.h"
#include "file_watcher.h"
#include <stdbool.h>

// 文件监控管理器结构
typedef struct FileMonitor FileMonitor;

// 文件变化通知回调函数类型
typedef void (*FileChangeCallback)(const FileEvent *event, void *user_data);

// 创建文件监控管理器
FileMonitor* file_monitor_create(void);

// 销毁文件监控管理器
void file_monitor_destroy(FileMonitor *monitor);

// 开始监控指定目录
bool file_monitor_start(FileMonitor *monitor, const char *directory_path);

// 停止监控
void file_monitor_stop(FileMonitor *monitor);

// 设置文件变化回调函数
void file_monitor_set_callback(FileMonitor *monitor, FileChangeCallback callback);

// 处理文件监控事件（在主循环中调用）
void file_monitor_process_events(FileMonitor *monitor);

// 检查是否正在监控
bool file_monitor_is_active(FileMonitor *monitor);

// 获取当前监控的目录
const char* file_monitor_get_directory(FileMonitor *monitor);

// 手动刷新当前目录
bool file_monitor_refresh(FileMonitor *monitor);

// 添加排除模式
bool file_monitor_add_exclude_pattern(FileMonitor *monitor, const char *pattern);

// 获取监控状态信息
void file_monitor_get_status(FileMonitor *monitor);

#endif // FILE_MONITOR_H 