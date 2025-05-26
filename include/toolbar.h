#ifndef TOOLBAR_H
#define TOOLBAR_H

#include "main.h"
#include "window.h"

// 前向声明
struct Window;

// 工具栏结构体
typedef struct Toolbar {
    struct Window *app;     // 应用程序窗口
    SDL_Rect rect;          // 工具栏区域
    // TODO: 添加工具栏按钮和状态
} Toolbar;

// 工具栏函数声明
Toolbar* toolbar_new(struct Window *app);
void toolbar_free(Toolbar *toolbar);
bool toolbar_handle_event(Toolbar *toolbar, SDL_Event *event);
void toolbar_draw(Toolbar *toolbar);

#endif // TOOLBAR_H