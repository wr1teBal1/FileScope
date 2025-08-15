# Sidebar 模块使用指南

## 概述

Sidebar模块是FileScope文件管理器的核心组件之一，负责显示快速访问项、驱动器列表和提供导航功能。该模块采用模块化设计，支持自定义样式、事件处理和动态内容更新。

## 核心特性

- **快速访问项**: 桌面、文档、下载、图片、音乐、视频等系统文件夹
- **驱动器管理**: 自动检测和显示本地驱动器
- **事件处理**: 完整的鼠标事件支持（点击、悬停、滚动）
- **自定义样式**: 可配置的颜色主题和布局
- **回调机制**: 支持项目选中事件回调
- **滚动支持**: 当项目过多时支持垂直滚动

## 数据结构

### Sidebar 主结构体

```c
struct Sidebar {
    struct Window *app;                        // 应用程序窗口引用
    SDL_Rect rect;                             // 侧边栏区域
    SidebarItem items[MAX_SIDEBAR_ITEMS];      // 项目数组
    int item_count;                            // 当前项目数量
    int selected_index;                        // 选中项索引
    int hover_index;                           // 悬停项索引
    SDL_Color bg_color;                        // 背景颜色
    SDL_Color text_color;                      // 文本颜色
    SDL_Color hover_color;                     // 悬停颜色
    SDL_Color selected_color;                  // 选中颜色
    SDL_Color separator_color;                 // 分隔线颜色
    int scroll_offset;                         // 滚动偏移量
    SidebarItemSelectedCallback on_item_selected;  // 选中回调函数
};
```

### SidebarItem 项目结构体

```c
typedef struct SidebarItem {
    SidebarItemType type;       // 项目类型
    char *name;                 // 显示名称
    char *path;                 // 文件路径
    SDL_Texture *icon;          // 图标纹理
    SDL_Rect rect;              // 项目区域
    SidebarState state;         // 项目状态
} SidebarItem;
```

### 项目类型枚举

```c
typedef enum {
    SIDEBAR_ITEM_QUICK_ACCESS,  // 快速访问项
    SIDEBAR_ITEM_DRIVE,         // 驱动器项
    SIDEBAR_ITEM_SEPARATOR      // 分隔线
} SidebarItemType;
```

## API 函数详解

### 1. 创建和销毁

#### `sidebar_new(struct Window *app)`
创建新的侧边栏实例。

**参数:**
- `app`: 应用程序窗口指针

**返回值:**
- 成功: 返回Sidebar指针
- 失败: 返回NULL

**使用示例:**
```c
Sidebar *sidebar = sidebar_new(window);
if (!sidebar) {
    // 处理创建失败
}
```

#### `sidebar_free(Sidebar *sidebar)`
释放侧边栏资源。

**参数:**
- `sidebar`: 要释放的侧边栏指针

**注意事项:**
- 会自动释放所有项目资源（名称、路径、图标）
- 调用后指针变为无效

### 2. 事件处理

#### `sidebar_handle_event(Sidebar *sidebar, SDL_Event *event)`
处理SDL事件。

**支持的事件类型:**
- `SDL_EVENT_MOUSE_MOTION`: 鼠标移动（更新悬停状态）
- `SDL_EVENT_MOUSE_BUTTON_DOWN`: 鼠标点击（选择项目）
- `SDL_EVENT_MOUSE_WHEEL`: 鼠标滚轮（垂直滚动）

**返回值:**
- `true`: 事件被处理
- `false`: 事件未被处理

**使用示例:**
```c
SDL_Event event;
while (SDL_PollEvent(&event)) {
    if (sidebar_handle_event(sidebar, &event)) {
        // 事件被侧边栏处理
        continue;
    }
    // 处理其他事件...
}
```

### 3. 绘制

#### `sidebar_draw(Sidebar *sidebar)`
绘制侧边栏到屏幕。

**绘制内容:**
- 背景色填充
- 右边框线
- 所有可见项目（考虑滚动偏移）

**使用示例:**
```c
// 在主渲染循环中调用
sidebar_draw(sidebar);
```

### 4. 回调设置

#### `sidebar_set_item_selected_callback(Sidebar *sidebar, SidebarItemSelectedCallback callback)`
设置项目选中回调函数。

**回调函数签名:**
```c
typedef void (*SidebarItemSelectedCallback)(Sidebar *sidebar, const char *path);
```

**使用示例:**
```c
void on_sidebar_item_selected(Sidebar *sidebar, const char *path) {
    printf("选中项目: %s\n", path);
    // 处理路径导航...
}

sidebar_set_item_selected_callback(sidebar, on_sidebar_item_selected);
```

### 5. 驱动器管理

#### `sidebar_refresh_drives(Sidebar *sidebar)`
刷新驱动器列表。

**功能:**
- 移除旧的驱动器项目
- 重新检测可用驱动器
- 更新项目位置

**使用场景:**
- 插入/移除USB设备后
- 网络驱动器连接状态改变
- 手动刷新需求

## 配置和自定义

### 颜色主题

可以通过修改Sidebar结构体中的颜色成员来自定义外观：

```c
// 设置浅色主题
sidebar->bg_color = (SDL_Color){255, 255, 255, 255};      // 白色背景
sidebar->text_color = (SDL_Color){0, 0, 0, 255};         // 黑色文本
sidebar->hover_color = (SDL_Color){240, 240, 240, 255};  // 浅灰悬停
sidebar->selected_color = (SDL_Color){220, 220, 220, 255}; // 浅灰选中

// 设置深色主题
sidebar->bg_color = (SDL_Color){40, 40, 40, 255};        // 深灰背景
sidebar->text_color = (SDL_Color){255, 255, 255, 255};   // 白色文本
sidebar->hover_color = (SDL_Color){60, 60, 60, 255};     // 深灰悬停
sidebar->selected_color = (SDL_Color){80, 80, 80, 255};  // 深灰选中
```

### 布局常量

在`sidebar.c`中定义了以下布局常量，可以根据需要调整：

```c
#define SIDEBAR_ITEM_HEIGHT 30        // 项目高度
#define SIDEBAR_ITEM_PADDING 5        // 项目内边距
#define SIDEBAR_SEPARATOR_HEIGHT 1    // 分隔线高度
#define SIDEBAR_WIDTH 200             // 侧边栏宽度（在头文件中）
```

## 扩展功能指南

### 1. 添加新的项目类型

要添加新的项目类型，需要：

1. 在`SidebarItemType`枚举中添加新类型
2. 在`load_icon`函数中添加图标加载逻辑
3. 在`sidebar_draw_item`中添加绘制逻辑
4. 创建相应的添加函数

**示例 - 添加网络驱动器类型:**
```c
// 在枚举中添加
typedef enum {
    SIDEBAR_ITEM_QUICK_ACCESS,
    SIDEBAR_ITEM_DRIVE,
    SIDEBAR_ITEM_NETWORK_DRIVE,  // 新增
    SIDEBAR_ITEM_SEPARATOR
} SidebarItemType;

// 在load_icon中添加
case SIDEBAR_ITEM_NETWORK_DRIVE:
    icon_path = "assets/icons/network.png";
    break;

// 创建添加函数
static void sidebar_add_network_drives(Sidebar *sidebar) {
    // 实现网络驱动器添加逻辑
}
```

### 2. 添加右键菜单支持

扩展事件处理函数以支持右键菜单：

```c
// 在sidebar_handle_event中添加
else if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
    if (event->button.button == SDL_BUTTON_RIGHT) {
        // 显示右键菜单
        show_context_menu(sidebar, event->button.x, event->button.y);
        return true;
    }
}
```

### 3. 添加拖放支持

实现拖放功能需要：

```c
// 添加拖放状态
typedef struct Sidebar {
    // ... 现有成员
    bool is_dragging;
    int drag_source_index;
    int drag_target_index;
} Sidebar;

// 在事件处理中添加拖放逻辑
if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
    // 开始拖放
    sidebar->is_dragging = true;
    sidebar->drag_source_index = sidebar_get_item_at(sidebar, x, y);
}
```

### 4. 添加搜索功能

实现搜索功能：

```c
// 添加搜索相关成员
typedef struct Sidebar {
    // ... 现有成员
    char search_text[256];
    bool is_searching;
    int filtered_item_count;
    SidebarItem filtered_items[MAX_SIDEBAR_ITEMS];
} Sidebar;

// 添加搜索函数
void sidebar_search(Sidebar *sidebar, const char *query) {
    // 实现搜索逻辑
    // 过滤项目并更新显示
}
```

### 5. 添加分组功能

实现项目分组：

```c
// 添加分组结构
typedef struct SidebarGroup {
    char *name;
    bool is_collapsed;
    int start_index;
    int item_count;
} SidebarGroup;

// 在Sidebar中添加分组数组
typedef struct Sidebar {
    // ... 现有成员
    SidebarGroup groups[MAX_SIDEBAR_GROUPS];
    int group_count;
} Sidebar;
```

## 性能优化建议

### 1. 图标缓存

实现图标缓存机制避免重复加载：

```c
// 添加图标缓存
typedef struct Sidebar {
    // ... 现有成员
    SDL_Texture *icon_cache[ICON_CACHE_SIZE];
    char *icon_paths[ICON_CACHE_SIZE];
} Sidebar;

// 在load_icon中检查缓存
SDL_Texture* load_icon(Sidebar *sidebar, SidebarItemType type) {
    // 检查缓存
    for (int i = 0; i < ICON_CACHE_SIZE; i++) {
        if (strcmp(sidebar->icon_paths[i], icon_path) == 0) {
            return sidebar->icon_cache[i];
        }
    }
    // 加载新图标并缓存
}
```

### 2. 虚拟化渲染

对于大量项目，实现虚拟化渲染：

```c
// 只渲染可见区域的项目
void sidebar_draw(Sidebar *sidebar) {
    // 计算可见范围
    int start_index = sidebar->scroll_offset / SIDEBAR_ITEM_HEIGHT;
    int end_index = start_index + (sidebar->rect.h / SIDEBAR_ITEM_HEIGHT) + 1;
    
    // 限制范围
    if (start_index < 0) start_index = 0;
    if (end_index > sidebar->item_count) end_index = sidebar->item_count;
    
    // 只绘制可见项目
    for (int i = start_index; i < end_index; i++) {
        sidebar_draw_item(sidebar, i);
    }
}
```

## 调试和故障排除

### 常见问题

1. **项目不显示**
   - 检查`item_count`是否正确
   - 验证项目坐标计算
   - 确认滚动偏移处理

2. **图标加载失败**
   - 检查图标文件路径
   - 验证SDL_image初始化
   - 查看控制台错误信息

3. **事件处理异常**
   - 确认事件坐标转换正确
   - 检查边界条件处理
   - 验证回调函数设置

### 调试工具

添加调试输出：

```c
#ifdef DEBUG_SIDEBAR
#define SIDEBAR_DEBUG(fmt, ...) printf("[SIDEBAR] " fmt "\n", ##__VA_ARGS__)
#else
#define SIDEBAR_DEBUG(fmt, ...)
#endif

// 在关键函数中添加调试信息
void sidebar_draw(Sidebar *sidebar) {
    SIDEBAR_DEBUG("绘制侧边栏，项目数量: %d", sidebar->item_count);
    // ... 绘制逻辑
}
```

## 测试指南

### 单元测试

为关键函数创建测试：

```c
void test_sidebar_creation() {
    // 测试侧边栏创建
    Sidebar *sidebar = sidebar_new(mock_window);
    assert(sidebar != NULL);
    assert(sidebar->item_count == 0);
    sidebar_free(sidebar);
}

void test_sidebar_item_addition() {
    // 测试项目添加
    Sidebar *sidebar = sidebar_new(mock_window);
    // 添加测试项目
    // 验证项目数量
    sidebar_free(sidebar);
}
```

### 集成测试

测试与其他模块的交互：

```c
void test_sidebar_file_system_integration() {
    // 测试文件系统集成
    // 验证路径解析
    // 测试驱动器检测
}
```

## 版本历史

- **v1.0**: 基础侧边栏功能
- **v1.1**: 添加滚动支持
- **v1.2**: 改进事件处理
- **v1.3**: 添加驱动器刷新功能

## 贡献指南

1. 遵循现有的代码风格
2. 添加适当的注释和文档
3. 为新功能编写测试
4. 更新本使用指南

## 联系方式

如有问题或建议，请联系开发团队或提交issue。 