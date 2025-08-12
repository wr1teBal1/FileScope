/*
 * 文件监控模块使用示例
 * 演示如何：
 * 1. 创建文件监控器
 * 2. 设置回调函数
 * 3. 启动和停止监控
 * 4. 处理文件变化事件
 */

#include "../include/file_watcher.h"
#include "../include/file_system.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef _WIN32
#include <windows.h>
#endif

// 文件变化事件回调函数
void on_file_changed(const FileEvent *event, void *user_data) {
    printf("文件变化事件: %s\n", file_event_type_to_string(event->type));
    printf("  文件路径: %s\n", event->file_path ? event->file_path : "未知");
    if (event->old_path) {
        printf("  旧路径: %s\n", event->old_path);
    }
    printf("  是否为目录: %s\n", event->is_directory ? "是" : "否");
    printf("  是否为隐藏文件: %s\n", event->is_hidden ? "是" : "否");
    printf("  文件大小: %zu 字节\n", event->file_size);
    printf("  时间戳: %ld\n", event->timestamp);
    printf("  --------------------\n");
}

// 创建测试文件
void create_test_file(const char *directory, const char *filename) {
    char filepath[1024];
    snprintf(filepath, sizeof(filepath), "%s/%s", directory, filename);
    
    FILE *file = fopen(filepath, "w");
    if (file) {
        fprintf(file, "这是一个测试文件，创建时间: %ld\n", time(NULL));
        fclose(file);
        printf("创建测试文件: %s\n", filepath);
    } else {
        printf("无法创建测试文件: %s\n", filepath);
    }
}

// 删除测试文件
void delete_test_file(const char *directory, const char *filename) {
    char filepath[1024];
    snprintf(filepath, sizeof(filepath), "%s/%s", directory, filename);
    
    if (remove(filepath) == 0) {
        printf("删除测试文件: %s\n", filepath);
    } else {
        printf("无法删除测试文件: %s\n", filepath);
    }
}

// 修改测试文件
void modify_test_file(const char *directory, const char *filename) {
    char filepath[1024];
    snprintf(filepath, sizeof(filepath), "%s/%s", directory, filename);
    
    FILE *file = fopen(filepath, "a");
    if (file) {
        fprintf(file, "文件被修改，修改时间: %ld\n", time(NULL));
        fclose(file);
        printf("修改测试文件: %s\n", filepath);
    } else {
        printf("无法修改测试文件: %s\n", filepath);
    }
}

// 重命名测试文件
void rename_test_file(const char *directory, const char *oldname, const char *newname) {
    char oldpath[1024], newpath[1024];
    snprintf(oldpath, sizeof(oldpath), "%s/%s", directory, oldname);
    snprintf(newpath, sizeof(newpath), "%s/%s", directory, newname);
    
    if (rename(oldpath, newpath) == 0) {
        printf("重命名文件: %s -> %s\n", oldpath, newpath);
    } else {
        printf("无法重命名文件: %s -> %s\n", oldpath, newpath);
    }
}

int main(int argc, char *argv[]) {
    // 获取要监控的目录
    char *directory = ".";
    if (argc > 1) {
        directory = argv[1];
    }
    
    printf("文件监控示例\n");
    printf("监控目录: %s\n", directory);
    printf("按 Ctrl+C 退出\n\n");
    
    // 创建文件监控配置
    FileWatcherConfig config = file_watcher_get_default_config();
    config.enabled = true;
    config.recursive = true;
    config.watch_attributes = true;
    config.watch_size = true;
    config.watch_access = false;
    config.debounce_ms = 100;  // 100ms防抖
    
    // 添加一些排除模式
    file_watcher_add_exclude_pattern(NULL, "*.tmp");
    file_watcher_add_exclude_pattern(NULL, "*.log");
    file_watcher_add_exclude_pattern(NULL, ".git");
    
    // 创建文件监控器
    FileWatcher *watcher = file_watcher_create(directory, &config);
    if (!watcher) {
        printf("无法创建文件监控器\n");
        return 1;
    }
    
    // 设置回调函数
    file_watcher_set_callback(watcher, on_file_changed, NULL);
    
    // 启动监控
    if (!file_watcher_start(watcher)) {
        printf("无法启动文件监控\n");
        file_watcher_destroy(watcher);
        return 1;
    }
    
    printf("文件监控已启动\n");
    printf("监控器状态: %s\n", file_watcher_is_running(watcher) ? "运行中" : "已停止");
    printf("监控目录: %s\n", file_watcher_get_directory(watcher));
    printf("\n");
    
    // 执行一些测试操作
    printf("执行测试操作...\n");
    
    // 创建测试文件
    create_test_file(directory, "test_file_1.txt");
    sleep(1);
    
    // 修改测试文件
    modify_test_file(directory, "test_file_1.txt");
    sleep(1);
    
    // 创建另一个测试文件
    create_test_file(directory, "test_file_2.txt");
    sleep(1);
    
    // 重命名文件
    rename_test_file(directory, "test_file_1.txt", "test_file_renamed.txt");
    sleep(1);
    
    // 删除文件
    delete_test_file(directory, "test_file_2.txt");
    sleep(1);
    
    printf("\n测试操作完成，继续监控中...\n");
    printf("按 Ctrl+C 退出\n\n");
    
    // 主循环 - 保持程序运行
    while (1) {
        // 处理文件监控事件
        file_watcher_process_events(watcher);
        
        // 短暂休眠
#ifdef _WIN32
        Sleep(100);
#else
        usleep(100000);  // 100ms
#endif
    }
    
    // 清理资源
    file_watcher_stop(watcher);
    file_watcher_destroy(watcher);
    
    return 0;
} 