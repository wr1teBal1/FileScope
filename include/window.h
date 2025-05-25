#ifndef WINDOW_H
#define WINDOW_H

#include "main.h"



 
struct Application {
    // SDL窗口
    SDL_Window *window;
    // SDL渲染器
    SDL_Renderer *renderer;
    // SDL字体
    TTF_Font * font;
    // SDL纹理
    SDL_Texture *background;
    // SDL文字纹理
    SDL_Texture *text_image ;
    // SDL事件
    SDL_Event event;
    // 是否关闭
    bool is_running;


};

bool app_new(struct Application **app);
void app_free(struct Application **app);
void app_run(struct Application *a);


#endif  WINDOW_H