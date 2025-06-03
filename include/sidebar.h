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
// 侧边栏状态枚举
typedef enum SidebarState {
    SIDEBAR_STATE_NORMAL,//  侧边栏处于正常状态
    SIDEBAR_STATE_FOCUSED,// 侧边栏处于焦点状态       
    SIDEBAR_STATE_HOVER,// 侧边栏处于悬停状态
    SIDEBAR_STATE_ACTIVE,// 侧边栏处于活动状态
    SIDEBAR_STATE_DISABLED// 侧边栏禁用状态
    } SidebarState;
    //按钮结构体
    typedef struct Button {
    SDL_Rect rect;          // 按钮区域
    char *text;             // 按钮文本
    SDL_Color text_color;   // 按钮文本颜色
    SDL_Color bg_color;    // 按钮背景颜色
    SDL_Color border_color; // 按钮边框颜色
    int border_width;      // 按钮边框宽度
    int padding;           // 按钮内边距
    int state;             // 按钮状态
    void (*on_click)(struct Button *button);    
    // TODO: 添加按钮点击事件回调函数
    } Button;

// 侧边栏函数声明
Sidebar* sidebar_new(struct Window *app);// 创建侧边栏
void sidebar_free(Sidebar *sidebar);// 销毁侧边栏
bool sidebar_handle_event(Sidebar *sidebar, SDL_Event *event);// 处理侧边栏事件
void sidebar_draw(Sidebar *sidebar);// 绘制侧边栏

#endif // SIDEBAR_H