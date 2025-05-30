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
            
            // 将事件传递给主窗口处理
            main_window_handle_event(main_window, &event);
        }
        
        // 绘制界面
        window_clear(window);           // 清除渲染器
        window_draw(window);            // 绘制窗口背景内容
        main_window_draw(main_window);  // 绘制主窗口内容
        window_present(window);         // 呈现渲染结果
        
        // 限制帧率
        SDL_Delay(16); // 约60FPS
    }
}

