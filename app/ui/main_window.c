/*
 * 主窗口模块（与 sidebar.c 风格一致的整体装配解读）
 * 目标：装配各 UI 组件（文件列表/侧边栏/工具栏/右键菜单），并负责事件分发与布局。
 * 
 * 组件装配与依赖：
 * - FileListView：核心内容区，负责目录呈现与交互；
 * - Sidebar：左侧导航区，负责路径快捷入口与驱动器跳转；
 * - Toolbar：上方操作区，负责历史导航/视图切换/搜索过滤；
 * - ContextMenu：上下文动作入口，依赖 FileListView 触发重命名/刷新等。
 * 
 * 事件分发优先级（先捕获悬浮层，风格与 sidebar 一致）：
 * 1) ContextMenu（可见时独占处理）
 * 2) Toolbar（按钮与搜索输入）
 * 3) FileListView（滚动/选择/双击/重命名）
 * 4) Sidebar（项目选择/滚动）
 * 
 * 绘制顺序（保证视觉层级）：
 * - FileListView -> Toolbar -> Sidebar -> ContextMenu（最上层）。
 * 
 * 布局策略（与 sidebar.c 保持常量一致）：
 * - FileListView 视口：x=SIDEBAR_WIDTH, y=TOOLBAR_HEIGHT；
 * - Toolbar：高度固定为 TOOLBAR_HEIGHT；
 * - Sidebar：宽度固定为 SIDEBAR_WIDTH，起点 y=TOOLBAR_HEIGHT。
 * 
 * 生命周期：
 * - new：创建并互相连接组件与回调（如目录变更 -> toolbar 历史；sidebar 选中 -> file_list 加载）。
 * - resize：依据窗口尺寸更新各组件的矩形区域；
 * - draw：按“先底层后顶层”的顺序绘制；
 * - free：对称释放各组件。
 */

#include "main_window.h"
#include "file_list.h"
#include "toolbar.h"
#include "sidebar.h"
#include "renderer.h"
#include "context_menu.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#ifndef PATH_MAX
#define PATH_MAX 260  // Windows默认最大路径长度
#endif

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
    
    // 处理驱动器路径格式转换
    char actual_path[PATH_MAX];
    if (strlen(path) == 2 && path[1] == ':') {
        // 如果是驱动器路径格式（如"E:"），转换为"E:\\"
        snprintf(actual_path, sizeof(actual_path), "%s\\", path);
    } else {
        // 其他路径直接使用
        strncpy(actual_path, path, sizeof(actual_path) - 1);
        actual_path[sizeof(actual_path) - 1] = '\0';
    }
    
    // 加载选中的目录
    file_list_view_load_directory(main_window->file_list_view, actual_path);
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

    // 设置用户数据，用于回调函数中获取主窗口实例
    a->user_data = window;
    
    // 设置回调函数
    window->file_list_view->on_right_click = on_file_list_right_click;
    window->file_list_view->on_directory_changed = on_directory_changed;
    
    // 获取当前窗口大小并设置初始布局
    int window_width, window_height;
    SDL_GetWindowSize(a->window, &window_width, &window_height);
    main_window_resize(window, window_width, window_height);
    
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

// 窗口大小调整函数
void main_window_resize(MainWindow *window, int width, int height) {
    if (!window || !window->app) {
        return;
    }
    
    printf("[DEBUG] Resizing main window to %dx%d\n", width, height);
    
    // 更新文件列表视图的视口区域
    if (window->file_list_view) {
        window->file_list_view->viewport.x = SIDEBAR_WIDTH;
        window->file_list_view->viewport.y = TOOLBAR_HEIGHT;
        window->file_list_view->viewport.w = width - SIDEBAR_WIDTH;
        window->file_list_view->viewport.h = height - TOOLBAR_HEIGHT;
        
        printf("[DEBUG] File list viewport: x=%d, y=%d, w=%d, h=%d\n", 
               window->file_list_view->viewport.x, window->file_list_view->viewport.y,
               window->file_list_view->viewport.w, window->file_list_view->viewport.h);
    }
    
    // 更新工具栏区域
    if (window->toolbar) {
        window->toolbar->rect.x = 0;
        window->toolbar->rect.y = 0;
        window->toolbar->rect.w = width;
        window->toolbar->rect.h = TOOLBAR_HEIGHT;
    }
    
    // 更新侧边栏区域
    if (window->sidebar) {
        window->sidebar->rect.x = 0;
        window->sidebar->rect.y = TOOLBAR_HEIGHT;
        window->sidebar->rect.w = SIDEBAR_WIDTH;
        window->sidebar->rect.h = height - TOOLBAR_HEIGHT;
    }
}
