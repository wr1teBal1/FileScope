#ifndef WINDOW_H
#define WINDOW_H

#include "main.h"



 
struct Window {
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

bool window_new(struct Window **window);
void window_free(struct Window **window);
void window_run(struct Window *w);


#endif  //WINDOW_H