/*
 * 文件监控管理器
 * 封装文件监控功能，提供简洁的接口
 */

#include "file_monitor.h"
#include "file_system.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 文件监控管理器结构
struct FileMonitor {
    FileWatcher *watcher;           // 文件监控器
    char *current_directory;        // 当前监控的目录
    FileChangeCallback callback;    // 文件变化回调函数
    bool is_active;                 // 是否正在监控
    int event_count;                // 事件计数
};

// 默认文件变化回调函数
static void default_file_change_callback(const FileEvent *event, void *user_data) {
    FileMonitor *monitor = (FileMonitor*)user_data;
    if (!monitor) return;
    
    monitor->event_count++;
    
    printf("[文件监控] 事件 #%d: %s - %s\n", 
           monitor->event_count,
           file_event_type_to_string(event->type),
           event->file_path ? event->file_path : "未知文件");
    
    // 这里可以添加更多的业务逻辑
    // 比如：更新文件列表、触发备份、发送通知等
}

// 创建文件监控管理器
FileMonitor* file_monitor_create(void) {
    FileMonitor *monitor = malloc(sizeof(FileMonitor));
    if (!monitor) return NULL;
    
    monitor->watcher = NULL;
    monitor->current_directory = NULL;
    monitor->callback = NULL;
    monitor->is_active = false;
    monitor->event_count = 0;
    
    return monitor;
}

// 销毁文件监控管理器
void file_monitor_destroy(FileMonitor *monitor) {
    if (!monitor) return;
    
    // 停止监控
    file_monitor_stop(monitor);
    
    // 释放资源
    if (monitor->current_directory) {
        free(monitor->current_directory);
    }
    
    free(monitor);
}

// 开始监控指定目录
bool file_monitor_start(FileMonitor *monitor, const char *directory_path) {
    if (!monitor || !directory_path) return false;
    
    // 如果已经在监控，先停止
    if (monitor->is_active) {
        file_monitor_stop(monitor);
    }
    
    // 检查目录是否存在
    if (!fs_path_exists(directory_path)) {
        printf("[错误] 目录不存在: %s\n", directory_path);
        return false;
    }
    
    // 创建文件监控配置
    FileWatcherConfig config = file_watcher_get_default_config();
    config.enabled = true;
    config.recursive = true;           // 递归监控子目录
    config.watch_attributes = true;    // 监控属性变化
    config.watch_size = true;          // 监控大小变化
    config.watch_access = false;       // 不监控访问时间（避免过多事件）
    config.debounce_ms = 150;         // 150ms防抖
    
    // 创建文件监控器
    monitor->watcher = file_watcher_create(directory_path, &config);
    if (!monitor->watcher) {
        printf("[错误] 无法创建文件监控器\n");
        return false;
    }
    
    // 添加一些排除模式
    file_monitor_add_exclude_pattern(monitor, "*.tmp");
    file_monitor_add_exclude_pattern(monitor, "*.temp");
    file_monitor_add_exclude_pattern(monitor, "*.bak");
    file_monitor_add_exclude_pattern(monitor, "Thumbs.db");
    file_monitor_add_exclude_pattern(monitor, ".DS_Store");
    file_monitor_add_exclude_pattern(monitor, ".git");
    file_monitor_add_exclude_pattern(monitor, ".svn");
    
    // 设置回调函数
    FileChangeCallback callback = monitor->callback ? monitor->callback : default_file_change_callback;
    file_watcher_set_callback(monitor->watcher, (FileWatcherCallback)callback, monitor);
    
    // 启动监控
    if (!file_watcher_start(monitor->watcher)) {
        printf("[错误] 无法启动文件监控\n");
        file_watcher_destroy(monitor->watcher);
        monitor->watcher = NULL;
        return false;
    }
    
    // 保存当前目录路径
    if (monitor->current_directory) {
        free(monitor->current_directory);
    }
    monitor->current_directory = strdup(directory_path);
    monitor->is_active = true;
    
    printf("[成功] 文件监控已启动\n");
    printf("  监控目录: %s\n", directory_path);
    printf("  递归监控: %s\n", config.recursive ? "是" : "否");
    printf("  防抖时间: %u ms\n", config.debounce_ms);
    
    return true;
}

// 停止监控
void file_monitor_stop(FileMonitor *monitor) {
    if (!monitor || !monitor->is_active) return;
    
    if (monitor->watcher) {
        file_watcher_stop(monitor->watcher);
        file_watcher_destroy(monitor->watcher);
        monitor->watcher = NULL;
    }
    
    monitor->is_active = false;
    printf("[信息] 文件监控已停止\n");
}

// 设置文件变化回调函数
void file_monitor_set_callback(FileMonitor *monitor, FileChangeCallback callback) {
    if (!monitor) return;
    
    monitor->callback = callback;
    
    // 如果监控器已经创建，更新回调
    if (monitor->watcher) {
        FileChangeCallback actual_callback = callback ? callback : default_file_change_callback;
        file_watcher_set_callback(monitor->watcher, (FileWatcherCallback)actual_callback, monitor);
    }
    
    printf("[设置] 文件变化回调已设置\n");
}

// 处理文件监控事件（在主循环中调用）
void file_monitor_process_events(FileMonitor *monitor) {
    if (!monitor || !monitor->is_active || !monitor->watcher) return;
    
    file_watcher_process_events(monitor->watcher);
}

// 检查是否正在监控
bool file_monitor_is_active(FileMonitor *monitor) {
    return monitor ? monitor->is_active : false;
}

// 获取当前监控的目录
const char* file_monitor_get_directory(FileMonitor *monitor) {
    return monitor ? monitor->current_directory : NULL;
}

// 手动刷新当前目录
bool file_monitor_refresh(FileMonitor *monitor) {
    if (!monitor || !monitor->is_active || !monitor->watcher) {
        printf("[错误] 文件监控器未激活\n");
        return false;
    }
    
    printf("[信息] 手动刷新目录: %s\n", monitor->current_directory);
    return file_watcher_refresh(monitor->watcher);
}

// 添加排除模式
bool file_monitor_add_exclude_pattern(FileMonitor *monitor, const char *pattern) {
    if (!monitor || !pattern) return false;
    
    if (!monitor->watcher) {
        printf("[错误] 文件监控器未创建\n");
        return false;
    }
    
    if (file_watcher_add_exclude_pattern(monitor->watcher, pattern)) {
        printf("[成功] 添加排除模式: %s\n", pattern);
        return true;
    } else {
        printf("[错误] 添加排除模式失败: %s\n", pattern);
        return false;
    }
}

// 获取监控状态信息
void file_monitor_get_status(FileMonitor *monitor) {
    if (!monitor) {
        printf("[状态] 文件监控: 未初始化\n");
        return;
    }
    
    printf("[状态] 文件监控状态:\n");
    printf("  监控目录: %s\n", monitor->current_directory ? monitor->current_directory : "无");
    printf("  监控状态: %s\n", monitor->is_active ? "运行中" : "已停止");
    printf("  事件计数: %d\n", monitor->event_count);
    
    if (monitor->watcher) {
        printf("  监控器状态: %s\n", file_watcher_is_running(monitor->watcher) ? "运行中" : "已停止");
    }
} 