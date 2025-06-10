#ifndef CONTEXT_MENU_H
#define CONTEXT_MENU_H

#include "main.h"
#include "window.h"
#include "file_item.h"

// 菜单项类型
typedef enum {
    MENU_ITEM_SEPARATOR,    // 分隔符
    MENU_ITEM_ACTION,       // 普通菜单项
    MENU_ITEM_SUBMENU       // 子菜单
} MenuItemType;

// 菜单项动作类型
typedef enum {
    ACTION_OPEN,            // 打开
    ACTION_COPY,            // 复制
    ACTION_CUT,             // 剪切
    ACTION_PASTE,           // 粘贴
    ACTION_DELETE,          // 删除
    ACTION_RENAME,          // 重命名
    ACTION_PROPERTIES,      // 属性
    ACTION_NEW_FOLDER,      // 新建文件夹
    ACTION_NEW_FILE,        // 新建文件
    ACTION_REFRESH          // 刷新
} MenuAction;

// 菜单项结构
typedef struct MenuItem {
    MenuItemType type;      // 菜单项类型
    char *text;             // 显示文本
    MenuAction action;      // 动作类型
    bool enabled;           // 是否启用
    struct MenuItem *next;  // 下一个菜单项
} MenuItem;

// 前向声明
struct FileListView;

// 右键菜单结构
typedef struct {
    struct Window *window;  // 窗口引用
    MenuItem *items;        // 菜单项列表
    int x, y;              // 菜单显示位置
    int width, height;     // 菜单尺寸
    bool visible;          // 是否可见
    FileItem *target_item; // 目标文件项（如果有）
    char *current_dir;     // 当前目录路径
    struct FileListView *file_list_view; // 文件列表视图引用
} ContextMenu;

// 创建右键菜单
ContextMenu* context_menu_new(struct Window *window);

// 释放右键菜单
void context_menu_free(ContextMenu *menu);

// 显示文件项右键菜单
void context_menu_show_for_file(ContextMenu *menu, FileItem *item, int x, int y);

// 显示空白区域右键菜单
void context_menu_show_for_blank(ContextMenu *menu, int x, int y);

// 隐藏菜单
void context_menu_hide(ContextMenu *menu);

// 处理菜单事件
bool context_menu_handle_event(ContextMenu *menu, SDL_Event *event);

// 绘制菜单
void context_menu_draw(ContextMenu *menu);

// 执行菜单动作
void context_menu_execute_action(ContextMenu *menu, MenuAction action);

// 设置当前目录
void context_menu_set_current_dir(ContextMenu *menu, const char *dir);

// 设置文件列表视图引用
void context_menu_set_file_list_view(ContextMenu *menu, struct FileListView *view);

#endif // CONTEXT_MENU_H