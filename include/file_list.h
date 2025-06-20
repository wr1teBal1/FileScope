#ifndef FILE_LIST_H
#define FILE_LIST_H

#include "main.h"
#include "window.h"
#include "file_item.h"
#include "file_system.h"

// 文件列表视图模式
typedef enum {
    VIEW_MODE_ICONS,       // 图标视图
    VIEW_MODE_LIST,        // 列表视图
    VIEW_MODE_DETAILS      // 详细信息视图
} ViewMode;

// 文件排序方式
typedef enum {
    SORT_BY_NAME,          // 按名称排序
    SORT_BY_SIZE,          // 按大小排序
    SORT_BY_TYPE,          // 按类型排序
    SORT_BY_DATE_MODIFIED  // 按修改日期排序
} SortMode;

// 前向声明
struct FileListView;
struct FileItem;

// 右键点击回调函数类型
typedef void (*RightClickCallback)(struct FileListView *view, int x, int y, struct FileItem *item);

// 目录变更回调函数类型
typedef void (*DirectoryChangedCallback)(struct FileListView *view, const char *path);

// 文件列表视图结构
typedef struct FileListView {
    struct Window *window;       // 应用程序实例
    FileList *files;             // 文件列表数据
    char *current_path;          // 当前目录路径
    ViewMode view_mode;          // 视图模式
    SortMode sort_mode;          // 排序方式
    bool show_hidden;            // 是否显示隐藏文件
    int scroll_offset_y;         // 垂直滚动偏移
    int item_width;              // 项目宽度
    int item_height;             // 项目高度
    int selected_index;          // 当前选中的索引
    SDL_Rect viewport;           // 视口区域
    SDL_Texture *folder_icon;    // 文件夹图标
    SDL_Texture *file_icon;      // 文件图标
    RightClickCallback on_right_click;                  // 右键点击回调
    DirectoryChangedCallback on_directory_changed;      // 目录变更回调
    
    // 内联编辑相关
    bool is_editing;             // 是否正在编辑
    int editing_index;           // 正在编辑的项目索引
    char *edit_buffer;           // 编辑缓冲区
    size_t edit_buffer_size;     // 编辑缓冲区大小
    size_t edit_cursor_pos;      // 光标位置
    Uint32 last_blink_time;      // 上次光标闪烁时间
    bool cursor_visible;         // 光标是否可见
} FileListView;

// 创建文件列表视图
FileListView* file_list_view_new(struct Window *window);

// 释放文件列表视图
void file_list_view_free(FileListView *view);

// 加载目录
bool file_list_view_load_directory(FileListView *view, const char *path);

// 加载驱动器列表
void file_list_view_load_drives(FileListView *view);

// 刷新文件列表
void file_list_view_refresh(FileListView *view);

// 设置视图模式
void file_list_view_set_mode(FileListView *view, ViewMode mode);

// 切换到下一个视图模式
void file_list_view_toggle_mode(FileListView *view);

// 设置排序方式
void file_list_view_set_sort(FileListView *view, SortMode sort);

// 设置是否显示隐藏文件
void file_list_view_set_show_hidden(FileListView *view, bool show_hidden);

// 处理事件
bool file_list_view_handle_event(FileListView *view, SDL_Event *event);

// 绘制文件列表
void file_list_view_draw(FileListView *view);

// 选择文件项
void file_list_view_select_item(FileListView *view, int index);

// 获取选中的文件项
FileItem* file_list_view_get_selected_item(FileListView *view);

// 打开选中的文件或目录
void file_list_view_open_selected(FileListView *view);

// 返回上级目录
void file_list_view_go_up(FileListView *view);

// 滚动文件列表
void file_list_view_scroll(FileListView *view, int delta);

// 加载图标
bool file_list_view_load_icons(FileListView *view);

// 设置右键点击回调
void file_list_view_set_right_click_callback(FileListView *view, RightClickCallback callback);

// 内联编辑相关函数
void file_list_view_start_editing(FileListView *view, int index);
void file_list_view_stop_editing(FileListView *view, bool save_changes);
bool file_list_view_is_editing(FileListView *view);
void file_list_view_handle_text_input(FileListView *view, const char *text);
void file_list_view_handle_key_input(FileListView *view, SDL_Scancode scancode);

#endif // FILE_LIST_H