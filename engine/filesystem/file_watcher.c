/*
 * 文件监控模块
 * 职责：
 * 1. 监控目录变化
 * 2. 发送文件变化通知
 * 3. 处理文件系统事件
 * 4. 自动刷新支持
 */

#include "file_watcher.h"
#include "file_system.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <stddef.h>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <tchar.h>
#include <winbase.h>
#endif

// 文件监控器内部结构
struct FileWatcher {
    char *directory_path;            // 监控的目录路径
    FileWatcherConfig config;        // 配置
    FileWatcherCallback callback;    // 回调函数
    void *user_data;                 // 用户数据
    
    // Windows 特定成员
#ifdef _WIN32
    HANDLE directory_handle;         // 目录句柄
    HANDLE change_handle;            // 变化通知句柄
    OVERLAPPED overlapped;           // 异步I/O结构
    char buffer[4096];               // 事件缓冲区
    DWORD bytes_returned;            // 返回的字节数
    bool is_running;                 // 是否正在运行
    HANDLE thread_handle;            // 监控线程句柄
    DWORD thread_id;                 // 线程ID
    CRITICAL_SECTION critical_section; // 临界区
#endif
    
    // 防抖相关
    struct {
        char *last_file_path;        // 最后一个文件路径
        FileEventType last_type;     // 最后一个事件类型
        time_t last_timestamp;       // 最后一个时间戳
        size_t last_size;            // 最后一个文件大小
    } debounce;
};

// 默认排除的文件模式
static const char* DEFAULT_EXCLUDE_PATTERNS[] = {
    "*.tmp", "*.temp", "*.bak", "*.swp", "*.swo",
    "~*", ".#*", "#*#", "*.log", "Thumbs.db",
    ".DS_Store", ".git", ".svn", ".hg"
};

// 默认配置
FileWatcherConfig file_watcher_get_default_config(void) {
    FileWatcherConfig config = {0};
    config.enabled = true;
    config.recursive = true;
    config.watch_attributes = true;
    config.watch_size = true;
    config.watch_access = false;
    config.debounce_ms = 100;  // 100ms防抖
    config.exclude_patterns = NULL;
    config.exclude_pattern_count = 0;
    config.include_patterns = NULL;
    config.include_pattern_count = 0;
    return config;
}

// 检查文件是否匹配模式
bool file_watcher_match_pattern(const char *filename, const char *pattern) {
    if (!filename || !pattern) return false;
    
    // 简单的通配符匹配实现
    const char *f = filename;
    const char *p = pattern;
    
    while (*f && *p) {
        if (*p == '*') {
            // 跳过连续的*
            while (*p == '*') p++;
            if (!*p) return true;  // 模式以*结尾
            
            // 查找下一个匹配位置
            while (*f && *f != *p) f++;
            if (!*f) return false;
        } else if (*p == '?' || *f == *p) {
            f++;
            p++;
        } else {
            return false;
        }
    }
    
    return !*f && !*p;
}

// 检查文件是否应该被排除
static bool should_exclude_file(const FileWatcher *watcher, const char *filename) {
    if (!filename) return true;
    
    // 检查排除模式
    for (int i = 0; i < watcher->config.exclude_pattern_count; i++) {
        if (file_watcher_match_pattern(filename, watcher->config.exclude_patterns[i])) {
            return true;
        }
    }
    
    // 检查包含模式（如果有的话）
    if (watcher->config.include_pattern_count > 0) {
        bool included = false;
        for (int i = 0; i < watcher->config.include_pattern_count; i++) {
            if (file_watcher_match_pattern(filename, watcher->config.include_patterns[i])) {
                included = true;
                break;
            }
        }
        if (!included) return true;
    }
    
    return false;
}

// 创建文件事件
static FileEvent* create_file_event(FileEventType type, const char *file_path, const char *old_path) {
    FileEvent *event = malloc(sizeof(FileEvent));
    if (!event) return NULL;
    
    event->type = type;
    event->file_path = file_path ? strdup(file_path) : NULL;
    event->old_path = old_path ? strdup(old_path) : NULL;
    event->timestamp = time(NULL);
    event->file_size = 0;
    event->is_directory = false;
    event->is_hidden = false;
    
    // 获取文件信息
    if (file_path) {
        struct stat st;
        if (stat(file_path, &st) == 0) {
            event->file_size = st.st_size;
            event->is_directory = S_ISDIR(st.st_mode);
            event->is_hidden = fs_is_hidden(file_path);
        }
    }
    
    return event;
}

// 防抖检查
static bool should_debounce(const FileWatcher *watcher, const FileEvent *event) {
    if (watcher->config.debounce_ms == 0) return false;
    
    time_t now = time(NULL);
    time_t diff = now - watcher->debounce.last_timestamp;
    
    // 检查是否是同一个文件的相同事件类型
    if (watcher->debounce.last_file_path && 
        event->file_path &&
        strcmp(watcher->debounce.last_file_path, event->file_path) == 0 &&
        watcher->debounce.last_type == event->type) {
        
        // 如果在防抖时间内，则忽略
        if (diff * 1000 < watcher->config.debounce_ms) {
            return true;
        }
    }
    
    return false;
}

// 更新防抖信息
static void update_debounce_info(FileWatcher *watcher, const FileEvent *event) {
    if (watcher->debounce.last_file_path) {
        free(watcher->debounce.last_file_path);
    }
    
    watcher->debounce.last_file_path = event->file_path ? strdup(event->file_path) : NULL;
    watcher->debounce.last_type = event->type;
    watcher->debounce.last_timestamp = event->timestamp;
    watcher->debounce.last_size = event->file_size;
}

// 处理文件事件
static void process_file_event(FileWatcher *watcher, FileEvent *event) {
    if (!watcher || !event) return;
    
    // 检查是否应该排除
    if (event->file_path && should_exclude_file(watcher, fs_get_filename(event->file_path))) {
        file_event_free(event);
        return;
    }
    
    // 防抖检查
    if (should_debounce(watcher, event)) {
        file_event_free(event);
        return;
    }
    
    // 更新防抖信息
    update_debounce_info(watcher, event);
    
    // 调用回调函数
    if (watcher->callback) {
        watcher->callback(event, watcher->user_data);
    }
    
    file_event_free(event);
}

#ifdef _WIN32
// Windows 监控线程函数
static DWORD WINAPI file_watcher_thread(LPVOID param) {
    FileWatcher *watcher = (FileWatcher*)param;
    
    while (watcher->is_running) {
        DWORD bytes_returned = 0;
        BOOL success = GetOverlappedResult(
            watcher->directory_handle,
            &watcher->overlapped,
            &bytes_returned,
            FALSE
        );
        
        if (success && bytes_returned > 0) {
            // 处理文件变化事件
            FILE_NOTIFY_INFORMATION *info = (FILE_NOTIFY_INFORMATION*)watcher->buffer;
            
            while (info) {
                // 转换文件名
                char filename[MAX_PATH];
                int len = WideCharToMultiByte(CP_UTF8, 0, info->FileName, 
                    info->FileNameLength / sizeof(WCHAR), filename, MAX_PATH, NULL, NULL);
                if (len > 0) {
                    filename[len] = '\0';
                    
                    // 构建完整路径
                    char full_path[MAX_PATH];
                    snprintf(full_path, MAX_PATH, "%s\\%s", watcher->directory_path, filename);
                    
                    // 确定事件类型
                    FileEventType event_type = FILE_EVENT_UNKNOWN;
                    switch (info->Action) {
                        case FILE_ACTION_ADDED:
                            event_type = FILE_EVENT_CREATED;
                            break;
                        case FILE_ACTION_REMOVED:
                            event_type = FILE_EVENT_DELETED;
                            break;
                        case FILE_ACTION_MODIFIED:
                            event_type = FILE_EVENT_MODIFIED;
                            break;
                        case FILE_ACTION_RENAMED_OLD_NAME:
                            // 处理重命名
                            event_type = FILE_EVENT_RENAMED;
                            break;
                        case FILE_ACTION_RENAMED_NEW_NAME:
                            event_type = FILE_EVENT_RENAMED;
                            break;
                    }
                    
                    // 创建并处理事件
                    if (event_type != FILE_EVENT_UNKNOWN) {
                        FileEvent *event = create_file_event(event_type, full_path, NULL);
                        if (event) {
                            EnterCriticalSection(&watcher->critical_section);
                            process_file_event(watcher, event);
                            LeaveCriticalSection(&watcher->critical_section);
                        }
                    }
                }
                
                // 移动到下一个事件
                if (info->NextEntryOffset == 0) break;
                info = (FILE_NOTIFY_INFORMATION*)((char*)info + info->NextEntryOffset);
            }
            
            // 重新开始监控
            memset(&watcher->overlapped, 0, sizeof(OVERLAPPED));
            watcher->overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
            
            ReadDirectoryChangesW(
                watcher->directory_handle,
                watcher->buffer,
                sizeof(watcher->buffer),
                watcher->config.recursive,
                FILE_NOTIFY_CHANGE_FILE_NAME |
                FILE_NOTIFY_CHANGE_DIR_NAME |
                FILE_NOTIFY_CHANGE_ATTRIBUTES |
                FILE_NOTIFY_CHANGE_SIZE |
                FILE_NOTIFY_CHANGE_LAST_WRITE |
                FILE_NOTIFY_CHANGE_LAST_ACCESS,
                NULL,
                &watcher->overlapped,
                NULL
            );
        }
        
        Sleep(10);  // 短暂休眠避免CPU占用过高
    }
    
    return 0;
}
#endif

// 创建文件监控器
FileWatcher* file_watcher_create(const char *directory_path, const FileWatcherConfig *config) {
    if (!directory_path) return NULL;
    
    FileWatcher *watcher = malloc(sizeof(FileWatcher));
    if (!watcher) return NULL;
    
    // 初始化基本成员
    watcher->directory_path = strdup(directory_path);
    watcher->callback = NULL;
    watcher->user_data = NULL;
    watcher->config = config ? *config : file_watcher_get_default_config();
    
    // 初始化防抖信息
    memset(&watcher->debounce, 0, sizeof(watcher->debounce));
    
#ifdef _WIN32
    // 初始化Windows特定成员
    watcher->directory_handle = INVALID_HANDLE_VALUE;
    watcher->change_handle = INVALID_HANDLE_VALUE;
    watcher->is_running = false;
    watcher->thread_handle = NULL;
    watcher->thread_id = 0;
    InitializeCriticalSection(&watcher->critical_section);
    
    // 打开目录句柄
    watcher->directory_handle = CreateFileA(
        directory_path,
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
        NULL
    );
    
    if (watcher->directory_handle == INVALID_HANDLE_VALUE) {
        free(watcher->directory_path);
        free(watcher);
        return NULL;
    }
#endif
    
    return watcher;
}

// 销毁文件监控器
void file_watcher_destroy(FileWatcher *watcher) {
    if (!watcher) return;
    
    // 停止监控
    file_watcher_stop(watcher);
    
    // 释放资源
    if (watcher->directory_path) {
        free(watcher->directory_path);
    }
    
    if (watcher->debounce.last_file_path) {
        free(watcher->debounce.last_file_path);
    }
    
#ifdef _WIN32
    if (watcher->directory_handle != INVALID_HANDLE_VALUE) {
        CloseHandle(watcher->directory_handle);
    }
    DeleteCriticalSection(&watcher->critical_section);
#endif
    
    free(watcher);
}

// 启动监控
bool file_watcher_start(FileWatcher *watcher) {
    if (!watcher || !watcher->config.enabled) return false;
    
#ifdef _WIN32
    if (watcher->is_running) return true;
    
    // 初始化异步I/O
    memset(&watcher->overlapped, 0, sizeof(OVERLAPPED));
    watcher->overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    
    // 开始监控
    BOOL success = ReadDirectoryChangesW(
        watcher->directory_handle,
        watcher->buffer,
        sizeof(watcher->buffer),
        watcher->config.recursive,
        FILE_NOTIFY_CHANGE_FILE_NAME |
        FILE_NOTIFY_CHANGE_DIR_NAME |
        FILE_NOTIFY_CHANGE_ATTRIBUTES |
        FILE_NOTIFY_CHANGE_SIZE |
        FILE_NOTIFY_CHANGE_LAST_WRITE |
        FILE_NOTIFY_CHANGE_LAST_ACCESS,
        NULL,
        &watcher->overlapped,
        NULL
    );
    
    if (success) {
        watcher->is_running = true;
        
        // 创建监控线程
        watcher->thread_handle = CreateThread(
            NULL,
            0,
            file_watcher_thread,
            watcher,
            0,
            &watcher->thread_id
        );
        
        return watcher->thread_handle != NULL;
    }
#endif
    
    return false;
}

// 停止监控
void file_watcher_stop(FileWatcher *watcher) {
    if (!watcher) return;
    
#ifdef _WIN32
    if (watcher->is_running) {
        watcher->is_running = false;
        
        if (watcher->thread_handle) {
            WaitForSingleObject(watcher->thread_handle, 1000);
            CloseHandle(watcher->thread_handle);
            watcher->thread_handle = NULL;
        }
    }
#endif
}

// 设置回调函数
void file_watcher_set_callback(FileWatcher *watcher, FileWatcherCallback callback, void *user_data) {
    if (watcher) {
        watcher->callback = callback;
        watcher->user_data = user_data;
    }
}

// 检查监控器是否正在运行
bool file_watcher_is_running(const FileWatcher *watcher) {
    return watcher ? watcher->is_running : false;
}

// 获取监控的目录路径
const char* file_watcher_get_directory(const FileWatcher *watcher) {
    return watcher ? watcher->directory_path : NULL;
}

// 手动刷新目录
bool file_watcher_refresh(FileWatcher *watcher) {
    if (!watcher || !watcher->callback) return false;
    
    DIR *dir = fs_open_directory(watcher->directory_path);
    if (!dir) return false;
    
    struct dirent *entry;
    while ((entry = fs_read_directory(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        // 构建完整路径
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", watcher->directory_path, entry->d_name);
        
        // 检查是否应该排除
        if (should_exclude_file(watcher, entry->d_name)) {
            continue;
        }
        
        // 创建事件
        FileEvent *event = create_file_event(FILE_EVENT_CREATED, full_path, NULL);
        if (event) {
            process_file_event(watcher, event);
        }
    }
    
    fs_close_directory(dir);
    return true;
}

// 添加排除模式
bool file_watcher_add_exclude_pattern(FileWatcher *watcher, const char *pattern) {
    if (!watcher || !pattern) return false;
    
    char **new_patterns = realloc(watcher->config.exclude_patterns, 
        (watcher->config.exclude_pattern_count + 1) * sizeof(char*));
    if (!new_patterns) return false;
    
    watcher->config.exclude_patterns = new_patterns;
    watcher->config.exclude_patterns[watcher->config.exclude_pattern_count] = strdup(pattern);
    watcher->config.exclude_pattern_count++;
    
    return true;
}

// 添加包含模式
bool file_watcher_add_include_pattern(FileWatcher *watcher, const char *pattern) {
    if (!watcher || !pattern) return false;
    
    char **new_patterns = realloc(watcher->config.include_patterns, 
        (watcher->config.include_pattern_count + 1) * sizeof(char*));
    if (!new_patterns) return false;
    
    watcher->config.include_patterns = new_patterns;
    watcher->config.include_patterns[watcher->config.include_pattern_count] = strdup(pattern);
    watcher->config.include_pattern_count++;
    
    return true;
}

// 移除排除模式
bool file_watcher_remove_exclude_pattern(FileWatcher *watcher, const char *pattern) {
    if (!watcher || !pattern) return false;
    
    for (int i = 0; i < watcher->config.exclude_pattern_count; i++) {
        if (strcmp(watcher->config.exclude_patterns[i], pattern) == 0) {
            free(watcher->config.exclude_patterns[i]);
            
            // 移动后面的元素
            for (int j = i; j < watcher->config.exclude_pattern_count - 1; j++) {
                watcher->config.exclude_patterns[j] = watcher->config.exclude_patterns[j + 1];
            }
            
            watcher->config.exclude_pattern_count--;
            return true;
        }
    }
    
    return false;
}

// 移除包含模式
bool file_watcher_remove_include_pattern(FileWatcher *watcher, const char *pattern) {
    if (!watcher || !pattern) return false;
    
    for (int i = 0; i < watcher->config.include_pattern_count; i++) {
        if (strcmp(watcher->config.include_patterns[i], pattern) == 0) {
            free(watcher->config.include_patterns[i]);
            
            // 移动后面的元素
            for (int j = i; j < watcher->config.include_pattern_count - 1; j++) {
                watcher->config.include_patterns[j] = watcher->config.include_patterns[j + 1];
            }
            
            watcher->config.include_pattern_count--;
            return true;
        }
    }
    
    return false;
}

// 释放文件事件
void file_event_free(FileEvent *event) {
    if (event) {
        if (event->file_path) free(event->file_path);
        if (event->old_path) free(event->old_path);
        free(event);
    }
}

// 获取事件类型字符串
const char* file_event_type_to_string(FileEventType type) {
    switch (type) {
        case FILE_EVENT_CREATED: return "CREATED";
        case FILE_EVENT_DELETED: return "DELETED";
        case FILE_EVENT_MODIFIED: return "MODIFIED";
        case FILE_EVENT_RENAMED: return "RENAMED";
        case FILE_EVENT_ATTRIBUTES: return "ATTRIBUTES";
        case FILE_EVENT_SIZE: return "SIZE";
        case FILE_EVENT_ACCESS: return "ACCESS";
        case FILE_EVENT_UNKNOWN:
        default: return "UNKNOWN";
    }
}

// 处理监控事件（需要在主循环中调用）
void file_watcher_process_events(FileWatcher *watcher) {
    // 在Windows实现中，事件处理在单独的线程中进行
    // 这个函数主要用于其他平台的实现
    (void)watcher;  // 避免未使用参数警告
}
