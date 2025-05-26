/*
 * 应用程序核心功能模块
 * 职责：
 * 1. 应用程序生命周期管理
 * 2. 协调各个模块的工作
 * 3. 处理核心业务逻辑
 */

#include "../platform/sdl/window.h"
#include <stdio.h>
#include <stdlib.h>

// 应用程序结构体
struct app_context {
    struct application *window;  // SDL窗口和渲染器
    char current_path[1024];    // 当前浏览的路径
    bool initialized;           // 初始化状态
};

// 创建应用程序上下文
struct app_context* app_create() {
    struct app_context *ctx = (struct app_context*)malloc(sizeof(struct app_context));
    if (!ctx) {
        return NULL;
    }
    
    // 初始化上下文
    ctx->window = (struct application*)malloc(sizeof(struct application));
    if (!ctx->window) {
        free(ctx);
        return NULL;
    }
    
    // 初始化SDL窗口
    if (!app_init_sdl(ctx->window)) {
        free(ctx->window);
        free(ctx);
        return NULL;
    }

    ctx->initialized = true;
    strcpy(ctx->current_path, ".");  // 默认从当前目录开始
    return ctx;
}

// 运行应用程序
void app_run(struct app_context *ctx) {
    if (!ctx || !ctx->initialized) {
        return;
    }

    ctx->window->running = true;
    
    // 主循环已经在window.c中实现，这里直接调用
    while (ctx->window->running) {
        // 处理事件和渲染
        app_handle_events(ctx);
        app_render(ctx);
    }
}

// 处理应用程序事件
void app_handle_events(struct app_context *ctx) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                ctx->window->running = false;
                break;
            case SDL_MOUSEBUTTONDOWN:
                // 处理鼠标点击，如文件选择等
                break;
            case SDL_KEYDOWN:
                // 处理键盘事件，如导航等
                break;
        }
    }
}

// 渲染应用程序界面
void app_render(struct app_context *ctx) {
    SDL_SetRenderDrawColor(ctx->window->renderer, 255, 255, 255, 255);
    SDL_RenderClear(ctx->window->renderer);

    // TODO: 渲染文件列表
    // TODO: 渲染工具栏
    // TODO: 渲染侧边栏

    SDL_RenderPresent(ctx->window->renderer);
}

// 清理应用程序资源
void app_destroy(struct app_context *ctx) {
    if (!ctx) {
        return;
    }

    if (ctx->window) {
        app_free(ctx->window);
        free(ctx->window);
    }

    free(ctx);
} 