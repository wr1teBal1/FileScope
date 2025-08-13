/*
 * 应用程序核心功能模块
 * 职责：
 * 1. 应用程序生命周期管理
 * 2. 协调各个模块的工作
 * 3. 处理核心业务逻辑
 */

#include "main.h"
#include "window.h"
#include "main_window.h"
#include "event.h"
#include "renderer.h"
#include "file_monitor.h"

// 应用程序主循环
void app_run(struct Window *window, MainWindow *main_window) {
    printf("[DEBUG] app_run called\n");
    
    if (!window) {
        printf("[ERROR] app_run: Window is NULL\n");
        return;
    }
    
    if (!main_window) {
        printf("[ERROR] app_run: MainWindow is NULL\n");
        return;
    }
    
    printf("[DEBUG] Starting main loop, window->is_running = %s\n", window->is_running ? "true" : "false");
    
    // 创建并启动文件监控
    FileMonitor *file_monitor = file_monitor_create();
    if (file_monitor) {
        // 获取当前目录并开始监控
        char *current_dir = fs_get_current_directory();
        if (current_dir) {
            printf("[DEBUG] 初始化文件监控，监控目录: %s\n", current_dir);
            if (file_monitor_start(file_monitor, current_dir)) {
                printf("[DEBUG] 文件监控初始化成功\n");
            } else {
                printf("[WARNING] 文件监控初始化失败\n");
            }
            free(current_dir);
        }
    } else {
        printf("[WARNING] 无法创建文件监控管理器\n");
    }
    
    // 主循环
    while (window->is_running) {
        
        // 处理SDL事件
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            // 处理退出事件
            if (event.type == SDL_EVENT_QUIT) {
                printf("[DEBUG] Quit event received\n");
                window->is_running = false;
                break;
            }
            
            // 处理ESC键退出
            if (event.type == SDL_EVENT_KEY_DOWN && 
                event.key.scancode == SDL_SCANCODE_ESCAPE) {
                printf("[DEBUG] ESC key pressed\n");
                window->is_running = false;
                break;
            }
            
            // 处理窗口事件
            if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                printf("[DEBUG] Window resized to %dx%d\n", 
                       event.window.data1, event.window.data2);
                window->config.window_width = event.window.data1;
                window->config.window_height = event.window.data2;
                
                // 调用主窗口大小调整函数
                main_window_resize(main_window, event.window.data1, event.window.data2);
            } else if (event.type == SDL_EVENT_WINDOW_MOVED) {
                printf("[DEBUG] Window moved to (%d,%d)\n", 
                       event.window.data1, event.window.data2);
                window->config.window_x = event.window.data1;
                window->config.window_y = event.window.data2;
            } else if (event.type == SDL_EVENT_WINDOW_MAXIMIZED) {
                printf("[DEBUG] Window maximized\n");
                window->config.window_maximized = true;
            } else if (event.type == SDL_EVENT_WINDOW_RESTORED) {
                printf("[DEBUG] Window restored\n");
                window->config.window_maximized = false;
            }
            
            // 将事件传递给主窗口处理
            main_window_handle_event(main_window, &event);
        }
        
        // 处理文件监控事件
        if (file_monitor) {
            file_monitor_process_events(file_monitor);
        }
        
        // 绘制界面
        window_clear(window);           // 清除渲染器
        window_draw(window);            // 绘制窗口背景内容
        main_window_draw(main_window);  // 绘制主窗口内容
        window_present(window);         // 呈现渲染结果
        
        // 限制帧率
        SDL_Delay(16); // 约60FPS
    }
    
    // 清理文件监控资源
    if (file_monitor) {
        file_monitor_destroy(file_monitor);
        printf("[DEBUG] 文件监控已清理\n");
    }
}

