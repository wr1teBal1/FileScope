#ifndef SIDEBAR_H
#define SIDEBAR_H

#include "main.h"
#include "window.h"

// 前向声明
struct Window;

// 侧边栏结构体
typedef struct Sidebar {
    struct Window *app;     // 应用程序窗口
    SDL_Rect rect;          // 侧边栏区域
    // TODO: 添加侧边栏内容和状态
} Sidebar;

// 侧边栏函数声明
Sidebar* sidebar_new(struct Window *app);
void sidebar_free(Sidebar *sidebar);
bool sidebar_handle_event(Sidebar *sidebar, SDL_Event *event);
void sidebar_draw(Sidebar *sidebar);

#endif // SIDEBAR_H