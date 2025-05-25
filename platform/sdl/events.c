/*
 * SDL事件处理模块
 * 职责：
 * 1. SDL事件循环
 * 2. 输入事件处理
 * 3. 窗口事件处理
 * 4. 事件分发系统
 */

#include "SDL3/SDL.h"



void app_events(struct Application *a) {
        // 处理事件
    while (SDL_PollEvent(&a->event)) {
        switch (a->event.type) {
            case SDL_EVENT_QUIT:
                a->is_running = false;
                break;
            case SDL_EVENT_KEY_DOWN:
                switch (a->event.key.scancode) {
                    case SDL_SCANCODE_ESCAPE:
                        a->is_running = false;
                        break;
                    default: 
                        break;
                }
                break;
            case SDL_EVENT_WINDOW_CLOSE:
                a->is_running = false;
                break;

            default:
                break;
        }
    }
}