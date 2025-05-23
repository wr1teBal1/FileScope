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


#define SDL_FLAGS SDL_INIT_VIDEO
#define SDL_WINDOW_WIDTH 800
#define SDL_WINDOW_HEIGHT 600
#define SDL_WINDOW_TITLE "File Scope"

struct Application {
    // SDL窗口
    SDL_Window *window;
    // SDL渲染器
    SDL_Renderer *renderer;
    // SDL纹理
    SDL_Texture *texture;
    // SDL事件
    SDL_Event event;
    // 是否关闭
    bool is_running;


};

bool app_init_sdl(struct Application *a);
void app_free(struct Application **app);
void app_run(struct Application *a);
void app_events(struct Application *a);
void app_draw(struct Application *a);
bool app_new(struct Application **app);

// SDL初始化
bool app_init_sdl(struct Application *a){
    // 初始化SDL
    if (SDL_Init(SDL_FLAGS) < 0) {
        fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
        return false;
    }
    // 创建SDL窗口
    a->window = SDL_CreateWindow(SDL_WINDOW_TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOW_WIDTH, SDL_WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!a->window) {
        fprintf(stderr, "Unable to create window: %s\n", SDL_GetError());
        return false;
    }
    // 创建SDL渲染器
    a->renderer = SDL_CreateRenderer(a->window, -1, 0);
    if (!a->renderer) {
        fprintf(stderr, "Unable to create renderer: %s\n", SDL_GetError());
        return false;
    }
    // 创建SDL纹理
    a->texture = SDL_CreateTexture(a->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, SDL_WINDOW_WIDTH, SDL_WINDOW_HEIGHT);
    if (!a->texture) {
        fprintf(stderr, "Unable to create texture: %s\n", SDL_GetError());
        return false;
    }

    return true;
}

// 释放SDL资源
void app_free(struct Application **app) {
    if (*app){
        struct Application *a = *app;
        // 释放SDL纹理
        if (a->texture) {
            SDL_DestroyTexture(a->texture);
            a->texture = NULL;
        }
        // 释放SDL渲染器
        if (a->renderer) {
            SDL_DestroyRenderer(a->renderer);
            a->renderer = NULL;
        }
        // 释放SDL窗口
        if (a->window) {
            SDL_DestroyWindow(app->window);
            a->window = NULL;
        }
        SDL_Quit();

        free(a);

        a = NULL;
        *app = NULL;

        printf("ALL CLEAN!!\n");

    }
}

bool app_new(struct Application **app) {
    *app = calloc(1,sizeof(struct Application));//

    if (*app == NULL)
    {
        fprintf(stderr,"ERROR calloc of new app\n");
        return false;
    }
    struct Application *a = *app;

    // 初始化应用程序
    if (!app_init_sdl(a))
    {
        return false;
    }
    
    a->is_running = t rue;
    a->window = NULL;
    a->renderer = NULL;
    a->texture = NULL;

    return true;
}

//run
void app_run(struct Application *a)
{
    while (a->is_running){
        // 处理事件
        app_events(app);
        // 绘制
        app_draw(app);
        // 延时
        SDL_Delay(16);

    }

}
