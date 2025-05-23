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
    SDL_SetRenderDrawColor(a->renderer, 0, 0, 0, 255);
    SDL_RenderClear(a->renderer);

    // 绘制纹理
    SDL_RenderCopy(a->renderer, a->texture, NULL, NULL);

    // 显示渲染结果
    SDL_RenderPresent(a->renderer);
}