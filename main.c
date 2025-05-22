// main.c
#include <SDL3/SDL.h>
#include "include/ui.h"
#include "include/log.h"
#include "include/resources.h"

int main(int argc, char* argv[]) {
    // 初始化 SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("无法初始化 SDL: %s", SDL_GetError());
        return 1;
    }

    // 初始化自定义资源（纹理、字体等）
    init_resources();

    // 创建主窗口（UI 层负责创建 renderer 和窗口）
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    if (!create_main_window(&window, &renderer)) {
        SDL_Log("无法创建主窗口\n");
        SDL_Quit();
        return 1;
    }

    // 主循环标志
    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
            handle_event(&event); // UI 或 Logic 模块处理事件
        }

        // 渲染逻辑
        SDL_SetRenderDrawColor(renderer, 255, 250, 240, 255); // 乳白色背景（FloralWhite）
        SDL_RenderClear(renderer);

        render_ui(renderer); // 渲染 UI 元素

        SDL_RenderPresent(renderer);
    }

    // 清理资源
    release_resources();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
