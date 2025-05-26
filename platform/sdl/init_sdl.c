#include "init_sdl.h"


// SDL初始化
bool window_init_sdl(struct Window *a){
    // 初始化SDL
    if (SDL_Init(SDL_FLAGS) < 0) {
        fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
        return false;
    }
    // 初始化SDL文字
    if (! TTF_Init()) {
        fprintf(stderr, "Unable to initialize SDL_ttf: %s\n", SDL_GetError());
        return false; 
    }
    // 创建SDL窗口
    a->window = SDL_CreateWindow(SDL_WINDOW_TITLE, SDL_WINDOW_WIDTH, SDL_WINDOW_HEIGHT, 0);
    if (!a->window) {
        fprintf(stderr, "Unable to create window: %s\n", SDL_GetError());
        return false;
    }
    // 创建SDL渲染器
    a->renderer = SDL_CreateRenderer(a->window, NULL);
     //创建渲染器
    if (!a->renderer) {
        fprintf(stderr, "Unable to create renderer: %s\n", SDL_GetError());
        return false;
    }

    //软件图标
    SDL_Surface *icon = IMG_Load("images/icon.png"); //贴图1
    if (!icon) {
        fprintf(stderr, "Unable to load icon: %s\n", SDL_GetError());
        return false;
    }  
    if (!SDL_SetWindowIcon(a->window, icon)) {
        fprintf(stderr, "Unable to set window icon: %s\n", SDL_GetError());
        SDL_DESTROY_SURFACE(icon);
        return false;
    } 
    SDL_DESTROY_SURFACE(icon);
 
    return true;
}
