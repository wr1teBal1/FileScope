/*
 * SDL事件处理模块
 * 职责：
 * 1. SDL事件循环
 * 2. 输入事件处理
 * 3. 窗口事件处理
 * 4. 事件分发系统
 */

#include "event.h"



void window_events(struct Window *w) {
        // 处理事件
    while (SDL_PollEvent(&w->event)) {
        switch (w->event.type) {
            case SDL_EVENT_QUIT:
                w->is_running = false;
                break;
            case SDL_EVENT_KEY_DOWN:
                switch (w->event.key.scancode) {
                    case SDL_SCANCODE_ESCAPE:
                        w->is_running = false;
                        break;
                    default: 
                        break;
                }
                break;
            case SDL_EVENT_WINDOW_RESIZED:
                // 窗口大小改变事件
                printf("[DEBUG] Window resized to %dx%d\n", 
                       w->event.window.data1, w->event.window.data2);
                // 更新配置中的窗口大小
                w->config.window_width = w->event.window.data1;
                w->config.window_height = w->event.window.data2;
                break;
            case SDL_EVENT_WINDOW_MOVED:
                // 窗口位置改变事件
                printf("[DEBUG] Window moved to (%d,%d)\n", 
                       w->event.window.data1, w->event.window.data2);
                // 更新配置中的窗口位置
                w->config.window_x = w->event.window.data1;
                w->config.window_y = w->event.window.data2;
                break;
            case SDL_EVENT_WINDOW_MAXIMIZED:
                // 窗口最大化事件
                printf("[DEBUG] Window maximized\n");
                w->config.window_maximized = true;
                break;
            case SDL_EVENT_WINDOW_RESTORED:
                // 窗口恢复事件
                printf("[DEBUG] Window restored\n");
                w->config.window_maximized = false;
                break;
            default:
                break;
        }
    }
}