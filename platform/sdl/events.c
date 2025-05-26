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

            default:
                break;
        }
    }
}