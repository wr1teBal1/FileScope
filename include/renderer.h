#ifndef RENDERER_H
#define RENDERER_H

#include "window.h"

bool app_load_media(struct Application *a);
void app_draw(struct Application *a);
bool ttf_show(struct Application *a,string str,SDL_Color color); 


#endif