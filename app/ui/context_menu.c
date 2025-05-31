/*
 * 右键菜单模块
 * 职责：
 * 1. 实现右键菜单的显示和隐藏
 * 2. 处理菜单项的点击事件
 * 3. 执行相应的文件操作
 * 4. 管理菜单的生命周期
 */

#include "context_menu.h"
#include "renderer.h"
#include <string.h>

// 菜单样式常量
#define MENU_ITEM_HEIGHT 25
#define MENU_PADDING 5
#define MENU_MIN_WIDTH 120

// 颜色常量
static const SDL_Color MENU_BORDER_COLOR = {100, 100, 100, 255};
static const SDL_Color MENU_BG_COLOR = {240, 240, 240, 255};
static const SDL_Color MENU_HOVER_COLOR = {200, 220, 255, 255};
static const SDL_Color MENU_TEXT_COLOR = {0, 0, 0, 255};
static const SDL_Color MENU_DISABLED_COLOR = {150, 150, 150, 255};

// 创建菜单项
static MenuItem* menu_item_new(MenuItemType type, const char *text, MenuAction action, bool enabled) {
    MenuItem *item = (MenuItem*)calloc(1, sizeof(MenuItem));
    if (!item) {
        return NULL;
    }
    
    item->type = type;
    item->text = text ? strdup(text) : NULL;
    item->action = action;
    item->enabled = enabled;
    item->next = NULL;
    
    return item;
}

// 释放菜单项
static void menu_item_free(MenuItem *item) {
    if (!item) {
        return;
    }
    
    if (item->text) {
        free(item->text);
    }
    
    if (item->next) {
        menu_item_free(item->next);
    }
    
    free(item);
}

// 添加菜单项到列表末尾
static void menu_add_item(ContextMenu *menu, MenuItem *item) {
    if (!menu || !item) {
        return;
    }
    
    if (!menu->items) {
        menu->items = item;
    } else {
        MenuItem *current = menu->items;
        while (current->next) {
            current = current->next;
        }
        current->next = item;
    }
}

// 创建文件项菜单
static void create_file_menu_items(ContextMenu *menu, FileItem *item) {
    if (!menu || !item) {
        return;
    }
    
    // 清空现有菜单项
    if (menu->items) {
        menu_item_free(menu->items);
        menu->items = NULL;
    }
    
    // 添加菜单项
    menu_add_item(menu, menu_item_new(MENU_ITEM_ACTION, "打开", ACTION_OPEN, true));
    menu_add_item(menu, menu_item_new(MENU_ITEM_SEPARATOR, NULL, 0, false));
    menu_add_item(menu, menu_item_new(MENU_ITEM_ACTION, "复制", ACTION_COPY, true));
    menu_add_item(menu, menu_item_new(MENU_ITEM_ACTION, "剪切", ACTION_CUT, true));
    menu_add_item(menu, menu_item_new(MENU_ITEM_ACTION, "删除", ACTION_DELETE, true));
    menu_add_item(menu, menu_item_new(MENU_ITEM_SEPARATOR, NULL, 0, false));
    menu_add_item(menu, menu_item_new(MENU_ITEM_ACTION, "重命名", ACTION_RENAME, true));
    menu_add_item(menu, menu_item_new(MENU_ITEM_SEPARATOR, NULL, 0, false));
    menu_add_item(menu, menu_item_new(MENU_ITEM_ACTION, "属性", ACTION_PROPERTIES, true));
}

// 创建空白区域菜单
static void create_blank_menu_items(ContextMenu *menu) {
    if (!menu) {
        return;
    }
    
    // 清空现有菜单项
    if (menu->items) {
        menu_item_free(menu->items);
        menu->items = NULL;
    }
    
    // 添加菜单项
    menu_add_item(menu, menu_item_new(MENU_ITEM_ACTION, "粘贴", ACTION_PASTE, true));
    menu_add_item(menu, menu_item_new(MENU_ITEM_SEPARATOR, NULL, 0, false));
    menu_add_item(menu, menu_item_new(MENU_ITEM_ACTION, "新建文件夹", ACTION_NEW_FOLDER, true));
    menu_add_item(menu, menu_item_new(MENU_ITEM_ACTION, "新建文件", ACTION_NEW_FILE, true));
    menu_add_item(menu, menu_item_new(MENU_ITEM_SEPARATOR, NULL, 0, false));
    menu_add_item(menu, menu_item_new(MENU_ITEM_ACTION, "刷新", ACTION_REFRESH, true));
}

// 计算菜单尺寸
static void calculate_menu_size(ContextMenu *menu) {
    if (!menu || !menu->window || !menu->window->font) {
        return;
    }
    
    TTF_Font *font = menu->window->font;
    int max_width = MENU_MIN_WIDTH;
    int total_height = MENU_PADDING * 2;
    
    MenuItem *item = menu->items;
    while (item) {
        if (item->type == MENU_ITEM_SEPARATOR) {
            total_height += 5; // 分隔符高度
        } else if (item->type == MENU_ITEM_ACTION && item->text) {
            // 计算文本宽度
            int text_width, text_height;
            if (TTF_GetStringSize(font, item->text, strlen(item->text), &text_width, &text_height)) {
                int item_width = text_width + MENU_PADDING * 2;
                if (item_width > max_width) {
                    max_width = item_width;
                }
            }
            total_height += MENU_ITEM_HEIGHT;
        }
        item = item->next;
    }
    
    menu->width = max_width;
    menu->height = total_height;
}

// 创建右键菜单
ContextMenu* context_menu_new(struct Window *window) {
    if (!window) {
        return NULL;
    }
    
    ContextMenu *menu = (ContextMenu*)calloc(1, sizeof(ContextMenu));
    if (!menu) {
        return NULL;
    }
    
    menu->window = window;
    menu->items = NULL;
    menu->x = 0;
    menu->y = 0;
    menu->width = 0;
    menu->height = 0;
    menu->visible = false;
    menu->target_item = NULL;
    
    return menu;
}

// 释放右键菜单
void context_menu_free(ContextMenu *menu) {
    if (!menu) {
        return;
    }
    
    if (menu->items) {
        menu_item_free(menu->items);
    }
    
    free(menu);
}

// 显示文件项右键菜单
void context_menu_show_for_file(ContextMenu *menu, FileItem *item, int x, int y) {
    if (!menu) {
        return;
    }
    
    menu->target_item = item;
    create_file_menu_items(menu, item);
    calculate_menu_size(menu);
    
    // 调整菜单位置，确保不超出窗口边界
    menu->x = x;
    menu->y = y;
    
    if (menu->x + menu->width > SDL_WINDOW_WIDTH) {
        menu->x = SDL_WINDOW_WIDTH - menu->width;
    }
    if (menu->y + menu->height > SDL_WINDOW_HEIGHT) {
        menu->y = SDL_WINDOW_HEIGHT - menu->height;
    }
    
    menu->visible = true;
}

// 显示空白区域右键菜单
void context_menu_show_for_blank(ContextMenu *menu, int x, int y) {
    if (!menu) {
        return;
    }
    
    menu->target_item = NULL;
    create_blank_menu_items(menu);
    calculate_menu_size(menu);
    
    // 调整菜单位置，确保不超出窗口边界
    menu->x = x;
    menu->y = y;
    
    if (menu->x + menu->width > SDL_WINDOW_WIDTH) {
        menu->x = SDL_WINDOW_WIDTH - menu->width;
    }
    if (menu->y + menu->height > SDL_WINDOW_HEIGHT) {
        menu->y = SDL_WINDOW_HEIGHT - menu->height;
    }
    
    menu->visible = true;
}

// 隐藏菜单
void context_menu_hide(ContextMenu *menu) {
    if (!menu) {
        return;
    }
    
    menu->visible = false;
    menu->target_item = NULL;
}

// 处理菜单事件
bool context_menu_handle_event(ContextMenu *menu, SDL_Event *event) {
    if (!menu || !event || !menu->visible) {
        return false;
    }
    
    switch (event->type) {
        case SDL_EVENT_MOUSE_BUTTON_DOWN: {
            int x = event->button.x;
            int y = event->button.y;
            
            // 检查点击是否在菜单内
            if (x >= menu->x && x < menu->x + menu->width &&
                y >= menu->y && y < menu->y + menu->height) {
                
                if (event->button.button == SDL_BUTTON_LEFT) {
                    // 计算点击的菜单项
                    int item_y = menu->y + MENU_PADDING;
                    MenuItem *item = menu->items;
                    
                    while (item) {
                        if (item->type == MENU_ITEM_ACTION) {
                            if (y >= item_y && y < item_y + MENU_ITEM_HEIGHT) {
                                if (item->enabled) {
                                    context_menu_execute_action(menu, item->action);
                                }
                                context_menu_hide(menu);
                                return true;
                            }
                            item_y += MENU_ITEM_HEIGHT;
                        } else if (item->type == MENU_ITEM_SEPARATOR) {
                            item_y += 5;
                        }
                        item = item->next;
                    }
                }
                return true; // 消费事件，防止传递给其他组件
            } else {
                // 点击菜单外部，隐藏菜单
                context_menu_hide(menu);
                return false; // 不消费事件，让其他组件处理
            }
        }
        
        case SDL_EVENT_KEY_DOWN: {
            // ESC键隐藏菜单
            if (event->key.scancode == SDL_SCANCODE_ESCAPE) {
                context_menu_hide(menu);
                return true;
            }
            break;
        }
    }
    
    return false;
}

// 绘制菜单
void context_menu_draw(ContextMenu *menu) {
    if (!menu || !menu->visible || !menu->window || !menu->window->renderer) {
        return;
    }
    
    SDL_Renderer *renderer = menu->window->renderer;
    TTF_Font *font = menu->window->font;
    
    if (!font) {
        return;
    }
    
    // 绘制菜单背景
    SDL_Color bg_color = MENU_BG_COLOR;
    SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, bg_color.a);
    SDL_FRect bg_rect = {menu->x, menu->y, menu->width, menu->height};
    SDL_RenderFillRect(renderer, &bg_rect);
    
    // 绘制菜单边框
    SDL_Color border_color = MENU_BORDER_COLOR;
    SDL_SetRenderDrawColor(renderer, border_color.r, border_color.g, border_color.b, border_color.a);
    SDL_RenderRect(renderer, &bg_rect);
    
    // 绘制菜单项
    int item_y = menu->y + MENU_PADDING;
    MenuItem *item = menu->items;
    
    while (item) {
        if (item->type == MENU_ITEM_ACTION && item->text) {
            // 绘制菜单项文本
            SDL_Color text_color = item->enabled ? MENU_TEXT_COLOR : MENU_DISABLED_COLOR;
            SDL_Surface *text_surface = TTF_RenderText_Blended(font, item->text, 0, text_color);
            
            if (text_surface) {
                SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
                if (text_texture) {
                    SDL_FRect text_rect = {
                        menu->x + MENU_PADDING,
                        item_y + (MENU_ITEM_HEIGHT - text_surface->h) / 2,
                        text_surface->w,
                        text_surface->h
                    };
                    SDL_RenderTexture(renderer, text_texture, NULL, &text_rect);
                    SDL_DestroyTexture(text_texture);
                }
                SDL_DestroySurface(text_surface);
            }
            
            item_y += MENU_ITEM_HEIGHT;
        } else if (item->type == MENU_ITEM_SEPARATOR) {
            // 绘制分隔符
            SDL_SetRenderDrawColor(renderer, border_color.r, border_color.g, border_color.b, border_color.a);
            SDL_RenderLine(renderer, 
                          menu->x + MENU_PADDING, item_y + 2,
                          menu->x + menu->width - MENU_PADDING, item_y + 2);
            item_y += 5;
        }
        
        item = item->next;
    }
}

// 执行菜单动作
void context_menu_execute_action(ContextMenu *menu, MenuAction action) {
    if (!menu) {
        return;
    }
    
    switch (action) {
        case ACTION_OPEN:
            printf("执行动作: 打开\n");
            // TODO: 实现打开文件/文件夹功能
            break;
            
        case ACTION_COPY:
            printf("执行动作: 复制\n");
            // TODO: 实现复制功能
            break;
            
        case ACTION_CUT:
            printf("执行动作: 剪切\n");
            // TODO: 实现剪切功能
            break;
            
        case ACTION_PASTE:
            printf("执行动作: 粘贴\n");
            // TODO: 实现粘贴功能
            break;
            
        case ACTION_DELETE:
            printf("执行动作: 删除\n");
            // TODO: 实现删除功能
            break;
            
        case ACTION_RENAME:
            printf("执行动作: 重命名\n");
            // TODO: 实现重命名功能
            break;
            
        case ACTION_PROPERTIES:
            printf("执行动作: 属性\n");
            // TODO: 实现属性对话框
            break;
            
        case ACTION_NEW_FOLDER:
            printf("执行动作: 新建文件夹\n");
            // TODO: 实现新建文件夹功能
            break;
            
        case ACTION_NEW_FILE:
            printf("执行动作: 新建文件\n");
            // TODO: 实现新建文件功能
            break;
            
        case ACTION_REFRESH:
            printf("执行动作: 刷新\n");
            // TODO: 实现刷新功能
            break;
            
        default:
            printf("未知动作: %d\n", action);
            break;
    }
}