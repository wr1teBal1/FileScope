#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "main.h"
#include "window.h"
#include "file_list.h"

// 前向声明
struct Window;
struct FileListView;
struct Toolbar;
struct Sidebar;

// 主窗口结构体
typedef struct MainWindow {
    struct Window *app;                    // 应用程序窗口
    struct FileListView *file_list_view;   // 文件列表视图
    struct Toolbar *toolbar;               // 工具栏
    struct Sidebar *sidebar;               // 侧边栏
} MainWindow;

// 主窗口函数声明
MainWindow* main_window_new(struct Window *app);
void main_window_free(MainWindow *window);
bool main_window_handle_event(MainWindow *window, SDL_Event *event);
void main_window_draw(MainWindow *window);

#endif // MAIN_WINDOW_H