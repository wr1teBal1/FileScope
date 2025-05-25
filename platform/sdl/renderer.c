/*
 * SDL渲染器模块
 * 职责：
 * 1. SDL渲染器初始化
 * 2. 基础图形渲染
 * 3. 纹理管理
 * 4. 渲染状态管理
 */
#include "SDL3/SDL.h"



void app_draw(struct Application *a) {
    // 清除渲染器
     SDL_RenderClear(a->renderer);

    // 绘制纹理
    SDL_RenderTexture (a->renderer, a->background , NULL, NULL);

    // 显示渲染结果
    SDL_RenderPresent(a->renderer);
}

bool app_load_texture(struct Application *a) {
    // 加载纹理
    a->background = IMG_LoadTexture(a->renderer, "images/事例.png");//贴图2 
    if (!a->background) { 
        fprintf(stderr, "Unable to load texture: %s\n", SDL_GetError());
        return false;
    }
    return true;  
}