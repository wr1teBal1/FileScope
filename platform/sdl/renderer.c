/*
 * SDL渲染器模块
 * 职责：
 * 1. SDL渲染器初始化
 * 2. 基础图形渲染
 * 3. 纹理管理
 * 4. 渲染状态管理
 */
#include "SDL3/SDL.h"


struct renderer {
    // SDL渲染器
    SDL_Renderer *renderer;
    // SDL纹理
    SDL_Texture *texture;
};

bool renderer_init_sdl(struct renderer *r);
void renderer_free(struct renderer *r);

// SDL渲染器初始化
bool renderer_init_sdl(struct renderer *r) {
    // 创建SDL渲染器
    r->renderer = SDL_CreateRenderer(r->renderer, NULL);
    if (!r->renderer) {
        fprintf(stderr, "Unable to create renderer: %s\n", SDL_GetError());
        return false;
    }
    // 创建SDL纹理
    //广告位招租
    return true;
}

// 释放SDL资源
void renderer_free(struct renderer *r) {
    // 释放SDL纹理
    if (r->texture) {
        SDL_DestroyTexture(r->texture);
        r->texture = NULL;
    }
    // 释放SDL渲染器
    if (r->renderer) {
        SDL_DestroyRenderer(r->renderer);
        r->renderer = NULL;
    }
    SDL_Quit();
}