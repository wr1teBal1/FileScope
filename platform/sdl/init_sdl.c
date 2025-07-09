#include "init_sdl.h"
#include "config.h"


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
    
    // 加载配置
    if (!config_load(&a->config)) {
        fprintf(stderr, "Failed to load config, using defaults\n");
    }
    
    // 创建SDL窗口，设置为可调整大小
    a->window = SDL_CreateWindow(SDL_WINDOW_TITLE, 
                                a->config.window_width, 
                                a->config.window_height, 
                                SDL_WINDOW_RESIZABLE);
    if (!a->window) {
        fprintf(stderr, "Unable to create window: %s\n", SDL_GetError());
        return false;
    }
    
    // 设置窗口位置
    if (a->config.window_x != SDL_WINDOWPOS_CENTERED && 
        a->config.window_y != SDL_WINDOWPOS_CENTERED) {
        SDL_SetWindowPosition(a->window, a->config.window_x, a->config.window_y);
    }
    
    // 如果之前是最大化状态，则最大化窗口
    if (a->config.window_maximized) {
        SDL_MaximizeWindow(a->window);
    }
    
    // 初始化调整大小状态
    a->is_resizing = false;
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
        // SDL_DestroySurface(icon);
        return false;
    } 
    // SDL_DestroySurface(icon);
 
    return true;
}
