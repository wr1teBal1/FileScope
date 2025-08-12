#ifndef FILE_WATCHER_H
#define FILE_WATCHER_H

#include "main.h"
#include <stdbool.h>
#include <time.h>
#include <sys/types.h>
#include <stddef.h>



// 文件变化事件类型
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

// 文件变化事件结构
typedef struct {
    FileEventType type;      // 事件类型
    char *file_path;         // 文件路径
    char *old_path;          // 重命名时的旧路径
    time_t timestamp;        // 事件时间戳
    size_t file_size;        // 文件大小
    bool is_directory;       // 是否为目录
    bool is_hidden;          // 是否为隐藏文件
} FileEvent;

// 文件监控回调函数类型
typedef void (*FileWatcherCallback)(const FileEvent *event, void *user_data);

// 文件监控配置
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

// 文件监控器结构
typedef struct FileWatcher FileWatcher;

// 创建文件监控器
FileWatcher* file_watcher_create(const char *directory_path, const FileWatcherConfig *config);

// 销毁文件监控器
void file_watcher_destroy(FileWatcher *watcher);

// 启动监控
bool file_watcher_start(FileWatcher *watcher);

// 停止监控
void file_watcher_stop(FileWatcher *watcher);

// 设置回调函数
void file_watcher_set_callback(FileWatcher *watcher, FileWatcherCallback callback, void *user_data);

// 检查监控器是否正在运行
bool file_watcher_is_running(const FileWatcher *watcher);

// 获取监控的目录路径
const char* file_watcher_get_directory(const FileWatcher *watcher);

// 手动刷新目录（扫描所有文件）
bool file_watcher_refresh(FileWatcher *watcher);

// 添加排除模式
bool file_watcher_add_exclude_pattern(FileWatcher *watcher, const char *pattern);

// 添加包含模式
bool file_watcher_add_include_pattern(FileWatcher *watcher, const char *pattern);

// 移除排除模式
bool file_watcher_remove_exclude_pattern(FileWatcher *watcher, const char *pattern);

// 移除包含模式
bool file_watcher_remove_include_pattern(FileWatcher *watcher, const char *pattern);

// 获取默认配置
FileWatcherConfig file_watcher_get_default_config(void);

// 释放文件事件
void file_event_free(FileEvent *event);

// 获取事件类型字符串
const char* file_event_type_to_string(FileEventType type);

// 检查文件是否匹配模式
bool file_watcher_match_pattern(const char *filename, const char *pattern);

// 处理监控事件（需要在主循环中调用）
void file_watcher_process_events(FileWatcher *watcher);

#endif // FILE_WATCHER_H 