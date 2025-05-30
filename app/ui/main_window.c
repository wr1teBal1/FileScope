/*
 * 主窗口模块
 * 职责：
 * 1. 创建和管理主窗口
 * 2. 包含文件列表视图、侧边栏、工具栏等UI组件
 * 3. 处理窗口事件
 */

#include "main_window.h"
#include "file_list.h"
#include "toolbar.h"
#include "sidebar.h"
#include "renderer.h"
#include <stdlib.h>

// 创建主窗口
MainWindow* main_window_new(Window *a) {
    if (!a) {
        return NULL;
    }

    MainWindow *window = (MainWindow*)calloc(1, sizeof(MainWindow));
    if (!window) {
        return NULL;
    }

    window->app = a;

    // 创建文件列表视图
    window->file_list_view = file_list_view_new(a);
    if (!window->file_list_view) {
        free(window);
        return NULL;
    }

    // TODO: 创建工具栏和侧边栏
    // window->toolbar = toolbar_new(app);
    // window->sidebar = sidebar_new(app);

    // 设置文件列表视图的视口区域
    window->file_list_view->viewport.x = 0;
    window->file_list_view->viewport.y = 0; // TODO: 考虑工具栏高度
    window->file_list_view->viewport.w = SDL_WINDOW_WIDTH; // TODO: 考虑侧边栏宽度
    window->file_list_view->viewport.h = SDL_WINDOW_HEIGHT; // TODO: 考虑工具栏高度

    // 加载默认目录
    file_list_view_load_directory(window->file_list_view, "."); // TODO: 加载用户主目录或上次打开的目录

    return window;
}

// 释放主窗口
void main_window_free(MainWindow *window) {
    if (!window) {
        return;
    }

    // 释放UI组件
    if (window->file_list_view) {
        file_list_view_free(window->file_list_view);
    }
    // TODO: 释放工具栏和侧边栏
    // if (window->toolbar) { toolbar_free(window->toolbar); }
    // if (window->sidebar) { sidebar_free(window->sidebar); }

    free(window);
}

// 处理主窗口事件
bool main_window_handle_event(MainWindow *window, SDL_Event *event) {
    if (!window || !event) {
        return false;
    }

    // 将事件传递给文件列表视图处理
    if (file_list_view_handle_event(window->file_list_view, event)) {
        return true;
    }

    // TODO: 将事件传递给工具栏和侧边栏处理
    // if (toolbar_handle_event(window->toolbar, event)) { return true; }
    // if (sidebar_handle_event(window->sidebar, event)) { return true; }

    return false;
}

// 绘制主窗口内容
void main_window_draw(MainWindow *window) {
    if (!window || !window->app || !window->app->renderer) {
        return;
    }
    
    // 绘制文件列表视图
    file_list_view_draw(window->file_list_view);

    // TODO: 绘制工具栏和侧边栏
    // toolbar_draw(window->toolbar);
    // sidebar_draw(window->sidebar);
}
