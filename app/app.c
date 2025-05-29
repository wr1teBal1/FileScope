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
    if (!window || !main_window) {
        return;
    }
    
    // 主循环
    while (window->is_running) {
        // 处理SDL事件
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            // 处理退出事件
            if (event.type == SDL_EVENT_QUIT) {
                window->is_running = false;
                break;
            }
            
            // 处理ESC键退出
            if (event.type == SDL_EVENT_KEY_DOWN && 
                event.key.scancode == SDL_SCANCODE_ESCAPE) {
                window->is_running = false;
                break;
            }
            
            // 将事件传递给主窗口处理
            main_window_handle_event(main_window, &event);
        }
        
        // 绘制界面
        window_draw(window);
        main_window_draw(main_window);
        
        // 限制帧率
        SDL_Delay(16); // 约60FPS
    }
}

