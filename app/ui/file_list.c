/*
 * 文件列表视图模块
 * 职责：
 * 1. 实现文件列表显示（详细信息、图标、列表等视图模式）
 * 2. 文件项的选择和高亮
 * 3. 拖放操作支持
 * 4. 右键菜单支持
 * 5. 文件排序和过滤
 */

#include "file_list.h"
#include "file_item.h"
#include "renderer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#ifndef PATH_MAX
#define PATH_MAX 260  // Windows默认最大路径长度
#endif
#else
#include <unistd.h>
#endif

// 默认项目尺寸
#define DEFAULT_ITEM_WIDTH 100
#define DEFAULT_ITEM_HEIGHT 80
#define DEFAULT_LIST_ITEM_HEIGHT 30

// 图标路径
#define FOLDER_ICON_PATH "images/folder.png"
#define FILE_ICON_PATH "images/file.png"

// Helper functions for formatting file size and time
static void format_file_size(uint64_t size, char *buffer, size_t buffer_size) {
    const char *units[] = {"B", "KB", "MB", "GB", "TB"};
    int unit_index = 0;
    double size_value = (double)size;
    
    while (size_value >= 1024.0 && unit_index < 4) {
        size_value /= 1024.0;
        unit_index++;
    }
    
    if (unit_index == 0) {
        snprintf(buffer, buffer_size, "%d %s", (int)size_value, units[unit_index]);
    } else {
        snprintf(buffer, buffer_size, "%.2f %s", size_value, units[unit_index]);
    }
}

static void format_time(time_t time_value, char *buffer, size_t buffer_size) {
    struct tm *time_info = localtime(&time_value);
    if (time_info) {
        strftime(buffer, buffer_size, "%Y-%m-%d %H:%M", time_info);
    } else {
        strncpy(buffer, "Unknown", buffer_size - 1);
        buffer[buffer_size - 1] = '\0';
    }
}

// 目录变更回调函数类型 - 用于通知toolbar


// toolbar会调用的函数:
// 加载目录
bool file_list_view_load_directory(FileListView *view, const char *path);

// 返回上级目录
void file_list_view_go_up(FileListView *view);

// 设置视图模式
void file_list_view_set_mode(FileListView *view, ViewMode mode);

// 刷新文件列表
void file_list_view_refresh(FileListView *view);

// 创建文件列表视图
FileListView* file_list_view_new(struct Window *window) {
    if (!window) {
        return NULL;
    }

    FileListView *view = (FileListView*)calloc(1, sizeof(FileListView));
    if (!view) {
        return NULL;
    }

    // 初始化成员
    view->window = window;
    view->files = file_list_new();
    if (!view->files) {
        free(view);
        return NULL;
    }

    // 设置默认值
    view->view_mode = VIEW_MODE_ICONS;
    view->sort_mode = SORT_BY_NAME;
    view->show_hidden = false;
    view->scroll_offset_y = 0;
    view->item_width = DEFAULT_ITEM_WIDTH;
    view->item_height = DEFAULT_ITEM_HEIGHT;
    view->selected_index = -1;
    
    view->current_path = NULL;
    
    view->on_right_click = NULL;
    view->on_directory_changed = NULL;
    
    // 设置视口区域（默认为整个窗口）
    view->viewport.x = 0;
    view->viewport.y = 0;
    view->viewport.w = SDL_WINDOW_WIDTH;
    view->viewport.h = SDL_WINDOW_HEIGHT;

    // 加载图标
    if (!file_list_view_load_icons(view)) {
        printf("警告：无法加载文件图标\n");
    }

    return view;
}

// 释放文件列表视图
void file_list_view_free(FileListView *view) {
    if (!view) {
        return;
    }

    // 释放文件列表
    if (view->files) {
        file_list_free(view->files);
    }

    // 释放当前路径
    if (view->current_path) {
        free(view->current_path);
    }

    // 释放图标纹理
    if (view->folder_icon) {
        SDL_DestroyTexture(view->folder_icon);
    }
    if (view->file_icon) {
        SDL_DestroyTexture(view->file_icon);
    }

    free(view);
}

// 加载目录
bool file_list_view_load_directory(FileListView *view, const char *path) {
    if (!view || !path) {
        return false;
    }

    // 重置滚动位置和选择
    view->scroll_offset_y = 0;
    view->selected_index = -1;

    // 加载目录内容
    bool result = file_list_load_directory(view->files, path);
    
    // 如果成功加载，保存当前路径并通知目录变更
    if (result) {
        // 更新当前路径
        if (view->current_path) {
            free(view->current_path);
        }
        view->current_path = strdup(path);
        
        // 调用目录变更回调
        if (view->on_directory_changed) {
            view->on_directory_changed(view, path);
        }
    }
    
    return result;
}

// 加载图标
bool file_list_view_load_icons(FileListView *view) {
    if (!view || !view->window || !view->window->renderer) {
        return false;
    }

    // 加载文件夹图标
    SDL_Surface *folder_surface = IMG_Load(FOLDER_ICON_PATH);
    if (folder_surface) {
        view->folder_icon = SDL_CreateTextureFromSurface(view->window->renderer, folder_surface);
        SDL_DestroySurface(folder_surface);
    } else {
        printf("Warning: Failed to load folder icon from %s\n", FOLDER_ICON_PATH);
        view->folder_icon = NULL;
    }

    // 加载文件图标
    SDL_Surface *file_surface = IMG_Load(FILE_ICON_PATH);
    if (file_surface) {
        view->file_icon = SDL_CreateTextureFromSurface(view->window->renderer, file_surface);
        SDL_DestroySurface(file_surface);
    } else {
        printf("Warning: Failed to load file icon from %s\n", FILE_ICON_PATH);
        view->file_icon = NULL;
    }

    return (view->folder_icon != NULL && view->file_icon != NULL);
}

// 设置视图模式
void file_list_view_set_mode(FileListView *view, ViewMode mode) {
    if (!view) {
        return;
    }

    view->view_mode = mode;
    
    // 根据视图模式调整项目高度
    if (mode == VIEW_MODE_ICONS) {
        view->item_height = DEFAULT_ITEM_HEIGHT;
    } else {
        view->item_height = DEFAULT_LIST_ITEM_HEIGHT;
    }
}

// 设置排序方式
void file_list_view_set_sort(FileListView *view, SortMode sort) {
    if (!view) {
        return;
    }

    view->sort_mode = sort;
    file_list_view_refresh(view);
}

// 设置是否显示隐藏文件
void file_list_view_set_show_hidden(FileListView *view, bool show_hidden) {
    if (!view) {
        return;
    }

    view->show_hidden = show_hidden;
    file_list_view_refresh(view);
}

// 刷新文件列表
void file_list_view_refresh(FileListView *view) {
    if (!view || !view->files || !view->files->current_dir) {
        return;
    }

    // 保存当前选中项的路径（如果有）
    char *selected_path = NULL;
    FileItem *selected_item = file_list_view_get_selected_item(view);
    if (selected_item) {
        selected_path = strdup(selected_item->path);
    }

    // 重新加载当前目录
    file_list_load_directory(view->files, view->files->current_dir);

    // 应用排序
    // TODO: 实现排序功能

    // 恢复选中状态（如果可能）
    if (selected_path) {
        int index = 0;
        FileItem *item = view->files->head;
        while (item) {
            if (strcmp(item->path, selected_path) == 0) {
                file_list_view_select_item(view, index);
                break;
            }
            item = item->next;
            index++;
        }
        free(selected_path);
    }
}

// 绘制文件列表
void file_list_view_draw(FileListView *view) {
    if (!view || !view->window || !view->window->renderer || !view->files) {
        return;
    }
    
    // 获取渲染器和字体
    SDL_Renderer *renderer = view->window->renderer;
    TTF_Font *font = view->window->font;
    
    // 文本颜色
    SDL_Color text_color = {0, 0, 0, 255};
    SDL_Color selected_text_color = {255, 255, 255, 255};
    SDL_Color selected_bg_color = {0, 120, 215, 255};
    
    // 设置裁剪区域，只在视口内绘制
    SDL_Rect viewportRect = {
        view->viewport.x,
        view->viewport.y,
        view->viewport.w,
        view->viewport.h
    };
    SDL_SetRenderClipRect(renderer, &viewportRect);
    
    // 绘制背景
    SDL_FRect bg_rect = {
        (float)view->viewport.x, 
        (float)view->viewport.y, 
        (float)view->viewport.w, 
        (float)view->viewport.h
    };
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &bg_rect);
    
    // 如果没有文件，显示空目录提示
    if (!view->files->head) {
        // 设置文本颜色
        SDL_Color empty_color = {128, 128, 128, 255};
        const char* empty_text = "Empty folder";
        size_t text_len = strlen(empty_text);
        
        // 渲染"空目录"文本
        SDL_Surface *text_surface = TTF_RenderText_Blended(font, empty_text, text_len, empty_color);
        if (text_surface) {
            SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
            if (text_texture) {
                SDL_FRect text_rect = {
                    0, 0, 
                    (float)text_surface->w, 
                    (float)text_surface->h
                };
                text_rect.x = (float)view->viewport.x + ((float)view->viewport.w - text_rect.w) / 2;
                text_rect.y = (float)view->viewport.y + ((float)view->viewport.h - text_rect.h) / 2;
                
                SDL_RenderTexture(renderer, text_texture, NULL, &text_rect);
                SDL_DestroyTexture(text_texture);
            }
            SDL_DestroySurface(text_surface);
        }
        
        // 重置裁剪区域
        SDL_SetRenderClipRect(renderer, NULL);
        return;
    }

    // 根据视图模式绘制文件列表
    if (view->view_mode == VIEW_MODE_ICONS) {
        // 图标视图
        int x = (int)view->viewport.x + 10;
        int y = (int)view->viewport.y + 10 - view->scroll_offset_y;
        int max_x = (int)view->viewport.x + (int)view->viewport.w - view->item_width - 10;
        
        // 遍历所有文件项
        int index = 0;
        FileItem *item = view->files->head;
        while (item) {
            // 跳过隐藏文件
            if (item->is_hidden && !view->show_hidden) {
                item = item->next;
                continue;
            }
            
            // 换行处理
            if (x > max_x) {
                x = (int)view->viewport.x + 10;
                y += view->item_height + 10;
            }
            
            // 只绘制可见区域内的项目
            if (y + view->item_height >= (int)view->viewport.y && y <= (int)view->viewport.y + (int)view->viewport.h) {
                // 绘制选中背景
                if (index == view->selected_index) {
                    SDL_SetRenderDrawColor(renderer, selected_bg_color.r, selected_bg_color.g, selected_bg_color.b, selected_bg_color.a);
                    SDL_FRect select_rect = {
                        (float)(x - 5), 
                        (float)(y - 5), 
                        (float)(view->item_width + 10), 
                        (float)(view->item_height + 10)
                    };
                    SDL_RenderFillRect(renderer, &select_rect);
                }
                
                // 绘制图标
                SDL_Texture *icon = (item->type == FILE_TYPE_DIRECTORY) ? view->folder_icon : view->file_icon;
                if (icon) {
                    SDL_FRect icon_rect = {
                        (float)(x + (view->item_width - 32) / 2), 
                        (float)y, 
                        32.0f, 
                        32.0f
                    };
                    SDL_RenderTexture(renderer, icon, NULL, &icon_rect);
                }
                
                // 绘制文件名
                SDL_Color current_text_color = (index == view->selected_index) ? selected_text_color : text_color;
                size_t name_len = strlen(item->name);
                SDL_Surface *text_surface = TTF_RenderText_Blended(font, item->name, name_len, current_text_color);
                if (text_surface) {
                    SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
                    if (text_texture) {
                        SDL_FRect text_rect = {0, 0, 0, 0};
                        text_rect.w = (text_surface->w > view->item_width) ? (float)view->item_width : (float)text_surface->w;
                        text_rect.h = (float)text_surface->h;
                        text_rect.x = (float)x + ((float)view->item_width - text_rect.w) / 2;
                        text_rect.y = (float)(y + 40);
                        
                        SDL_RenderTexture(renderer, text_texture, NULL, &text_rect);
                        SDL_DestroyTexture(text_texture);
                    }
                    SDL_DestroySurface(text_surface);
                }
            }
            
            // 移动到下一个位置
            x += view->item_width + 10;
            index++;
            item = item->next;
        }
    } else {
        // 列表视图
        int y = (int)view->viewport.y + 5 - view->scroll_offset_y;
        
        // 遍历所有文件项
        int index = 0;
        FileItem *item = view->files->head;
        while (item) {
            // 跳过隐藏文件
            if (item->is_hidden && !view->show_hidden) {
                item = item->next;
                continue;
            }
            
            // 只绘制可见区域内的项目
            if (y + view->item_height >= (int)view->viewport.y && y <= (int)view->viewport.y + (int)view->viewport.h) {
                // 绘制选中背景
                if (index == view->selected_index) {
                    SDL_SetRenderDrawColor(renderer, selected_bg_color.r, selected_bg_color.g, selected_bg_color.b, selected_bg_color.a);
                    SDL_FRect select_rect = {
                        (float)view->viewport.x + 5, 
                        (float)y, 
                        (float)view->viewport.w - 10, 
                        (float)view->item_height
                    };
                    SDL_RenderFillRect(renderer, &select_rect);
                }
                
                // 绘制图标
                SDL_Texture *icon = (item->type == FILE_TYPE_DIRECTORY) ? view->folder_icon : view->file_icon;
                if (icon) {
                    SDL_FRect icon_rect = {
                        (float)view->viewport.x + 10, 
                        (float)y + (float)(view->item_height - 16) / 2, 
                        16.0f, 
                        16.0f
                    };
                    SDL_RenderTexture(renderer, icon, NULL, &icon_rect);
                }
                
                // 绘制文件名
                SDL_Color current_text_color = (index == view->selected_index) ? selected_text_color : text_color;
                size_t name_len = strlen(item->name);
                SDL_Surface *text_surface = TTF_RenderText_Blended(font, item->name, name_len, current_text_color);
                if (text_surface) {
                    SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
                    if (text_texture) {
                        SDL_FRect text_rect = {0, 0, 0, 0};
                        text_rect.w = (float)text_surface->w;
                        text_rect.h = (float)text_surface->h;
                        text_rect.x = (float)view->viewport.x + 35;
                        text_rect.y = (float)y + ((float)view->item_height - text_rect.h) / 2;
                        
                        SDL_RenderTexture(renderer, text_texture, NULL, &text_rect);
                        SDL_DestroyTexture(text_texture);
                    }
                    SDL_DestroySurface(text_surface);
                }
                
                // 详细视图模式下显示文件大小和修改时间
                if (view->view_mode == VIEW_MODE_DETAILS) {
                    // 显示文件大小
                    char size_str[64] = {0};
                    if (item->type == FILE_TYPE_DIRECTORY) {
                        strcpy(size_str, "<DIR>");
                    } else {
                        // 格式化文件大小
                        format_file_size(item->size, size_str, sizeof(size_str));
                    }
                    
                    size_t size_len = strlen(size_str);
                    SDL_Surface *size_surface = TTF_RenderText_Blended(font, size_str, size_len, current_text_color);
                    if (size_surface) {
                        SDL_Texture *size_texture = SDL_CreateTextureFromSurface(renderer, size_surface);
                        if (size_texture) {
                            SDL_FRect size_rect = {0, 0, 0, 0};
                            size_rect.w = (float)size_surface->w;
                            size_rect.h = (float)size_surface->h;
                            size_rect.x = (float)view->viewport.x + 300;
                            size_rect.y = (float)y + ((float)view->item_height - size_rect.h) / 2;
                            
                            SDL_RenderTexture(renderer, size_texture, NULL, &size_rect);
                            SDL_DestroyTexture(size_texture);
                        }
                        SDL_DestroySurface(size_surface);
                    }
                    
                    // 显示修改时间
                    char time_str[64] = {0};
                    format_time(item->modified_time, time_str, sizeof(time_str));
                    
                    size_t time_len = strlen(time_str);
                    SDL_Surface *time_surface = TTF_RenderText_Blended(font, time_str, time_len, current_text_color);
                    if (time_surface) {
                        SDL_Texture *time_texture = SDL_CreateTextureFromSurface(renderer, time_surface);
                        if (time_texture) {
                            SDL_FRect time_rect = {0, 0, 0, 0};
                            time_rect.w = (float)time_surface->w;
                            time_rect.h = (float)time_surface->h;
                            time_rect.x = (float)view->viewport.x + 450;
                            time_rect.y = (float)y + ((float)view->item_height - time_rect.h) / 2;
                            
                            SDL_RenderTexture(renderer, time_texture, NULL, &time_rect);
                            SDL_DestroyTexture(time_texture);
                        }
                        SDL_DestroySurface(time_surface);
                    }
                }
            }
            
            // 移动到下一行
            y += view->item_height + 2;
            index++;
            item = item->next;
        }
    }
    
    // 重置裁剪区域
    SDL_SetRenderClipRect(renderer, NULL);
}

// 选择文件项
void file_list_view_select_item(FileListView *view, int index) {
    if (!view || !view->files) {
        return;
    }
    
    // 计算可见项目的数量
    int visible_count = 0;
    FileItem *item = view->files->head;
    while (item) {
        if (!item->is_hidden || view->show_hidden) {
            visible_count++;
        }
        item = item->next;
    }
    
    // 确保索引在有效范围内
    if (index < -1 || index >= visible_count) {
        index = -1; // 无选择
    }
    
    view->selected_index = index;
}

// 获取选中的文件项
FileItem* file_list_view_get_selected_item(FileListView *view) {
    if (!view || !view->files || view->selected_index < 0) {
        return NULL;
    }
    
    int visible_index = 0;
    FileItem *item = view->files->head;
    
    while (item) {
        // 跳过隐藏文件（如果需要）
        if (item->is_hidden && !view->show_hidden) {
            item = item->next;
            continue;
        }
        
        if (visible_index == view->selected_index) {
            return item;
        }
        
        visible_index++;
        item = item->next;
    }
    
    return NULL;
}

// 打开选中的文件或目录
void file_list_view_open_selected(FileListView *view) {
    if (!view) {
        return;
    }
    
    FileItem *selected = file_list_view_get_selected_item(view);
    if (!selected) {
        return;
    }
    
    if (selected->type == FILE_TYPE_DIRECTORY) {
        // 检查是否是返回上级目录的项目
        if (strcmp(selected->name, "..") == 0) {
            // 返回上级目录
            file_list_view_go_up(view);
        } else {
            // 检查当前是否在驱动器列表中
            if (view->files->current_dir && strcmp(view->files->current_dir, "[Drives]") == 0) {
                // 从驱动器列表进入驱动器，确保路径正确
                char drive_root[4];
                sprintf(drive_root, "%s\\", selected->name);
                file_list_view_load_directory(view, drive_root);
            } else {
                // 如果是目录或驱动器，进入该目录
                file_list_view_load_directory(view, selected->path);
            }
        }
    } else {
        // 打开文件
#ifdef _WIN32
        // Windows平台：使用ShellExecute
        HINSTANCE result = ShellExecute(NULL, "open", selected->path, NULL, NULL, SW_SHOWNORMAL);
        
        if ((intptr_t)result <= 32) {
            // 如果ShellExecute失败，尝试使用SDL_OpenURL作为备用
            char file_url[PATH_MAX + 8];
            snprintf(file_url, sizeof(file_url), "file:///%s", selected->path);
            
            // 将反斜杠转换为正斜杠
            for (char *p = file_url + 8; *p; p++) {
                if (*p == '\\') {
                    *p = '/';
                }
            }
            
            SDL_OpenURL(file_url);
        }
#else
        // 其他平台：使用SDL_OpenURL
        char file_url[PATH_MAX + 8];
        snprintf(file_url, sizeof(file_url), "file://%s", selected->path);
        SDL_OpenURL(file_url);
#endif
    }
}

// 返回上级目录
void file_list_view_go_up(FileListView *view) {
    if (!view || !view->current_path) {
        return;
    }
    
    // 获取上级目录路径
    char parent_dir[PATH_MAX];
    strcpy(parent_dir, view->current_path);
    
    // 检查是否已经在驱动器根目录（如 "E:\\"）
    size_t len = strlen(parent_dir);
    if ((len == 3 && parent_dir[1] == ':' && parent_dir[2] == '\\') ||
        (len == 2 && parent_dir[1] == ':')) {
        // 已经是驱动器根目录，显示所有驱动器
        file_list_view_load_drives(view);
        return;
    }
    
    // 移除末尾的路径分隔符（如果有）
    if (len > 1 && (parent_dir[len-1] == '/' || parent_dir[len-1] == '\\')) {
        parent_dir[len-1] = '\0';
        len--; // 更新长度
    }
    
    // 查找最后一个路径分隔符
    char *last_sep = strrchr(parent_dir, '/');
    if (!last_sep) {
        last_sep = strrchr(parent_dir, '\\');
    }
    
    if (last_sep) {
        // 如果是根目录下的子目录，保留根目录符号
        if (last_sep == parent_dir) {
            *(last_sep + 1) = '\0';
        } else {
            *last_sep = '\0';
        }
        
        // 检查截取后是否为驱动器根目录（Windows）
        size_t new_len = strlen(parent_dir);
        if (new_len == 2 && parent_dir[1] == ':') {
            // 已经是驱动器根目录（如 "C:"），添加反斜杠并加载
            strcat(parent_dir, "\\");
            file_list_view_load_directory(view, parent_dir);
            return;
        }
        
        // 加载上级目录
        file_list_view_load_directory(view, parent_dir);
    } else {
        // 没有路径分隔符，显示所有驱动器
#ifdef _WIN32
        file_list_view_load_drives(view);
#endif
    }
}

// 滚动文件列表
void file_list_view_scroll(FileListView *view, int delta) {
    if (!view) {
        return;
    }
    
    // 计算最大滚动范围
    int max_scroll = 0;
    
    if (view->view_mode == VIEW_MODE_ICONS) {
        // 计算图标视图的行数
        int items_per_row = (view->viewport.w - 20) / (view->item_width + 10);
        if (items_per_row < 1) items_per_row = 1;
        
        int visible_count = 0;
        FileItem *item = view->files->head;
        while (item) {
            if (!item->is_hidden || view->show_hidden) {
                visible_count++;
            }
            item = item->next;
        }
        
        int rows = (visible_count + items_per_row - 1) / items_per_row;
        max_scroll = rows * (view->item_height + 10) - view->viewport.h + 20;
    } else {
        // 列表视图或详细信息视图
        int visible_count = 0;
        FileItem *item = view->files->head;
        while (item) {
            if (!item->is_hidden || view->show_hidden) {
                visible_count++;
            }
            item = item->next;
        }
        
        max_scroll = visible_count * (view->item_height + 2) - view->viewport.h + 10;
    }
    
    // 应用滚动偏移
    view->scroll_offset_y += delta;
    
    // 限制滚动范围
    if (view->scroll_offset_y < 0) {
        view->scroll_offset_y = 0;
    } else if (max_scroll > 0 && view->scroll_offset_y > max_scroll) {
        view->scroll_offset_y = max_scroll;
    }
}

// 加载Windows驱动器列表
void file_list_view_load_drives(FileListView *view) {
    if (!view || !view->files) {
        return;
    }
    
#ifdef _WIN32
    // 清空当前列表
    file_list_clear(view->files);
    
    // 设置当前目录为驱动器列表标识
    if (view->files->current_dir) {
        free(view->files->current_dir);
    }
    view->files->current_dir = strdup("[Drives]");
    
    // 获取所有逻辑驱动器
    DWORD drives = GetLogicalDrives();
    
    for (int i = 0; i < 26; i++) {
        if (drives & (1 << i)) {
            char drive_letter = 'A' + i;
            char drive_path[4];
            sprintf(drive_path, "%c:\\", drive_letter);
            
            // 检查驱动器类型
            UINT drive_type = GetDriveType(drive_path);
            if (drive_type == DRIVE_NO_ROOT_DIR) {
                continue; // 跳过无效驱动器
            }
            
            // 创建驱动器项目
            FileItem *drive_item = (FileItem*)calloc(1, sizeof(FileItem));
            if (drive_item) {
                // 设置驱动器名称
                char drive_name[32];
                sprintf(drive_name, "%c:", drive_letter);
                drive_item->name = strdup(drive_name);
                
                // 设置显示名称（包含驱动器类型）
                char display_name[64];
                const char *type_str = "";
                switch (drive_type) {
                    case DRIVE_FIXED:
                        type_str = " (Local Disk)";
                        break;
                    case DRIVE_REMOVABLE:
                        type_str = " (Removable Disk)";
                        break;
                    case DRIVE_REMOTE:
                        type_str = " (Network Drive)";
                        break;
                    case DRIVE_CDROM:
                        type_str = " (CD-ROM)";
                        break;
                    case DRIVE_RAMDISK:
                        type_str = " (RAM Disk)";
                        break;
                }
                sprintf(display_name, "%c:%s", drive_letter, type_str);
                drive_item->display_name = strdup(display_name);
                
                // 设置路径
                drive_item->path = strdup(drive_path);
                drive_item->type = FILE_TYPE_DIRECTORY;
                drive_item->size = 0;
                drive_item->is_hidden = false;
                drive_item->next = NULL;
                
                // 添加到列表
                file_list_add_item(view->files, drive_item);
            }
        }
    }
#endif
}

// 处理事件
bool file_list_view_handle_event(FileListView *view, SDL_Event *event) {
    if (!view || !event) {
        return false;
    }
    
    switch (event->type) {
        case SDL_EVENT_MOUSE_WHEEL: {
            // 鼠标滚轮事件
            int delta = -event->wheel.y * 30; // 滚动速度因子
            file_list_view_scroll(view, delta);
            return true;
        }
        
        case SDL_EVENT_MOUSE_BUTTON_DOWN: {
            // 鼠标点击事件
            int x = event->button.x;
            int y = event->button.y;
            
            // 检查点击是否在视口内
            if (x >= view->viewport.x && x < view->viewport.x + view->viewport.w &&
                y >= view->viewport.y && y < view->viewport.y + view->viewport.h) {
                
                // 计算点击的项目
                int clicked_index = -1;
                
                if (view->view_mode == VIEW_MODE_ICONS) {
                    // 图标视图
                    int items_per_row = (view->viewport.w - 20) / (view->item_width + 10);
                    if (items_per_row < 1) items_per_row = 1;
                    
                    int row = (y - view->viewport.y + view->scroll_offset_y - 10) / (view->item_height + 10);
                    int col = (x - view->viewport.x - 10) / (view->item_width + 10);
                    
                    if (col >= 0 && col < items_per_row) {
                        clicked_index = row * items_per_row + col;
                    }
                } else {
                    // 列表视图或详细信息视图
                    clicked_index = (y - view->viewport.y + view->scroll_offset_y - 5) / (view->item_height + 2);
                }
                
                // 验证点击的索引是否有效
                int visible_count = 0;
                FileItem *item = view->files->head;
                while (item) {
                    if (!item->is_hidden || view->show_hidden) {
                        if (visible_count == clicked_index) {
                            // 处理左键和右键点击
                            if (event->button.button == SDL_BUTTON_LEFT) {
                                file_list_view_select_item(view, clicked_index);
                                
                                // 检查双击
                                if (event->button.clicks == 2) {
                                    file_list_view_open_selected(view);
                                }
                            } else if (event->button.button == SDL_BUTTON_RIGHT) {
                                // 右键点击：选中项目并显示右键菜单
                                file_list_view_select_item(view, clicked_index);
                                if (view->on_right_click) {
                                    view->on_right_click(view, x, y, item);
                                }
                                printf("右键点击文件: %s\n", item->display_name);
                            }
                            
                            return true;
                        }
                        visible_count++;
                    }
                    item = item->next;
                }
                
                // 点击空白区域
                if (event->button.button == SDL_BUTTON_LEFT) {
                    // 左键点击空白区域，取消选择
                    file_list_view_select_item(view, -1);
                } else if (event->button.button == SDL_BUTTON_RIGHT) {
                    // 右键点击空白区域，显示空白区域右键菜单
                    file_list_view_select_item(view, -1);
                    if (view->on_right_click) {
                        view->on_right_click(view, x, y, NULL);
                    }
                    printf("右键点击空白区域\n");
                }
                
                return true;
            }
            break;
        }
        
        case SDL_EVENT_KEY_DOWN: {
            // 键盘事件
            switch (event->key.scancode) {
                case SDL_SCANCODE_UP:
                    if (view->selected_index > 0) {
                        file_list_view_select_item(view, view->selected_index - 1);
                    }
                    return true;
                    
                case SDL_SCANCODE_DOWN: {
                    int visible_count = 0;
                    FileItem *item = view->files->head;
                    while (item) {
                        if (!item->is_hidden || view->show_hidden) {
                            visible_count++;
                        }
                        item = item->next;
                    }
                    
                    if (view->selected_index < visible_count - 1) {
                        file_list_view_select_item(view, view->selected_index + 1);
                    }
                    return true;
                }
                
                case SDL_SCANCODE_RETURN:
                    file_list_view_open_selected(view);
                    return true;
                    
                case SDL_SCANCODE_BACKSPACE:
                    file_list_view_go_up(view);
                    return true;
            }
            break;
        }
    }
    
    return false;
}