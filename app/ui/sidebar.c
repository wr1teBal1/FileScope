/*
 * 侧边栏模块
 * 职责：
 * 1. 实现侧边栏界面
 * 2. 显示快速访问项（桌面、下载、文档等）
 * 3. 显示驱动器和设备列表
 * 4. 处理侧边栏项目的选择和导航
 */

#include "sidebar.h"
#include "renderer.h"
#include "file_system.h"
#include "path_resolver.h"
#include "toolbar.h"
#include <stdlib.h>
#include <string.h>
#include <SDL3_image/SDL_image.h>

// 侧边栏项目高度
#define SIDEBAR_ITEM_HEIGHT 30
// 侧边栏项目边距
#define SIDEBAR_ITEM_PADDING 5
// 侧边栏分隔线高度
#define SIDEBAR_SEPARATOR_HEIGHT 1

// 前向声明私有函数
static void sidebar_add_quick_access_items(Sidebar *sidebar); 
static void sidebar_add_drives(Sidebar *sidebar);
static void sidebar_add_separator(Sidebar *sidebar);
static void sidebar_add_item(Sidebar *sidebar, SidebarItemType type, const char *name, const char *path);
static void sidebar_draw_item(Sidebar *sidebar, int index);
static int sidebar_get_item_at(Sidebar *sidebar, int x, int y);
static SDL_Texture* load_icon(Sidebar *sidebar, SidebarItemType type);  //加载图标
static bool get_special_folder_path_wrapper(SpecialFolder folder, char *path, size_t path_size);

// 创建侧边栏
Sidebar* sidebar_new(struct Window *app) {
    if (!app) {
        return NULL;
    }   // 检查app是否为空

    Sidebar *sidebar = (Sidebar*)calloc(1, sizeof(Sidebar)); // 使用 calloc 分配内存，calloc 会将内存初始化为0
    if (!sidebar) {
        return NULL;
    }   // 检查sidebar是否为空

    sidebar->app = app;    // 将app赋值给sidebar
    sidebar->rect.x = 0;   // 设置x坐标
    sidebar->rect.y = TOOLBAR_HEIGHT; // 从工具栏底部开始
    sidebar->rect.w = SIDEBAR_WIDTH;  // 设置宽度
    sidebar->rect.h = SDL_WINDOW_HEIGHT - TOOLBAR_HEIGHT; // 调整高度，减去工具栏高度
    sidebar->item_count = 0;   // 项目数量初始化为0
    sidebar->selected_index = -1;   // 选中项索引初始化为-1（无选中）
    sidebar->hover_index = -1;   // 悬停项索引初始化为-1（无悬停）
    sidebar->scroll_offset = 0;   // 滚动偏移量初始化为0
    
    // 设置颜色
    sidebar->bg_color = (SDL_Color){240, 240, 245, 255};         // 浅灰色背景
    sidebar->text_color = (SDL_Color){50, 50, 50, 255};          // 深灰色文本
    sidebar->hover_color = (SDL_Color){220, 220, 230, 255};      // 悬停颜色
    sidebar->selected_color = (SDL_Color){200, 200, 225, 255};   // 选中颜色
    sidebar->separator_color = (SDL_Color){200, 200, 200, 255};  // 分隔线颜色

    // 添加快速访问项
    sidebar_add_quick_access_items(sidebar);
    
    // 添加分隔线
    sidebar_add_separator(sidebar);
    
    // 添加驱动器列表
    sidebar_add_drives(sidebar);

    return sidebar;          //返回创建的侧边栏
}

// 释放侧边栏
void sidebar_free(Sidebar *sidebar) {
    if (!sidebar) {
        return;   // 如果侧边栏为空，直接返回
    }
    
    // 释放项目资源
    for (int i = 0; i < sidebar->item_count; i++) {
        if (sidebar->items[i].name) {
            free(sidebar->items[i].name);   // 释放项目名称
        }
        if (sidebar->items[i].path) {
            free(sidebar->items[i].path);   // 释放项目路径
        }
        if (sidebar->items[i].icon) {
            SDL_DestroyTexture(sidebar->items[i].icon);   //使用 SDL_DestroyTexture() 释放SDL纹理资源
        }
    }
        
        free(sidebar);   // 释放侧边栏本身
//         为什么需要这行代码
// 1. 防止内存泄漏
// 如果不调用 free(sidebar)，这块内存永远不会被释放
// 程序运行时间越长，泄漏的内存越多
// 2. 资源完整性
// 确保所有分配的资源都被正确释放
// 包括结构体本身和其包含的所有子资源
// 3. 系统稳定性
// 及时释放不需要的内存
// 避免系统内存耗尽
// 类比理解
// 想象一下：
// calloc 就像向银行借钱（分配内存）
// 使用内存就像使用借来的钱
// free 就像还钱给银行（释放内存）
// 如果不还钱（不调用 free），银行就会一直记着这笔债（内存泄漏），最终可能导致银行破产（系统内存耗尽）。
// 所以 free(sidebar) 是内存管理的最后一步，确保程序"借"来的所有内存都被正确"归还"。
}

// 处理侧边栏事件
bool sidebar_handle_event(Sidebar *sidebar, SDL_Event *event) { // 事件处理
    if (!sidebar || !event) return false; // 参数检查
    
    switch (event->type) {
        // 鼠标移动事件
        case SDL_EVENT_MOUSE_MOTION: {
            int x = event->motion.x;
            int y = event->motion.y;// 鼠标坐标
            
            // 检查鼠标是否在侧边栏边界内
            if (x >= sidebar->rect.x && x < sidebar->rect.x + sidebar->rect.w &&
                y >= sidebar->rect.y && y < sidebar->rect.y + sidebar->rect.h) {
                
                int item_index = sidebar_get_item_at(sidebar, x, y); // 获取项目索引
                
                // 如果鼠标悬停在其他项目上，则更新悬停项索引
                if (item_index != sidebar->hover_index) {
                    sidebar->hover_index = item_index;
                    return true;
                }
            } else {
                   // 如果鼠标不在侧边栏边界内，则清除悬停项索引
                    sidebar->hover_index = -1;
                    return true;
                
            }
            break;
        }
        // 鼠标点击事件
        case SDL_EVENT_MOUSE_BUTTON_DOWN: {
            if (event->button.button == SDL_BUTTON_LEFT) {
                int x = event->button.x;
                int y = event->button.y;
                
                // 检查鼠标是否在侧边栏边界内
                if (x >= sidebar->rect.x && x < sidebar->rect.x + sidebar->rect.w &&
                    y >= sidebar->rect.y && y < sidebar->rect.y + sidebar->rect.h) {
                    
                        // 获取项目索引
                    int item_index = sidebar_get_item_at(sidebar, x, y);

                    if (item_index >= 0 && item_index < sidebar->item_count) {
                        sidebar->selected_index = item_index; // 设置选中项索引
                        
                        // 如果设置了回调函数则调用
                        if (sidebar->on_item_selected && sidebar->items[item_index].path) {
                            //双重检查，1.防止回调函数为空 2.防止项目路径为空
                            // 回调函数的作用是通知主窗口切换到指定路径
                            sidebar->on_item_selected(sidebar, sidebar->items[item_index].path);
                            //1传递侧边栏本身 2传递项目路径
                        }
                        return true;
                    }
                }
            }
            break;
        }
        // 鼠标滚动事件
        case SDL_EVENT_MOUSE_WHEEL: {
           if(event->wheel.y != 0)

           //计算滚动的距离
           sidebar->scroll_offset += event->wheel.y * 20; // 20为滚动速度，可以根据需要调整

           //限制滚动范围
           int max_scroll = sidebar->item_count * SIDEBAR_ITEM_HEIGHT - sidebar->rect.h;
           //内容总高度 − 可见区域高度。如果内容比可见区域还短会得到负数，所以立刻设为 0（表示不需要滚动）。
           if(max_scroll  < 0)  max_scroll = 0; 

           if(sidebar->scroll_offset > max_scroll ) sidebar->scroll_offset = max_scroll ;// 限制最大滚动距离
           if(sidebar->scroll_offset < 0) sidebar->scroll_offset = 0;// 限制最小滚动距离

           return true;
        }
        break;
    }
    
    return false;
}

// 绘制侧边栏
    void sidebar_draw(Sidebar *sidebar){
        //先检查三个参数
        if(!sidebar||!sidebar->app||!sidebar->app->renderer)
        {
            return;
        }
    
    //获取渲染器 
     SDL_Renderer *renderer = sidebar->app->renderer;
    
    // 绘制侧边栏背景
    SDL_SetRenderDrawColor(renderer, 
                          sidebar->bg_color.r, 
                          sidebar->bg_color.g, 
                          sidebar->bg_color.b, 
                          sidebar->bg_color.a);
                          //颜色确定
    SDL_FRect frect = {
        (float)sidebar->rect.x,
        (float)sidebar->rect.y,
        (float)sidebar->rect.w,
        (float)sidebar->rect.h
    }; //绘制浮点矩形
    SDL_RenderFillRect(renderer, &frect);
    
     // 绘制侧边栏右边框
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    SDL_RenderLine(renderer, 
                  (float)(sidebar->rect.x + sidebar->rect.w - 1), 
                  (float)sidebar->rect.y, 
                  (float)(sidebar->rect.x + sidebar->rect.w - 1), 
                  (float)(sidebar->rect.y + sidebar->rect.h));
    
    // 绘制侧边栏项目
    for (int i = 0; i < sidebar->item_count; i++) {
        sidebar_draw_item(sidebar, i);
    }
}

// 设置项目选中回调
void sidebar_set_item_selected_callback(Sidebar *sidebar, SidebarItemSelectedCallback callback) {
    if (sidebar) {
        sidebar->on_item_selected = callback;// 设置回调函数
    }
}

// 添加快速访问项
static void sidebar_add_quick_access_items(Sidebar *sidebar) {
    if (!sidebar) {
        return;
    }
    
    // 添加桌面
    char desktop_path[256];
    if (get_special_folder_path_wrapper(FOLDER_DESKTOP, desktop_path, sizeof(desktop_path))) {
        sidebar_add_item(sidebar, SIDEBAR_ITEM_QUICK_ACCESS, "桌面", desktop_path);
    }
    
    // 添加文档
    char documents_path[256];
    if (get_special_folder_path_wrapper(FOLDER_DOCUMENTS, documents_path, sizeof(documents_path))) {
        sidebar_add_item(sidebar, SIDEBAR_ITEM_QUICK_ACCESS, "文档", documents_path);
    }
    
    // 添加下载
    char downloads_path[256];
    if (get_special_folder_path_wrapper(FOLDER_DOWNLOADS, downloads_path, sizeof(downloads_path))) {
        sidebar_add_item(sidebar, SIDEBAR_ITEM_QUICK_ACCESS, "下载", downloads_path);
    }
    
    // 添加图片
    char pictures_path[256];
    if (get_special_folder_path_wrapper(FOLDER_PICTURES, pictures_path, sizeof(pictures_path))) {
        sidebar_add_item(sidebar, SIDEBAR_ITEM_QUICK_ACCESS, "图片", pictures_path);
    }
    
    // 添加音乐
    char music_path[256];
    if (get_special_folder_path_wrapper(FOLDER_MUSIC, music_path, sizeof(music_path))) {
        sidebar_add_item(sidebar, SIDEBAR_ITEM_QUICK_ACCESS, "音乐", music_path);
    }
    
    // 添加视频
    char videos_path[256];
    if (get_special_folder_path_wrapper(FOLDER_VIDEOS, videos_path, sizeof(videos_path))) {
        sidebar_add_item(sidebar, SIDEBAR_ITEM_QUICK_ACCESS, "视频", videos_path);
    }
}

// 添加驱动器列表
static void sidebar_add_drives(Sidebar *sidebar) {
    if (!sidebar) {
        return;
    }
    
    // 获取驱动器列表
    DriveInfo drives[26];// 最多支持26个驱动器
    int drive_count = get_drives(drives, 26);
    
    for (int i = 0; i < drive_count; i++) {
        char drive_name[64];
        
        // 格式化驱动器名称
        if (drives[i].label[0] != '\0') {
            snprintf(drive_name, sizeof(drive_name), "%c: (%s)", drives[i].letter, drives[i].label);
        } else {
            snprintf(drive_name, sizeof(drive_name), "%c:", drives[i].letter);
        }
        
        // 格式化驱动器路径
        char drive_path[4];
        snprintf(drive_path, sizeof(drive_path), "%c:", drives[i].letter);
        
        // 添加驱动器项
        sidebar_add_item(sidebar, SIDEBAR_ITEM_DRIVE, drive_name, drive_path);
    }
}

// 添加分隔线
static void sidebar_add_separator(Sidebar *sidebar) {
    if (!sidebar || sidebar->item_count >= MAX_SIDEBAR_ITEMS) {
        return;
    }// 检查参数
    
    int index =  sidebar->item_count ++  ;                   // 获取索引
    SidebarItem *item = &sidebar->items[index];                // 获取项指针   
    
    item->type = SIDEBAR_ITEM_SEPARATOR;                              // 设置类型为分隔线
    item->name = NULL;                                                // 名称为空
    item->path = NULL;                                                // 路径为空
    item->icon = NULL;                                                // 图标为空
    item->state = SIDEBAR_STATE_NORMAL;                              // 设置状态为正常
    
    // 设置分隔线区域
    item->rect.x = sidebar->rect.x;
    item->rect.y = sidebar->rect.y + SIDEBAR_ITEM_PADDING + index * SIDEBAR_ITEM_HEIGHT; // 计算Y坐标
    item->rect.w = sidebar->rect.w;                                                    
    item->rect.h = SIDEBAR_SEPARATOR_HEIGHT;
}

// 加载图标
static SDL_Texture* load_icon(Sidebar *sidebar, SidebarItemType type) {
    if (!sidebar || !sidebar->app || !sidebar->app->renderer) {
        return NULL;
    }
    
    SDL_Renderer *renderer = sidebar->app->renderer; // 获取渲染器
    SDL_Texture *texture = NULL;                      // 图标纹理
    
    // 根据项目类型加载不同的图标
    const char *icon_path = NULL;
    
    switch (type) {
        case SIDEBAR_ITEM_QUICK_ACCESS:  // 快速访问图标
            icon_path = "assets/icons/folder.png";
            break;
        case SIDEBAR_ITEM_DRIVE:  // 驱动器图标
            icon_path = "assets/icons/drive.png";
            break;
        default:
            return NULL;
    }
    
    // 加载图标
    SDL_Surface *surface = IMG_Load(icon_path);
    if (!surface) {
        // 如果找不到图标，创建一个简单的彩色方块
        surface = SDL_CreateSurface(16, 16, SDL_PIXELFORMAT_RGBA32);
        if (!surface) {
            return NULL;
        }
        
        // 根据类型设置不同的颜色
        Uint32 color;
        const SDL_PixelFormatDetails *format_details = SDL_GetPixelFormatDetails(surface->format);
        if (type == SIDEBAR_ITEM_QUICK_ACCESS) {
            color = SDL_MapRGBA(format_details, NULL, 255, 200, 0, 255);  // 黄色文件夹
        } else {
            color = SDL_MapRGBA(format_details, NULL, 100, 150, 200, 255); // 蓝色驱动器
        }
        
        // 填充颜色
        SDL_FillSurfaceRect(surface, NULL, color);
    }
    
    // 创建纹理
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    
    // 释放表面
    SDL_DestroySurface(surface);
    
    return texture;
}

// 添加侧边栏项目
static void sidebar_add_item(Sidebar *sidebar, SidebarItemType type, const char *name, const char *path) {
    if (!sidebar || !name || !path || sidebar->item_count >= MAX_SIDEBAR_ITEMS) {
        return;
    }
    
    int index = sidebar->item_count++;
    SidebarItem *item = &sidebar->items[index];
    
    item->type = type;
    item->name = strdup(name);
    item->path = strdup(path);
    item->icon = load_icon(sidebar, type); // 加载图标
    item->state = SIDEBAR_STATE_NORMAL;
    
    // 设置项目区域
    item->rect.x = sidebar->rect.x;
    item->rect.y = sidebar->rect.y + SIDEBAR_ITEM_PADDING + index * SIDEBAR_ITEM_HEIGHT;
    item->rect.w = sidebar->rect.w;
    item->rect.h = SIDEBAR_ITEM_HEIGHT;
}

// 绘制侧边栏项目
static void sidebar_draw_item(Sidebar *sidebar, int index) {
    if (!sidebar || index < 0 || index >= sidebar->item_count) {
        return;
    }
    
    SDL_Renderer *renderer = sidebar->app->renderer;
    SidebarItem *item = &sidebar->items[index];
    
    // 计算项目的实际Y坐标（考虑滚动）
    int y = item->rect.y - sidebar->scroll_offset;
    
    // 如果项目不在可见区域内，则跳过绘制
    if (y + item->rect.h < sidebar->rect.y || y >= sidebar->rect.y + sidebar->rect.h) {
        return;
    }
    
    // 创建绘制区域
    SDL_Rect draw_rect = {
        item->rect.x,
        y,
        item->rect.w,
        item->rect.h
    };
    
    // 根据项目类型绘制
    if (item->type == SIDEBAR_ITEM_SEPARATOR) {
        // 绘制分隔线
        SDL_SetRenderDrawColor(renderer, 
                              sidebar->separator_color.r, 
                              sidebar->separator_color.g, 
                              sidebar->separator_color.b, 
                              sidebar->separator_color.a);
        SDL_RenderLine(renderer, 
                      (float)(draw_rect.x + SIDEBAR_ITEM_PADDING), 
                      (float)draw_rect.y, 
                      (float)(draw_rect.x + draw_rect.w - SIDEBAR_ITEM_PADDING * 2), 
                      (float)draw_rect.y);
    } else {
        // 绘制项目背景
        SDL_Color bg_color;
        
        if (index == sidebar->selected_index) {
            // 选中项背景
            bg_color = sidebar->selected_color;
        } else if (index == sidebar->hover_index) {
            // 悬停项背景
            bg_color = sidebar->hover_color;
        } else {
            // 普通项背景
            bg_color = sidebar->bg_color;
        }
        
        SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, bg_color.a);
        SDL_FRect fdraw_rect = {
            (float)draw_rect.x,
            (float)draw_rect.y,
            (float)draw_rect.w,
            (float)draw_rect.h
        };
        SDL_RenderFillRect(renderer, &fdraw_rect);
        
        // 绘制图标（如果有）
        if (item->icon) {
            SDL_FRect icon_rect = {
                (float)(draw_rect.x + SIDEBAR_ITEM_PADDING),
                (float)(draw_rect.y + (draw_rect.h - 16) / 2), // 假设图标大小为16x16
                16.0f,
                16.0f
            };
            SDL_RenderTexture(renderer, item->icon, NULL, &icon_rect);
        }
        
        // 绘制文本
        if (item->name) {
            // 创建文本表面
            SDL_Surface *text_surface = TTF_RenderText_Blended(
                sidebar->app->font,
                item->name,
                strlen(item->name),
                sidebar->text_color
            );
            
            if (text_surface) {
                // 创建文本纹理
                SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
                
                if (text_texture) {
                    // 计算文本位置
                    SDL_FRect text_rect;
                    text_rect.x = (float)(draw_rect.x + SIDEBAR_ITEM_PADDING + (item->icon ? 20 : 0)); // 如果有图标，则文本右移
                    text_rect.y = (float)(draw_rect.y + (draw_rect.h - text_surface->h) / 2);
                    text_rect.w = (float)text_surface->w;
                    text_rect.h = (float)text_surface->h;
                    
                    // 绘制文本
                    SDL_RenderTexture(renderer, text_texture, NULL, &text_rect);
                    
                    // 释放纹理
                    SDL_DestroyTexture(text_texture);
                }
                
                // 释放表面
                SDL_DestroySurface(text_surface);
            }
        }
    }
}

// 获取指定坐标处的项目索引
static int sidebar_get_item_at(Sidebar *sidebar, int x, int y) {
    if (!sidebar) {
        return -1;
    }
    
    // 考虑滚动偏移
    y += sidebar->scroll_offset;
    
    // 遍历所有项目
    for (int i = 0; i < sidebar->item_count; i++) {
        SidebarItem *item = &sidebar->items[i];
        
        // 检查坐标是否在项目区域内
        if (x >= item->rect.x && x < item->rect.x + item->rect.w &&
            y >= item->rect.y && y < item->rect.y + item->rect.h) {
            return i;
        }
    }
    
    return -1;
}

// 获取特殊文件夹路径的包装函数
static bool get_special_folder_path_wrapper(SpecialFolder folder, char *path, size_t path_size) {
    if (!path || path_size == 0) {
        return false;
    }
    
    // 使用路径解析器处理虚拟路径
    const char *virtual_path = NULL;
    switch (folder) {
        case FOLDER_DESKTOP:
            virtual_path = "桌面";
            break;
        case FOLDER_DOCUMENTS:
            virtual_path = "我的文档";
            break;
        case FOLDER_DOWNLOADS:
            virtual_path = "下载";
            break;
        case FOLDER_PICTURES:
            virtual_path = "图片";
            break;
        case FOLDER_MUSIC:
            virtual_path = "音乐";
            break;
        case FOLDER_VIDEOS:
            virtual_path = "视频";
            break;
        default:
            return false;
    }
    
    // 解析虚拟路径
    PathInfo *info = path_resolve(virtual_path, NULL);
    if (!info || !info->is_valid) {
        if (info) {
            path_info_free(info);
        }
        // 如果路径解析失败，回退到原来的方法
        return get_special_folder_path(folder, path, path_size);
    }
    
    // 复制解析后的路径
    strncpy(path, info->absolute_path, path_size - 1);
    path[path_size - 1] = '\0';
    
    path_info_free(info);
    return true;
}

// 刷新驱动器列表
void sidebar_refresh_drives(Sidebar *sidebar) {
    if (!sidebar) {
        return;
    }
    
    // 查找第一个分隔线的位置
    int separator_index = -1;
    for (int i = 0; i < sidebar->item_count; i++) {
        if (sidebar->items[i].type == SIDEBAR_ITEM_SEPARATOR) {
            separator_index = i;
            break;
        }
    }
    
    if (separator_index < 0) {
        // 如果没有找到分隔线，则添加一个
        sidebar_add_separator(sidebar);
        separator_index = sidebar->item_count - 1;
    }
    
    // 移除所有驱动器项目
    int i = separator_index + 1;
    while (i < sidebar->item_count) {
        if (sidebar->items[i].type == SIDEBAR_ITEM_DRIVE) {
            // 释放资源
            if (sidebar->items[i].name) {
                free(sidebar->items[i].name);
            }
            if (sidebar->items[i].path) {
                free(sidebar->items[i].path);
            }
            if (sidebar->items[i].icon) {
                SDL_DestroyTexture(sidebar->items[i].icon);
            }
            
            // 移动后面的项目
            for (int j = i; j < sidebar->item_count - 1; j++) {
                sidebar->items[j] = sidebar->items[j + 1];
            }
            
            // 减少项目计数
            sidebar->item_count--;
        } else {
            i++;
        }
    }
    
    // 添加驱动器列表
    sidebar_add_drives(sidebar);
    
    // 更新项目位置
    for (int i = 0; i < sidebar->item_count; i++) {
        sidebar->items[i].rect.x = sidebar->rect.x;
        sidebar->items[i].rect.y = sidebar->rect.y + SIDEBAR_ITEM_PADDING + i * SIDEBAR_ITEM_HEIGHT;
    }
}
