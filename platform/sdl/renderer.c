/*
 * SDL渲染器模块
 * 职责：
 * 1. SDL渲染器初始化
 * 2. 基础图形渲染
 * 3. 纹理管理
 * 4. 渲染状态管理
 */
#include "renderer.h"



void window_draw(struct Window *a) {
    // 清除渲染器
     SDL_RenderClear(a->renderer);

    // 绘制纹理
    SDL_RenderTexture (a->renderer, a->background , NULL, NULL);
    // 绘制文本
    SDL_RenderTexture(a->renderer, a->text_image, NULL, NULL);

    // 显示渲染结果
    SDL_RenderPresent(a->renderer);
}

bool window_load_media(struct Window *a) {
    // 加载纹理 
    a->background = IMG_LoadTexture(a->renderer, "images/事例.png");//贴图2 
    if (!a->background) { 
        fprintf(stderr, "Unable to load texture: %s\n", SDL_GetError());
        return false;
    }
    //加载字体
    a->font = TTF_OpenFont("fonts/msyh.ttf", TEXT_SIZE); 
    if (!a->font) {
        fprintf(stderr, "Unable to load font: %s\n", SDL_GetError()); 
        return false;
    } 
    return true;  
}
 
bool ttf_show(struct Window *a,const char* str,SDL_Color color){
    SDL_Surface *surf = TTF_RenderText_Blended(a->font, str,0, color);
    if (!surf) {
        fprintf(stderr, "Unable to create surface: %s\n", TTF_GetError());
        return false;
    }
    a->text_image = SDL_CreateTextureFromSurface(a->renderer, surf);
    SDL_DESTROY_SURFACE(surf); 
    surf = NULL;
    if (!a->text_image) {
        fprintf(stderr, "Unable to  create texture: %s\n", SDL_GetError());
        return false;
    }
    return true;  
}