#ifndef SIDEBAR_H
#define SIDEBAR_H

#include "main.h"
#include "window.h"

// 侧边栏宽度定义
#define SIDEBAR_WIDTH 200
// 侧边栏项目最大数量
#define MAX_SIDEBAR_ITEMS 20

// 前向声明
struct Window;

// 特殊文件夹枚举
typedef enum {
    FOLDER_DESKTOP,    // 桌面
    FOLDER_DOCUMENTS,  // 文档
    FOLDER_DOWNLOADS,  // 下载
    FOLDER_MUSIC,      // 音乐
    FOLDER_PICTURES,   // 图片
    FOLDER_VIDEOS      // 视频
} SpecialFolder;

// 驱动器信息结构体
typedef struct DriveInfo {
    char letter;       // 驱动器盘符
    char label[256];   // 驱动器标签
    char fs_type[32];  // 文件系统类型
    uint64_t total_size;  // 总大小
    uint64_t free_size;   // 可用大小
    bool is_removable;    // 是否可移动设备
} DriveInfo;

// 侧边栏状态枚举
typedef enum SidebarState {
    SIDEBAR_STATE_NORMAL,    // 侧边栏处于正常状态
    SIDEBAR_STATE_FOCUSED,   // 侧边栏处于焦点状态       
    SIDEBAR_STATE_HOVER,     // 侧边栏处于悬停状态
    SIDEBAR_STATE_ACTIVE,    // 侧边栏处于活动状态
    SIDEBAR_STATE_DISABLED   // 侧边栏禁用状态
} SidebarState;

// 侧边栏项目类型
typedef enum {
    SIDEBAR_ITEM_QUICK_ACCESS,  // 快速访问项
    SIDEBAR_ITEM_DRIVE,         // 驱动器项
    SIDEBAR_ITEM_SEPARATOR      // 分隔线
} SidebarItemType;

// 侧边栏项目结构体
typedef struct SidebarItem {
    SidebarItemType type;       // 项目类型
    char *name;                 // 显示名称
    char *path;                 // 路径
    SDL_Texture *icon;          // 图标
    SDL_Rect rect;              // 项目区域
    SidebarState state;         // 项目状态
} SidebarItem;

// 前向声明Sidebar结构体
typedef struct Sidebar Sidebar;

// 侧边栏项目选中回调函数类型
typedef void (*SidebarItemSelectedCallback)(Sidebar *sidebar, const char *path);

// 侧边栏结构体
struct Sidebar {
    struct Window *app;                        // 应用程序窗口
    SDL_Rect rect;                             // 侧边栏区域
    SidebarItem items[MAX_SIDEBAR_ITEMS];      // 侧边栏项目
    int item_count;                            // 项目数量
    int selected_index;                        // 选中项索引
    int hover_index;                           // 悬停项索引
    SDL_Color bg_color;                        // 背景颜色
    SDL_Color text_color;                      // 文本颜色
    SDL_Color hover_color;                     // 悬停颜色
    SDL_Color selected_color;                  // 选中颜色
    SDL_Color separator_color;                 // 分隔线颜色
    int scroll_offset;                         // 滚动偏移
    SidebarItemSelectedCallback on_item_selected;  // 项目选中回调
};

// 侧边栏函数声明
Sidebar* sidebar_new(struct Window *app);                                                         // 创建侧边栏
void sidebar_free(Sidebar *sidebar);                                                              // 销毁侧边栏
bool sidebar_handle_event(Sidebar *sidebar, SDL_Event *event);                                    // 处理侧边栏事件
void sidebar_draw(Sidebar *sidebar);                                                              // 绘制侧边栏
void sidebar_set_item_selected_callback(Sidebar *sidebar, SidebarItemSelectedCallback callback);  // 设置项目选中回调
void sidebar_refresh_drives(Sidebar *sidebar);                                                    // 刷新驱动器列表

// 文件系统相关函数声明
bool get_special_folder_path(SpecialFolder folder, char *path, size_t path_size);  // 获取特殊文件夹路径
int get_drives(DriveInfo *drives, int max_count);                                  // 获取驱动器列表

#endif // SIDEBAR_H