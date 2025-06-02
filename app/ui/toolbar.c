/*
 * 工具栏模块
 * 职责：
 * 1. 实现工具栏界面
 * 2. 管理工具栏按钮（前进、后退、上一级、搜索等）
 * 3. 处理工具栏事件和回调
 * 4. 工具栏状态管理（按钮启用/禁用）
 */

#include "toolbar.h"
#include "renderer.h"
#include "file_list.h"
#include <string.h>
#include <math.h>
#include "main_window.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// 工具栏样式常量
#define TOOLBAR_HEIGHT 40      // 工具栏的高度（像素）
#define BUTTON_PADDING 5       // 按钮内边距（像素）
#define BUTTON_SIZE 30         // 按钮的尺寸（像素）
#define BUTTON_SPACING 10      // 按钮之间的间距（像素）

// 颜色常量
static const SDL_Color TOOLBAR_BG_COLOR = {240, 240, 240, 255};
static const SDL_Color BUTTON_COLOR = {200, 200, 200, 255};
static const SDL_Color BUTTON_HOVER_COLOR = {180, 180, 180, 255};
static const SDL_Color BUTTON_ACTIVE_COLOR = {160, 160, 160, 255};
static const SDL_Color BUTTON_DISABLED_COLOR = {220, 220, 220, 128};
static const SDL_Color BUTTON_BORDER_COLOR = {100, 100, 100, 255};
static const SDL_Color BUTTON_ICON_COLOR = {50, 50, 50, 255};


// 绘制工具栏按钮
static void draw_toolbar_button(Toolbar *toolbar, ToolbarButton *button) {
    if (!toolbar || !button || !toolbar->app || !toolbar->app->renderer) {
        return;
    }

    SDL_Renderer *renderer = toolbar->app->renderer;
    
    // 选择按钮颜色
    SDL_Color button_color;
    if (!button->enabled) {
        button_color = BUTTON_DISABLED_COLOR;
    } else if (button->pressed) {
        button_color = BUTTON_ACTIVE_COLOR;
    } else if (button->hovered) {
        button_color = BUTTON_HOVER_COLOR;
    } else {
        button_color = BUTTON_COLOR;
    }
    
    // 绘制按钮背景
    SDL_SetRenderDrawColor(renderer, button_color.r, button_color.g, button_color.b, button_color.a);
    SDL_FRect frect = {
        (float)button->rect.x,
        (float)button->rect.y,
        (float)button->rect.w,
        (float)button->rect.h
    };
    SDL_RenderFillRect(renderer, &frect);
    
    // 绘制按钮边框
    SDL_SetRenderDrawColor(renderer, BUTTON_BORDER_COLOR.r, BUTTON_BORDER_COLOR.g, BUTTON_BORDER_COLOR.b, BUTTON_BORDER_COLOR.a);
    SDL_RenderRect(renderer, &frect);
    
    // 绘制按钮图标
    SDL_SetRenderDrawColor(renderer, BUTTON_ICON_COLOR.r, BUTTON_ICON_COLOR.g, BUTTON_ICON_COLOR.b, BUTTON_ICON_COLOR.a);
    
    int icon_size = BUTTON_SIZE / 2;
    int icon_x = button->rect.x + (button->rect.w - icon_size) / 2;
    int icon_y = button->rect.y + (button->rect.h - icon_size) / 2;
    
    // 根据按钮类型绘制不同的图标
    switch (button->type) {
        case BUTTON_BACK:
            // 绘制后退箭头
            {
                SDL_FPoint points[3] = {
                    {(float)(icon_x + icon_size), (float)icon_y},
                    {(float)icon_x, (float)(icon_y + icon_size / 2)},
                    {(float)(icon_x + icon_size), (float)(icon_y + icon_size)}
                };
                SDL_RenderLines(renderer, points, 3);
            }
            break;
            
        case BUTTON_FORWARD:
            // 绘制前进箭头
            {
                SDL_FPoint points[3] = {
                    {(float)icon_x, (float)icon_y},
                    {(float)(icon_x + icon_size), (float)(icon_y + icon_size / 2)},
                    {(float)icon_x, (float)(icon_y + icon_size)}
                };
                SDL_RenderLines(renderer, points, 3);
            }
            break;
            
        case BUTTON_UP:
            // 绘制上一级箭头
            {
                SDL_FPoint points[3] = {
                    {(float)(icon_x + icon_size / 2), (float)icon_y},
                    {(float)icon_x, (float)(icon_y + icon_size / 2)},
                    {(float)(icon_x + icon_size), (float)(icon_y + icon_size / 2)}
                };
                SDL_RenderLines(renderer, points, 3);
                SDL_FRect line = {
                    (float)(icon_x + icon_size / 2),
                    (float)(icon_y + icon_size / 2),
                    1.0f,
                    (float)(icon_size / 2)
                };
                SDL_RenderFillRect(renderer, &line);
            }
            break;
            
        case BUTTON_HOME:
            // 绘制主目录图标 (简化的房子)
            {
                SDL_FPoint roof[3] = {
                    {(float)(icon_x + icon_size / 2), (float)icon_y},
                    {(float)icon_x, (float)(icon_y + icon_size / 2)},
                    {(float)(icon_x + icon_size), (float)(icon_y + icon_size / 2)}
                };
                SDL_RenderLines(renderer, roof, 3);
                
                SDL_FRect house = {
                    (float)(icon_x + icon_size / 4), 
                    (float)(icon_y + icon_size / 2), 
                    (float)(icon_size / 2), 
                    (float)(icon_size / 2)
                };
                SDL_RenderRect(renderer, &house);
            }
            break;
            
        case BUTTON_REFRESH:
            // 绘制刷新图标 (圆形)
            {
                int cx = icon_x + icon_size / 2;
                int cy = icon_y + icon_size / 2;
                int r = icon_size / 2 - 2;  // 稍微缩小半径以确保在按钮内
                
                // 绘制完整圆形
                for (int i = 0; i < 16; i++) {
                    double angle1 = i * M_PI / 8;
                    double angle2 = (i + 1) * M_PI / 8;
                    
                    float x1 = cx + r * cos(angle1);
                    float y1 = cy + r * sin(angle1);
                    float x2 = cx + r * cos(angle2);
                    float y2 = cy + r * sin(angle2);
                    
                    SDL_RenderLine(renderer, x1, y1, x2, y2);
                }
                
                // 绘制箭头
                SDL_FPoint arrow[3] = {
                    {(float)(cx + r * 0.7), (float)(cy - r * 0.7)},
                    {(float)(cx + r), (float)(cy - r * 0.7)},
                    {(float)(cx + r), (float)(cy - r * 0.4)}
                };
                SDL_RenderLines(renderer, arrow, 3);
            }
            break;
            
        case BUTTON_SEARCH:
            // 绘制搜索图标 (简化的放大镜)
            {
                int r = icon_size / 3;
                SDL_FPoint center = {(float)(icon_x + r), (float)(icon_y + r)};
                
                // 绘制圆圈
                for (int i = 0; i < 8; i++) {
                    double angle = i * M_PI / 4;
                    float x1 = center.x + r * cos(angle);
                    float y1 = center.y + r * sin(angle);
                    float x2 = center.x + r * cos(angle + M_PI / 4);
                    float y2 = center.y + r * sin(angle + M_PI / 4);
                    SDL_RenderLine(renderer, x1, y1, x2, y2);
                }
                
                // 绘制手柄
                SDL_RenderLine(
                    renderer,
                    center.x + r * 0.7f,
                    center.y + r * 0.7f,
                    (float)(icon_x + icon_size),
                    (float)(icon_y + icon_size)
                );
            }
            break;
            
        case BUTTON_VIEW:
            // 绘制视图切换图标 (简化的网格)
            {
                SDL_FRect grid1 = {
                    (float)icon_x, 
                    (float)icon_y, 
                    (float)(icon_size/2 - 1), 
                    (float)(icon_size/2 - 1)
                };
                SDL_FRect grid2 = {
                    (float)(icon_x + icon_size/2 + 1), 
                    (float)icon_y, 
                    (float)(icon_size/2 - 1), 
                    (float)(icon_size/2 - 1)
                };
                SDL_FRect grid3 = {
                    (float)icon_x, 
                    (float)(icon_y + icon_size/2 + 1), 
                    (float)(icon_size/2 - 1), 
                    (float)(icon_size/2 - 1)
                };
                SDL_FRect grid4 = {
                    (float)(icon_x + icon_size/2 + 1), 
                    (float)(icon_y + icon_size/2 + 1), 
                    (float)(icon_size/2 - 1), 
                    (float)(icon_size/2 - 1)
                };
                
                SDL_RenderRect(renderer, &grid1);
                SDL_RenderRect(renderer, &grid2);
                SDL_RenderRect(renderer, &grid3);
                SDL_RenderRect(renderer, &grid4);
            }
            break;
            
        default:
            break;
    }
}

// 查找点击的按钮
static ToolbarButton* find_button_at_point(Toolbar *toolbar, int x, int y) {
    if (!toolbar) {
        return NULL;
    }
    
    for (int i = 0; i < toolbar->button_count; i++) {
        ToolbarButton *button = &toolbar->buttons[i];
        if (x >= button->rect.x && x < button->rect.x + button->rect.w &&
            y >= button->rect.y && y < button->rect.y + button->rect.h) {
            return button;
        }
    }
    
    return NULL;
}

// 添加目录到历史记录
static void add_to_history(Toolbar *toolbar, const char *path) {
    if (!toolbar || !path) {
        return;
    }
    
    // 清除当前位置之后的历史记录
    for (int i = toolbar->history_index + 1; i < toolbar->history_count; i++) {
        free(toolbar->history[i]);
        toolbar->history[i] = NULL;
    }
    
    // 更新历史记录数量
    toolbar->history_count = toolbar->history_index + 1;
    
    // 检查是否需要扩展历史记录容量
    if (toolbar->history_count >= toolbar->history_capacity) {
        int new_capacity = toolbar->history_capacity * 2;
        char **new_history = (char**)realloc(toolbar->history, new_capacity * sizeof(char*));
        if (!new_history) {
            return;
        }
        
        toolbar->history = new_history;
        toolbar->history_capacity = new_capacity;
    }
    
    // 添加新路径到历史记录
    toolbar->history[toolbar->history_count] = strdup(path);
    if (!toolbar->history[toolbar->history_count]) {
        return;
    }
    
    toolbar->history_count++;
    toolbar->history_index = toolbar->history_count - 1;
    
    // 更新按钮状态
    toolbar->buttons[BUTTON_BACK].enabled = (toolbar->history_index > 0);
    toolbar->buttons[BUTTON_FORWARD].enabled = (toolbar->history_index < toolbar->history_count - 1);
}

// 执行后退操作
static void navigate_back(Toolbar *toolbar) {
    if (!toolbar || toolbar->history_index <= 0 || !toolbar->app || !toolbar->app->user_data) {
        return;
    }
    
    toolbar->history_index--;
    const char *path = toolbar->history[toolbar->history_index];
    
    // 更新文件列表视图
    MainWindow *main_window = (MainWindow*)toolbar->app->user_data;
    file_list_view_load_directory(main_window->file_list_view, path);
    
    // 更新按钮状态
    toolbar->buttons[BUTTON_BACK].enabled = (toolbar->history_index > 0);
    toolbar->buttons[BUTTON_FORWARD].enabled = (toolbar->history_index < toolbar->history_count - 1);
}

// 执行前进操作
static void navigate_forward(Toolbar *toolbar) {
    if (!toolbar || toolbar->history_index >= toolbar->history_count - 1 || !toolbar->app || !toolbar->app->user_data) {
        return;
    }
    
    toolbar->history_index++;
    const char *path = toolbar->history[toolbar->history_index];
    
    // 更新文件列表视图
    MainWindow *main_window = (MainWindow*)toolbar->app->user_data;
    file_list_view_load_directory(main_window->file_list_view, path);
    
    // 更新按钮状态
    toolbar->buttons[BUTTON_BACK].enabled = (toolbar->history_index > 0);
    toolbar->buttons[BUTTON_FORWARD].enabled = (toolbar->history_index < toolbar->history_count - 1);
}

// 执行上一级操作
static void navigate_up(Toolbar *toolbar) {
    if (!toolbar || !toolbar->app || !toolbar->app->user_data) {
        return;
    }
    
    MainWindow *main_window = (MainWindow*)toolbar->app->user_data;
    FileListView *file_list = main_window->file_list_view;
    
    if (!file_list || !file_list->current_path) {
        return;
    }
    
    // 获取上一级目录路径
    char *path = strdup(file_list->current_path);
    if (!path) {
        return;
    }
    
    // 移除末尾的路径分隔符
    size_t len = strlen(path);
    if (len > 0 && (path[len - 1] == '/' || path[len - 1] == '\\')) {
        path[len - 1] = '\0';
        len--;
    }
    
    // 找到最后一个路径分隔符
    char *last_sep = strrchr(path, '/');
    if (!last_sep) {
        last_sep = strrchr(path, '\\');
    }
    
    if (last_sep) {
        // 如果是根目录，保留路径分隔符
        if (last_sep == path) {
            path[1] = '\0';
        } else {
            *last_sep = '\0';
        }
        
        // 加载上一级目录
        file_list_view_load_directory(file_list, path);
        add_to_history(toolbar, path);
    }
    
    free(path);
}

// 执行主目录操作
static void navigate_home(Toolbar *toolbar) {
    if (!toolbar || !toolbar->app || !toolbar->app->user_data) {
        return;
    }
    
    MainWindow *main_window = (MainWindow*)toolbar->app->user_data;
    FileListView *file_list = main_window->file_list_view;
    
    if (!file_list) {
        return;
    }
    
   // 获取用户主目录
    // const char *home_dir = SDL_GetHomePath();
    // const char* SDL_GetUserFolder(SDL_Folder folder);
    const char *home_dir = SDL_GetUserFolder(SDL_FOLDER_HOME);
    if (home_dir) {
        file_list_view_load_directory(file_list, home_dir);
        add_to_history(toolbar, home_dir);
    }
}

// 执行刷新操作
static void refresh_view(Toolbar *toolbar) {
    if (!toolbar || !toolbar->app || !toolbar->app->user_data) {
        return;
    }
    
    MainWindow *main_window = (MainWindow*)toolbar->app->user_data;
    FileListView *file_list = main_window->file_list_view;
    
    if (!file_list || !file_list->current_path) {
        return;
    }
    
    // 重新加载当前目录
    file_list_view_load_directory(file_list, file_list->current_path);
}

// 执行按钮操作
static void execute_button_action(Toolbar *toolbar, ToolbarButton *button) {
    if (!toolbar || !button || !button->enabled) {
        return;
    }
    
    switch (button->type) {
        case BUTTON_BACK:
            navigate_back(toolbar);
            break;
            
        case BUTTON_FORWARD:
            navigate_forward(toolbar);
            break;
            
        case BUTTON_UP:
            navigate_up(toolbar);
            break;
            
        case BUTTON_HOME:
            navigate_home(toolbar);
            break;
            
        case BUTTON_REFRESH:
            refresh_view(toolbar);
            break;
            
        case BUTTON_SEARCH:
            // TODO: 实现搜索功能
            break;
            
        case BUTTON_VIEW:
            // TODO: 实现视图切换功能
            break;
            
        default:
            break;
    }
}

// 创建工具栏
Toolbar* toolbar_new(struct Window *app) {
    if (!app) {
        return NULL;
    }
    
    Toolbar *toolbar = (Toolbar*)calloc(1, sizeof(Toolbar));
    if (!toolbar) {
        return NULL;
    }
    
    toolbar->app = app;
    
    // 设置工具栏区域
    toolbar->rect.x = 0;
    toolbar->rect.y = 0;
    toolbar->rect.w = SDL_WINDOW_WIDTH;
    toolbar->rect.h = TOOLBAR_HEIGHT;
    
    // 初始化按钮
    int button_x = BUTTON_PADDING;
    
    // 后退按钮
    toolbar->buttons[BUTTON_BACK].type = BUTTON_BACK;
    toolbar->buttons[BUTTON_BACK].rect.x = button_x;
    toolbar->buttons[BUTTON_BACK].rect.y = (TOOLBAR_HEIGHT - BUTTON_SIZE) / 2;
    toolbar->buttons[BUTTON_BACK].rect.w = BUTTON_SIZE;
    toolbar->buttons[BUTTON_BACK].rect.h = BUTTON_SIZE;
    toolbar->buttons[BUTTON_BACK].tooltip = "后退";
    toolbar->buttons[BUTTON_BACK].enabled = false;
    button_x += BUTTON_SIZE + BUTTON_SPACING;

    // 前进按钮
    toolbar->buttons[BUTTON_FORWARD].type = BUTTON_FORWARD;
    toolbar->buttons[BUTTON_FORWARD].rect.x = button_x;
    toolbar->buttons[BUTTON_FORWARD].rect.y = (TOOLBAR_HEIGHT - BUTTON_SIZE) / 2;
    toolbar->buttons[BUTTON_FORWARD].rect.w = BUTTON_SIZE;
    toolbar->buttons[BUTTON_FORWARD].rect.h = BUTTON_SIZE;
    toolbar->buttons[BUTTON_FORWARD].tooltip = "前进";
    toolbar->buttons[BUTTON_FORWARD].enabled = false;
    button_x += BUTTON_SIZE + BUTTON_SPACING;
    
    // 上一级按钮
    toolbar->buttons[BUTTON_UP].type = BUTTON_UP;
    toolbar->buttons[BUTTON_UP].rect.x = button_x;
    toolbar->buttons[BUTTON_UP].rect.y = (TOOLBAR_HEIGHT - BUTTON_SIZE) / 2;
    toolbar->buttons[BUTTON_UP].rect.w = BUTTON_SIZE;
    toolbar->buttons[BUTTON_UP].rect.h = BUTTON_SIZE;
    toolbar->buttons[BUTTON_UP].tooltip = "上一级";
    toolbar->buttons[BUTTON_UP].enabled = true;
    button_x += BUTTON_SIZE + BUTTON_SPACING;
    
    // 主目录按钮
    toolbar->buttons[BUTTON_HOME].type = BUTTON_HOME;
    toolbar->buttons[BUTTON_HOME].rect.x = button_x;
    toolbar->buttons[BUTTON_HOME].rect.y = (TOOLBAR_HEIGHT - BUTTON_SIZE) / 2;
    toolbar->buttons[BUTTON_HOME].rect.w = BUTTON_SIZE;
    toolbar->buttons[BUTTON_HOME].rect.h = BUTTON_SIZE;
    toolbar->buttons[BUTTON_HOME].tooltip = "主目录";
    toolbar->buttons[BUTTON_HOME].enabled = true;
    button_x += BUTTON_SIZE + BUTTON_SPACING;
    
    // 刷新按钮
    toolbar->buttons[BUTTON_REFRESH].type = BUTTON_REFRESH;
    toolbar->buttons[BUTTON_REFRESH].rect.x = button_x;
    toolbar->buttons[BUTTON_REFRESH].rect.y = (TOOLBAR_HEIGHT - BUTTON_SIZE) / 2;
    toolbar->buttons[BUTTON_REFRESH].rect.w = BUTTON_SIZE;
    toolbar->buttons[BUTTON_REFRESH].rect.h = BUTTON_SIZE;
    toolbar->buttons[BUTTON_REFRESH].tooltip = "刷新";
    toolbar->buttons[BUTTON_REFRESH].enabled = true;
    button_x += BUTTON_SIZE + BUTTON_SPACING;
    
    // 搜索按钮
    toolbar->buttons[BUTTON_SEARCH].type = BUTTON_SEARCH;
    toolbar->buttons[BUTTON_SEARCH].rect.x = button_x;
    toolbar->buttons[BUTTON_SEARCH].rect.y = (TOOLBAR_HEIGHT - BUTTON_SIZE) / 2;
    toolbar->buttons[BUTTON_SEARCH].rect.w = BUTTON_SIZE;
    toolbar->buttons[BUTTON_SEARCH].rect.h = BUTTON_SIZE;
    toolbar->buttons[BUTTON_SEARCH].tooltip = "搜索";
    toolbar->buttons[BUTTON_SEARCH].enabled = true;
    button_x += BUTTON_SIZE + BUTTON_SPACING;
    
    // 视图按钮
    toolbar->buttons[BUTTON_VIEW].type = BUTTON_VIEW;
    toolbar->buttons[BUTTON_VIEW].rect.x = button_x;
    toolbar->buttons[BUTTON_VIEW].rect.y = (TOOLBAR_HEIGHT - BUTTON_SIZE) / 2;
    toolbar->buttons[BUTTON_VIEW].rect.w = BUTTON_SIZE;
    toolbar->buttons[BUTTON_VIEW].rect.h = BUTTON_SIZE;
    toolbar->buttons[BUTTON_VIEW].tooltip = "视图";
    toolbar->buttons[BUTTON_VIEW].enabled = true;
    
    toolbar->button_count = BUTTON_COUNT;
    
    // 初始化历史记录
    toolbar->history_capacity = 10;
    toolbar->history = (char**)calloc(toolbar->history_capacity, sizeof(char*));
    if (!toolbar->history) {
        free(toolbar);
        return NULL;
    }
    
    toolbar->history_count = 0;
    toolbar->history_index = -1;
    
    return toolbar;
}

// 释放工具栏
void toolbar_free(Toolbar *toolbar) {
    if (!toolbar) {
        return;
    }
    
    // 释放历史记录
    if (toolbar->history) {
        for (int i = 0; i < toolbar->history_count; i++) {
            if (toolbar->history[i]) {
                free(toolbar->history[i]);
            }
        }
        free(toolbar->history);
    }
    
    free(toolbar);
}

// 处理工具栏事件
bool toolbar_handle_event(Toolbar *toolbar, SDL_Event *event) {
    if (!toolbar || !event) {
        return false;
    }
    
    switch (event->type) {
        case SDL_EVENT_MOUSE_MOTION:
            {
                int x = event->motion.x;
                int y = event->motion.y;
                
                // 检查鼠标是否在工具栏区域内
                if (x >= toolbar->rect.x && x < toolbar->rect.x + toolbar->rect.w &&
                    y >= toolbar->rect.y && y < toolbar->rect.y + toolbar->rect.h) {
                    
                    // 更新按钮悬停状态
                    ToolbarButton *hover_button = find_button_at_point(toolbar, x, y);
                    for (int i = 0; i < toolbar->button_count; i++) {
                        toolbar->buttons[i].hovered = (hover_button == &toolbar->buttons[i]);
                    }
                    
                    return true;
                } else {
                    // 鼠标移出工具栏区域，清除所有悬停状态
                    for (int i = 0; i < toolbar->button_count; i++) {
                        toolbar->buttons[i].hovered = false;
                    }
                }
            }
            break;
            
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            {
                int x = event->button.x;
                int y = event->button.y;
                
                // 检查鼠标点击是否在工具栏区域内
                if (event->button.button == SDL_BUTTON_LEFT &&
                    x >= toolbar->rect.x && x < toolbar->rect.x + toolbar->rect.w &&
                    y >= toolbar->rect.y && y < toolbar->rect.y + toolbar->rect.h) {
                    
                    // 查找点击的按钮
                    ToolbarButton *button = find_button_at_point(toolbar, x, y);
                    if (button) {
                        button->pressed = true;
                        return true;
                    }
                }
            }
            break;
            
        case SDL_EVENT_MOUSE_BUTTON_UP:
            {
                int x = event->button.x;
                int y = event->button.y;
                
                // 检查是否有按钮被按下
                for (int i = 0; i < toolbar->button_count; i++) {
                    ToolbarButton *button = &toolbar->buttons[i];
                    if (button->pressed) {
                        button->pressed = false;
                        
                        // 检查鼠标释放是否在按钮区域内
                        if (x >= button->rect.x && x < button->rect.x + button->rect.w &&
                            y >= button->rect.y && y < button->rect.y + button->rect.h) {
                            
                            // 执行按钮操作
                            execute_button_action(toolbar, button);
                        }
                        
                        return true;
                    }
                }
            }
            break;
            
        default:
            break;
    }
    
    return false;
}

// 通知工具栏目录已更改
void toolbar_notify_directory_changed(Toolbar *toolbar, const char *path) {
    if (!toolbar || !path) {
        return;
    }
    
    // 添加到历史记录
    add_to_history(toolbar, path);
}

// 绘制工具栏
void toolbar_draw(Toolbar *toolbar) {
    if (!toolbar || !toolbar->app || !toolbar->app->renderer) {
        return;
    }
    
    SDL_Renderer *renderer = toolbar->app->renderer;
    
    // 绘制工具栏背景
    SDL_SetRenderDrawColor(renderer, TOOLBAR_BG_COLOR.r, TOOLBAR_BG_COLOR.g, TOOLBAR_BG_COLOR.b, TOOLBAR_BG_COLOR.a);
    SDL_FRect toolbar_frect = {
        (float)toolbar->rect.x,
        (float)toolbar->rect.y,
        (float)toolbar->rect.w,
        (float)toolbar->rect.h
    };
    SDL_RenderFillRect(renderer, &toolbar_frect);
    
    // 绘制工具栏边框
    SDL_SetRenderDrawColor(renderer, BUTTON_BORDER_COLOR.r, BUTTON_BORDER_COLOR.g, BUTTON_BORDER_COLOR.b, BUTTON_BORDER_COLOR.a);
    SDL_RenderRect(renderer, &toolbar_frect);
    
    // 绘制工具栏按钮
    for (int i = 0; i < toolbar->button_count; i++) {
        draw_toolbar_button(toolbar, &toolbar->buttons[i]);
    }
}

// 工具栏基本函数声明
Toolbar* toolbar_new(struct Window *app);  // 创建新的工具栏
void toolbar_free(Toolbar *toolbar);       // 释放工具栏资源
bool toolbar_handle_event(Toolbar *toolbar, SDL_Event *event);  // 处理用户事件
void toolbar_draw(Toolbar *toolbar);       // 绘制工具栏

// 通知工具栏目录已更改
void toolbar_notify_directory_changed(Toolbar *toolbar, const char *path);  // 当目录改变时更新历史

// 历史导航函数
bool toolbar_go_back(Toolbar *toolbar);     // 后退到上一个访问的目录
bool toolbar_go_forward(Toolbar *toolbar);  // 前进到下一个访问的目录
bool toolbar_go_up(Toolbar *toolbar);       // 上升到父目录
bool toolbar_go_home(Toolbar *toolbar);     // 返回到主目录

// 按钮操作函数
bool toolbar_refresh(Toolbar *toolbar);     // 刷新当前目录
bool toolbar_toggle_view(Toolbar *toolbar); // 切换文件视图模式
bool toolbar_search(Toolbar *toolbar, const char *search_term);  // 搜索文件

// 设置按钮状态
void toolbar_set_button_enabled(Toolbar *toolbar, ToolbarButtonType button_type, bool enabled);  // 启用/禁用特定按钮
void toolbar_update_button_states(Toolbar *toolbar);  // 更新所有按钮状态
