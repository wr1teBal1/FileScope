/*
 * SDL窗口管理模块
 * 职责：
 * 1. SDL窗口创建和管理
 * 2. 窗口事件处理
 * 3. 窗口样式设置
 * 4. 多显示器支持
 */

#include "window.h"
#include "init_sdl.h"
#include "renderer.h"
 



// 释放SDL资源
void app_free(struct Application **app) {
    if (*app){
        struct Application *a = *app;
        // 释放SDL字体
        if (a->font) {
            TTF_CloseFont(a->font);
            a->font = NULL;
        }
        // 释放SDL事件
        if (a->event) {
            SDL_DestroyEvent(&a->event);
            a->event = NULL;
        } 
        // 释放SDL纹理
        if (a->background) {
            SDL_DestroyTexture(a->background);
            a->background = NULL;
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
        TTF_Quit();
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
    if (!app_init_sdl(a)){
        return false;
    }
    //
    if (!app_load_texture(a)){
        return false;
    }

    a->is_running = true;


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
