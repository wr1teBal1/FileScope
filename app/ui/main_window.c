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
#include "context_menu.h"
#include <stdlib.h>

// 右键点击回调函数
static void on_file_list_right_click(FileListView *view, int x, int y, FileItem *item) {
    // 获取主窗口实例
    MainWindow *main_window = (MainWindow*)view->window->user_data;
    if (!main_window || !main_window->context_menu) {
        return;
    }
    
    if (item) {
        // 右键点击文件/文件夹
        context_menu_show_for_file(main_window->context_menu, item, x, y);
    } else {
        // 右键点击空白区域
        context_menu_show_for_blank(main_window->context_menu, x, y);
    }
}

// 目录变更回调函数 - 连接FileListView和Toolbar
static void on_directory_changed(FileListView *view, const char *path) {
    // 获取主窗口实例
    MainWindow *main_window = (MainWindow*)view->window->user_data;
    if (!main_window || !main_window->toolbar) {
        return;
    }
    
    // 通知工具栏目录已更改
    toolbar_notify_directory_changed(main_window->toolbar, path);
}

// 侧边栏项目选中回调函数
static void on_sidebar_item_selected(Sidebar *sidebar, const char *path) {
    if (!sidebar || !path) {
        return;
    }
    
    // 获取主窗口实例
    MainWindow *main_window = (MainWindow*)sidebar->app->user_data;
    if (!main_window || !main_window->file_list_view) {
        return;
    }
    
    // 加载选中的目录
    file_list_view_load_directory(main_window->file_list_view, path);
}

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

    // 创建右键菜单
    window->context_menu = context_menu_new(a);
    if (!window->context_menu) {
        file_list_view_free(window->file_list_view);
        free(window);
        return NULL;
    }
    
    // 设置上下文菜单的文件列表视图引用
    context_menu_set_file_list_view(window->context_menu, window->file_list_view);

    // 创建工具栏
    window->toolbar = toolbar_new(a);
    if (!window->toolbar) {
        context_menu_free(window->context_menu);
        file_list_view_free(window->file_list_view);
        free(window);
        return NULL;
    }

    // 创建侧边栏
    window->sidebar = sidebar_new(a);
    if (!window->sidebar) {
        toolbar_free(window->toolbar);
        context_menu_free(window->context_menu);
        file_list_view_free(window->file_list_view);
        free(window);
        return NULL;
    }
    
    // 设置侧边栏项目选中回调
    sidebar_set_item_selected_callback(window->sidebar, on_sidebar_item_selected);

    // 设置文件列表视图的视口区域（考虑侧边栏宽度）
    window->file_list_view->viewport.x = SIDEBAR_WIDTH;
    window->file_list_view->viewport.y = TOOLBAR_HEIGHT;
    window->file_list_view->viewport.w = SDL_WINDOW_WIDTH - SIDEBAR_WIDTH;
    window->file_list_view->viewport.h = SDL_WINDOW_HEIGHT - TOOLBAR_HEIGHT;

    // 设置用户数据，用于回调函数中获取主窗口实例
    a->user_data = window;
    
    // 设置回调函数
    window->file_list_view->on_right_click = on_file_list_right_click;
    window->file_list_view->on_directory_changed = on_directory_changed;
    
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
    if (window->context_menu) {
        context_menu_free(window->context_menu);
    }
    if (window->toolbar) {
        toolbar_free(window->toolbar);
    }
    // 释放侧边栏
    if (window->sidebar) { 
        sidebar_free(window->sidebar); 
    }

    free(window);
}

// 处理主窗口事件
bool main_window_handle_event(MainWindow *window, SDL_Event *event) {
    if (!window || !event) {
        return false;
    }

    // 优先处理右键菜单事件
    if (context_menu_handle_event(window->context_menu, event)) {
        return true;
    }

    // 处理工具栏事件
    if (window->toolbar && toolbar_handle_event(window->toolbar, event)) {
        return true;
    }

    // 将事件传递给文件列表视图处理
    if (file_list_view_handle_event(window->file_list_view, event)) {
        return true;
    }

    // 将事件传递给侧边栏处理
    if (window->sidebar && sidebar_handle_event(window->sidebar, event)) { 
        return true; 
    }

    return false;
}

// 绘制主窗口内容
void main_window_draw(MainWindow *window) {
    if (!window || !window->app || !window->app->renderer) {
        return;
    }
    
    // 绘制文件列表视图
    file_list_view_draw(window->file_list_view);

    // 绘制工具栏
    if (window->toolbar) {
        toolbar_draw(window->toolbar);
    }
    
    // 绘制侧边栏
    if (window->sidebar) {
        sidebar_draw(window->sidebar);
    }

    // 绘制右键菜单（最后绘制，确保在最上层）
    context_menu_draw(window->context_menu);
}
