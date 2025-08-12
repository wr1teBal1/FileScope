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
#include <SDL3_ttf/SDL_ttf.h> // 确保加上
#include <SDL3/SDL.h> // 添加SDL3头文件
// #include "toolbar.h"
// #include "renderer.h"
// #include "file_list.h"
// #include <string.h>
// #include <math.h>
// #include "main_window.h"
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
    SDL_RenderFillRect(renderer, &frect);// 绘制矩形
    
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
            // 绘制上一级箭头（闭合三角形+精确箭杆连接）
            {
                float top_x = icon_x + icon_size / 2.0f;
                float top_y = icon_y;
                float left_x = icon_x;
                float left_y = icon_y + icon_size / 2.0f;
                float right_x = icon_x + icon_size;
                float right_y = icon_y + icon_size / 2.0f;
                float mid_x = icon_x + icon_size / 2.0f;
                float mid_y = icon_y + icon_size / 2.0f;
                float shaft_bottom_y = icon_y + icon_size - 2;

                // 闭合三角形
                SDL_FPoint triangle[4] = {
                    {top_x, top_y},
                    {left_x, left_y},
                    {right_x, right_y},
                    {top_x, top_y}
                };
                SDL_RenderLines(renderer, triangle, 4);
                // 箭杆与底边中点无缝连接
                SDL_RenderLine(renderer, mid_x, mid_y, mid_x, shaft_bottom_y);
            }
            break;
            
        case BUTTON_HOME:
            // 绘制主目录图标 (闭合屋顶三角形+房屋主体与屋檐底边重合)
            {
                // 屋顶三角形
                float roof_top_x = icon_x + icon_size / 2.0f;
                float roof_top_y = icon_y;
                float roof_left_x = icon_x;
                float roof_left_y = icon_y + icon_size / 2.5f;
                float roof_right_x = icon_x + icon_size;
                float roof_right_y = icon_y + icon_size / 2.5f;
                SDL_FPoint roof[4] = {
                    {roof_top_x, roof_top_y},
                    {roof_left_x, roof_left_y},
                    {roof_right_x, roof_right_y},
                    {roof_top_x, roof_top_y}
                };
                SDL_RenderLines(renderer, roof, 4);

                // 房屋主体（顶部与屋檐底边重合）
                float house_x = roof_left_x + (roof_right_x - roof_left_x) / 4.0f;
                float house_w = (roof_right_x - roof_left_x) / 2.0f;
                float house_y = roof_left_y; // 顶部与屋檐底边重合
                float house_h = icon_size - (house_y - icon_y) - 2;
                SDL_FRect house = {house_x, house_y, house_w, house_h};
                SDL_RenderRect(renderer, &house);

                // 屋顶与房屋主体连接线（左右两侧）
                SDL_RenderLine(renderer, roof_left_x, roof_left_y, house_x, house_y);
                SDL_RenderLine(renderer, roof_right_x, roof_right_y, house_x + house_w, house_y);
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
                }//有问题
                
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
            // 激活搜索功能
            toolbar_search_start(toolbar);
            break;
            
        case BUTTON_VIEW:
            // 实现视图切换功能
            {
                MainWindow *main_window = (MainWindow*)toolbar->app->user_data;
                if (main_window && main_window->file_list_view) {
                    file_list_view_toggle_mode(main_window->file_list_view);
                }
            }
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

    toolbar->search_text[0] = '\0';// 搜索文本
    toolbar->search_active = false;// 搜索是否激活
    toolbar->search_cursor_pos = 0;// 搜索光标位置
    
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
                    
                    // 检查是否点击了搜索框
                    int search_box_w = 220;
                    int search_box_h = BUTTON_SIZE;
                    int search_box_x = toolbar->rect.x + toolbar->rect.w - search_box_w - BUTTON_PADDING;
                    int search_box_y = toolbar->rect.y + (toolbar->rect.h - search_box_h) / 2;
                    
                    if (x >= search_box_x && x < search_box_x + search_box_w &&
                        y >= search_box_y && y < search_box_y + search_box_h) {
                        
                        // 检查是否点击了清除搜索按钮
                        if (toolbar->search_text[0] != '\0') {
                            int clear_button_size = 16;
                            int clear_button_x = search_box_x + search_box_w - clear_button_size - 4;
                            int clear_button_y = search_box_y + (search_box_h - clear_button_size) / 2;
                            
                            if (x >= clear_button_x && x < clear_button_x + clear_button_size &&
                                y >= clear_button_y && y < clear_button_y + clear_button_size) {
                                // 点击了清除搜索按钮
                                toolbar->search_text[0] = '\0';
                                toolbar->search_cursor_pos = 0;
                                toolbar_search(toolbar, "");
                                printf("[DEBUG] 点击清除搜索按钮\n");
                                return true;
                            }
                        }
                        
                        // 点击了搜索框
                        if (toolbar->search_active) {
                            // 如果搜索栏已经激活，不做任何操作
                        } else {
                            // 如果搜索栏未激活，重新激活搜索
                            toolbar_search_start(toolbar);
                            // 保持现有的搜索文本
                        }
                        return true;
                    } else if (toolbar->search_active) {
                        // 点击了搜索框外的区域，停止搜索
                        toolbar_search_stop(toolbar);
                    }
                    
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
            
        case SDL_EVENT_KEY_DOWN:
            // 如果搜索栏激活，处理键盘输入
            if (toolbar->search_active) {
                printf("[DEBUG] 收到按键: %d\n", event->key.scancode);
                toolbar_search_handle_key(toolbar, event->key.scancode);
                return true;
            }
            break;
            
        case SDL_EVENT_TEXT_INPUT:
            // 如果搜索栏激活，处理文本输入
            if (toolbar->search_active) {
                printf("[DEBUG] 收到文本输入: '%s'\n", event->text.text);
                toolbar_search_handle_text(toolbar, event->text.text);
                return true;
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
    // 绘制搜索输入框（右侧，边界分明）
    int search_box_w = 220;
    int search_box_h = BUTTON_SIZE;
    int search_box_x = toolbar->rect.x + toolbar->rect.w - search_box_w - BUTTON_PADDING;
    int search_box_y = toolbar->rect.y + (toolbar->rect.h - search_box_h) / 2;
    SDL_FRect search_box = {
        (float)search_box_x, (float)search_box_y, (float)search_box_w, (float)search_box_h
    };
    
    // 根据搜索状态选择背景颜色
    SDL_Color search_bg_color = toolbar->search_active ? 
        (SDL_Color){255, 255, 240, 255} : (SDL_Color){255, 255, 255, 255};
    SDL_Color search_border_color = toolbar->search_active ? 
        (SDL_Color){0, 120, 215, 255} : (SDL_Color){100, 100, 100, 255};
    
    // 背景
    SDL_SetRenderDrawColor(renderer, search_bg_color.r, search_bg_color.g, search_bg_color.b, search_bg_color.a);
    SDL_RenderFillRect(renderer, &search_box);
    // 边框
    SDL_SetRenderDrawColor(renderer, search_border_color.r, search_border_color.g, search_border_color.b, search_border_color.a);
    SDL_RenderRect(renderer, &search_box);
    // 显示输入内容`
    if (toolbar->search_active || toolbar->search_text[0] != '\0') {
        // 这里只做简单文本渲染，实际可用TTF渲染
        SDL_Color text_color = {30, 30, 30, 255};
        if (toolbar->app->font && toolbar->search_text[0] != '\0') {
            size_t text_len = strlen(toolbar->search_text);
            SDL_Surface *text_surface = TTF_RenderText_Blended(toolbar->app->font, toolbar->search_text, text_len, text_color);
            if (text_surface) {
                SDL_Texture *text_tex = SDL_CreateTextureFromSurface(renderer, text_surface);
                if (text_tex) {
                    SDL_FRect dst = {(float)(search_box_x + 8), (float)(search_box_y + (search_box_h - text_surface->h) / 2), (float)text_surface->w, (float)text_surface->h};
                    SDL_RenderTexture(renderer, text_tex, NULL, &dst);
                    SDL_DestroyTexture(text_tex);
                }
                SDL_DestroySurface(text_surface);
            }
        }
        
        // 如果搜索栏激活，绘制光标
        if (toolbar->search_active) {
            int cursor_x = search_box_x + 8;
            if (toolbar->search_text[0] != '\0' && toolbar->app->font) {
                // 计算光标位置
                char temp_text[256];
                strncpy(temp_text, toolbar->search_text, toolbar->search_cursor_pos);
                temp_text[toolbar->search_cursor_pos] = '\0';
                
                SDL_Surface *cursor_surface = TTF_RenderText_Blended(toolbar->app->font, temp_text, strlen(temp_text), text_color);
                if (cursor_surface) {
                    cursor_x += cursor_surface->w;
                    SDL_DestroySurface(cursor_surface);
                }
            }
            
            // 绘制光标
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderLine(renderer, cursor_x, search_box_y + 4, cursor_x, search_box_y + search_box_h - 4);
        }
        
        // 绘制清除搜索按钮（X）
        if (toolbar->search_text[0] != '\0') {
            int clear_button_size = 16;
            int clear_button_x = search_box_x + search_box_w - clear_button_size - 4;
            int clear_button_y = search_box_y + (search_box_h - clear_button_size) / 2;
            
            // 绘制清除按钮背景（圆形）
            SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
            SDL_FRect clear_button = {(float)clear_button_x, (float)clear_button_y, (float)clear_button_size, (float)clear_button_size};
            SDL_RenderFillRect(renderer, &clear_button);
            
            // 绘制X符号
            SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
            SDL_RenderLine(renderer, 
                          clear_button_x + 4, clear_button_y + 4, 
                          clear_button_x + clear_button_size - 4, clear_button_y + clear_button_size - 4);
            SDL_RenderLine(renderer, 
                          clear_button_x + clear_button_size - 4, clear_button_y + 4, 
                          clear_button_x + 4, clear_button_y + clear_button_size - 4);
        }
    } else {
        // 显示placeholder
        SDL_Color ph_color = {180, 180, 180, 255};
        if (toolbar->app->font) {
            size_t ph_len = strlen("搜索...");
            SDL_Surface *ph_surface = TTF_RenderText_Blended(toolbar->app->font, "搜索...", ph_len, ph_color);
            if (ph_surface) {
                SDL_Texture *ph_tex = SDL_CreateTextureFromSurface(renderer, ph_surface);
                if (ph_tex) {
                    SDL_FRect dst = {(float)(search_box_x + 8), (float)(search_box_y + (search_box_h - ph_surface->h) / 2), (float)ph_surface->w, (float)ph_surface->h};
                    SDL_RenderTexture(renderer, ph_tex, NULL, &dst);
                    SDL_DestroyTexture(ph_tex);
                }
                SDL_DestroySurface(ph_surface);
            }
        }
    }
}

void toolbar_search_start(Toolbar *toolbar) {
    if (!toolbar) return;
    printf("[DEBUG] 搜索栏激活\n");
    toolbar->search_active = true;
    
    // 不清除现有搜索文本，保持用户之前的搜索内容
    // 如果搜索文本为空，设置光标位置为0
    if (toolbar->search_text[0] == '\0') {
        toolbar->search_cursor_pos = 0;
    } else {
        // 如果已有搜索文本，将光标移到文本末尾
        toolbar->search_cursor_pos = strlen(toolbar->search_text);
    }
    
    // 启用SDL文本输入模式，传入窗口参数
    if (toolbar->app && toolbar->app->window) {
        SDL_StartTextInput(toolbar->app->window);
        printf("[DEBUG] SDL文本输入已启用\n");
    } else {
        printf("[WARNING] 无法启用SDL文本输入：窗口指针无效\n");
    }
}
void toolbar_search_stop(Toolbar *toolbar) {
    if (!toolbar) return;
    printf("[DEBUG] 搜索栏停止\n");
    toolbar->search_active = false;
    
    // 停止SDL文本输入模式，传入窗口参数
    if (toolbar->app && toolbar->app->window) {
        SDL_StopTextInput(toolbar->app->window);
        printf("[DEBUG] SDL文本输入已停止\n");
    } else {
        printf("[WARNING] 无法停止SDL文本输入：窗口指针无效\n");
    }
    
    // 不清除搜索文本，保持搜索结果可见
    // 用户可以通过点击搜索框重新激活搜索，或者通过其他方式清除搜索
}
void toolbar_search_handle_text(Toolbar *toolbar, const char *text) {
    if (!toolbar || !toolbar->search_active || !text) return;
    printf("[DEBUG] 处理文本输入: '%s', 当前文本: '%s', 光标位置: %d\n", 
           text, toolbar->search_text, toolbar->search_cursor_pos);
    
    size_t len = strlen(toolbar->search_text);
    size_t tlen = strlen(text);
    if (len + tlen < sizeof(toolbar->search_text) - 1) {
        // 在光标位置插入文本
        memmove(&toolbar->search_text[toolbar->search_cursor_pos + tlen], 
               &toolbar->search_text[toolbar->search_cursor_pos], 
               len - toolbar->search_cursor_pos + 1);
        memcpy(&toolbar->search_text[toolbar->search_cursor_pos], text, tlen);
        toolbar->search_cursor_pos += tlen;
        printf("[DEBUG] 文本更新后: '%s', 光标位置: %d\n", 
               toolbar->search_text, toolbar->search_cursor_pos);
        
        // 实时更新搜索过滤
        toolbar_search(toolbar, toolbar->search_text);
    }
}
void toolbar_search_handle_key(Toolbar *toolbar, SDL_Scancode scancode) {
    if (!toolbar || !toolbar->search_active) return;
    size_t len = strlen(toolbar->search_text);
    switch (scancode) {
        case SDL_SCANCODE_BACKSPACE:
            if (toolbar->search_cursor_pos > 0 && len > 0) {
                // 从光标位置向前删除一个字符
                memmove(&toolbar->search_text[toolbar->search_cursor_pos-1], 
                       &toolbar->search_text[toolbar->search_cursor_pos], 
                       len - toolbar->search_cursor_pos + 1);
                toolbar->search_cursor_pos--;
                
                // 实时更新搜索过滤
                toolbar_search(toolbar, toolbar->search_text);
            }
            break;
        case SDL_SCANCODE_RETURN:
            // 执行搜索
            if (toolbar->search_text[0] != '\0') {
                printf("[DEBUG] 执行搜索: %s\n", toolbar->search_text);
                // 调用实际的搜索功能
                toolbar_search(toolbar, toolbar->search_text);
            }
            toolbar_search_stop(toolbar);
            break;
        case SDL_SCANCODE_ESCAPE:
            toolbar_search_stop(toolbar);
            break;
        case SDL_SCANCODE_LEFT:
            if (toolbar->search_cursor_pos > 0) {
                toolbar->search_cursor_pos--;
            }
            break;
        case SDL_SCANCODE_RIGHT:
            if (toolbar->search_cursor_pos < len) {
                toolbar->search_cursor_pos++;
            }
            break;
        case SDL_SCANCODE_L:
            // Ctrl+L 清除搜索
            if (SDL_GetModState() & SDL_KMOD_CTRL) {
                toolbar->search_text[0] = '\0';
                toolbar->search_cursor_pos = 0;
                toolbar_search(toolbar, "");
                printf("[DEBUG] 清除搜索\n");
            }
            break;
        default:
            break;
    }
}

// 实现工具栏搜索功能
bool toolbar_search(Toolbar *toolbar, const char *search_term) {
    if (!toolbar || !toolbar->app || !toolbar->app->user_data) {
        return false;
    }
    
    MainWindow *main_window = (MainWindow*)toolbar->app->user_data;
    if (!main_window || !main_window->file_list_view) {
        return false;
    }
    
    printf("[DEBUG] 工具栏执行搜索: '%s'\n", search_term);
    
    // 调用文件列表视图的搜索功能
    if (search_term && strlen(search_term) > 0) {
        file_list_view_set_search_filter(main_window->file_list_view, search_term);
    } else {
        file_list_view_clear_search_filter(main_window->file_list_view);
    }
    
    return true;
}

