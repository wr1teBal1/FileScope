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
void window_free(struct Window **app) {
    if (*app){
        struct Window *a = *app;
        // 释放SDL文字纹理
        if (a->text_image) {
            SDL_DestroyTexture(a->text_image);
            a->text_image = NULL;
        }  
        // 释放SDL字体
        if (a->font) {
            TTF_CloseFont(a->font);
            a->font = NULL;
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
            SDL_DestroyWindow(a->window);
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

bool window_new(struct Window **app) {
    *app = calloc(1,sizeof(struct Window));//

    if (*app == NULL)
    {
        fprintf(stderr,"ERROR calloc of new app\n");
        return false;
    }
    struct Window *a = *app;

    // 初始化应用程序
    if (!window_init_sdl(a)){
        return false;
    }
    //
    if (!window_load_media(a)){
        return false;
    }

    a->is_running = true;


    return true;
}

void window_run(struct Window *a)
{
    // 这个函数现在只是一个占位符，实际的主循环已移至app.c
    // 保留此函数以维持向后兼容性
    // 实际应用中应使用app.c中的app_run函数

}
