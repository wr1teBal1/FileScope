#ifndef RENDERER_H
#define RENDERER_H

#include "window.h"

bool window_load_media(struct Window *window);
void window_clear(struct Window *window);
void window_draw(struct Window *window);
void window_present(struct Window *window);
bool ttf_show(struct Window *window,const char* str,SDL_Color color); 


#endif //RENDERER_H