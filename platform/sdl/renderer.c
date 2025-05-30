/*
 * SDL渲染器模块
 * 职责：
 * 1. SDL渲染器初始化
 * 2. 基础图形渲染
 * 3. 纹理管理
 * 4. 渲染状态管理
 */
#include "renderer.h"



void window_clear(struct Window *a) {
    if (!a || !a->renderer) {
        return;
    }
    
    // 设置背景色
    SDL_SetRenderDrawColor(a->renderer, 240, 240, 240, 255); // 浅灰色背景
    // 清除渲染器
    SDL_RenderClear(a->renderer);
}

void window_present(struct Window *a) {
    if (!a || !a->renderer) {
        return;
    }
    
    // 显示渲染结果
    SDL_RenderPresent(a->renderer);
}

void window_draw(struct Window *a) {
    if (!a || !a->renderer) {
        return;
    }
    
    // 只绘制窗口特有的内容，不清除和呈现
    // 只有当背景纹理存在时才绘制
    if (a->background) {
        SDL_RenderTexture(a->renderer, a->background, NULL, NULL);
    }
    
    // 绘制文本
    if (a->text_image) {
        SDL_RenderTexture(a->renderer, a->text_image, NULL, NULL);
    }
}

bool window_load_media(struct Window *a) {
    // 暂时注释掉背景图片加载
    // a->background = IMG_LoadTexture(a->renderer, "images/事例.png");
    // if (!a->background) { 
    //     fprintf(stderr, "Unable to load texture: %s\n", SDL_GetError());
    //     return false;
    // }
    
    // 设置背景为NULL，后续可以用纯色背景
    a->background = NULL;
    
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
        fprintf(stderr, "Unable to create surface: %s\n", SDL_GetError());
        return false;
    }
    a->text_image = SDL_CreateTextureFromSurface(a->renderer, surf);
    // SDL_DestroySurface(surf); 
    surf = NULL;
    if (!a->text_image) {
        fprintf(stderr, "Unable to  create texture: %s\n", SDL_GetError());
        return false;
    }
    return true;  
}