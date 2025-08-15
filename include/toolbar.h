#ifndef TOOLBAR_H
#define TOOLBAR_H

#include "main.h"
#include "window.h"
#pragma once
// 工具栏样式常量
#define TOOLBAR_HEIGHT 40
#define BUTTON_PADDING 5
#define BUTTON_SIZE 30
#define BUTTON_SPACING 10

// 前向声明
struct Window;

// 按钮类型
typedef enum {
    BUTTON_BACK,        // 后退
    BUTTON_FORWARD,     // 前进
    BUTTON_UP,          // 上一级
    BUTTON_HOME,        // 主目录
    BUTTON_REFRESH,     // 刷新
    BUTTON_SEARCH,      // 搜索
    BUTTON_VIEW,        // 视图切换
    BUTTON_COPY,        // 复制 (新增)
    BUTTON_COUNT        // 按钮总数
} ToolbarButtonType;

// 按钮结构
typedef struct {
    ToolbarButtonType type;    // 按钮类型
    SDL_Rect rect;             // 按钮区域
    const char *tooltip;       // 提示文本
    bool enabled;              // 是否启用
    bool hovered;              // 是否悬停
    bool pressed;              // 是否按下
} ToolbarButton;

// 工具栏结构体
typedef struct Toolbar {
    struct Window *app;        // 应用程序窗口
    SDL_Rect rect;             // 工具栏区域
    ToolbarButton buttons[BUTTON_COUNT];  // 按钮数组
    int button_count;          // 按钮数量
    
    // 历史记录
    char **history;            // 历史路径数组
    int history_capacity;      // 历史记录容量
    int history_count;         // 历史记录数量
    int history_index;         // 当前历史记录索引

    // 搜索相关
    char search_text[256];     // 搜索输入内容
    bool search_active;        // 是否处于搜索输入状态
    int search_cursor_pos;     // 光标位置
    
    // 地址栏相关
    char address_bar_text[1024];  // 地址栏输入内容
    bool address_bar_active;      // 是否处于地址栏输入状态
    int address_bar_cursor_pos;   // 地址栏光标位置
    bool address_bar_editing;     // 是否正在编辑地址栏
} Toolbar;

// 工具栏基本函数声明
Toolbar* toolbar_new(struct Window *app);// 创建工具栏
void toolbar_free(Toolbar *toolbar);  // 释放工具栏
bool toolbar_handle_event(Toolbar *toolbar, SDL_Event *event); // 处理工具栏事件
void toolbar_draw(Toolbar *toolbar);                          // 绘制工具栏

// 通知工具栏目录已更改
void toolbar_notify_directory_changed(Toolbar *toolbar, const char *path);

// 历史导航函数
bool toolbar_go_back(Toolbar *toolbar);
bool toolbar_go_forward(Toolbar *toolbar);
bool toolbar_go_up(Toolbar *toolbar);
bool toolbar_go_home(Toolbar *toolbar);

// 按钮操作函数
bool toolbar_refresh(Toolbar *toolbar);
bool toolbar_toggle_view(Toolbar *toolbar);
bool toolbar_search(Toolbar *toolbar, const char *search_term);

// 设置按钮状态
void toolbar_set_button_enabled(Toolbar *toolbar, ToolbarButtonType button_type, bool enabled);
void toolbar_update_button_states(Toolbar *toolbar);

// 搜索相关API
void toolbar_search_start(Toolbar *toolbar);
void toolbar_search_stop(Toolbar *toolbar);
void toolbar_search_handle_text(Toolbar *toolbar, const char *text);
void toolbar_search_handle_key(Toolbar *toolbar, SDL_Scancode scancode);

// 地址栏相关API   （暂时不实现）
void toolbar_address_bar_start(Toolbar *toolbar);
void toolbar_address_bar_stop(Toolbar *toolbar);
void toolbar_address_bar_handle_text(Toolbar *toolbar, const char *text);
void toolbar_address_bar_handle_key(Toolbar *toolbar, SDL_Scancode scancode);
void toolbar_address_bar_navigate(Toolbar *toolbar, const char *path);

#endif // TOOLBAR_H