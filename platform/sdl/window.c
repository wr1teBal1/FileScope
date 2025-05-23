/*
 * SDL窗口管理模块
 * 职责：
 * 1. SDL窗口创建和管理
 * 2. 窗口事件处理
 * 3. 窗口样式设置
 * 4. 多显示器支持
 */

#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"
#include "studio.h"

#define SDL_FLAGS SDL_INIT_VIDEO
#define SDL_WINDOW_WIDTH 800
#define SDL_WINDOW_HEIGHT 600
#define SDL_WINDOW_TITLE "File Scope"

struct window {
    // SDL窗口
    SDL_Window *window;
    // SDL事件
    SDL_Event event;
};

bool window_init_sdl(struct window *w);
void window_free(struct window *w);


// SDL窗口初始化
bool window_init_sdl(struct window *w) {
    // 创建SDL窗口
    w->window = SDL_CreateWindow(SDL_WINDOW_TITLE,SDL_WINDOW_WIDTH, SDL_WINDOW_HEIGHT, 0);
    if (!w->window) {
        fprintf(stderr, "Unable to create window: %s\n", SDL_GetError());
        return false;
    }
    return true;
}
// 释放SDL资源
void window_free(struct window *w) {
    // 释放SDL窗口
    if (w->window) {
        SDL_DestroyWindow(w->window);
        w->window = NULL;
    }
    SDL_Quit();
}

