# Toolbar模块使用指南

## 概述

Toolbar模块是FileScope应用程序的核心UI组件之一，负责提供文件浏览的导航和控制功能。该模块实现了完整的工具栏界面，包括按钮管理、事件处理、历史记录、搜索功能和地址栏等特性。

## 模块结构

### 核心数据结构

#### ToolbarButtonType 枚举
```c
typedef enum {
    BUTTON_BACK,        // 后退按钮
    BUTTON_FORWARD,     // 前进按钮
    BUTTON_UP,          // 上一级按钮
    BUTTON_HOME,        // 主目录按钮
    BUTTON_REFRESH,     // 刷新按钮
    BUTTON_SEARCH,      // 搜索按钮
    BUTTON_VIEW,        // 视图切换按钮
    BUTTON_COUNT        // 按钮总数（用于数组大小）
} ToolbarButtonType;
```

#### ToolbarButton 结构体
```c
typedef struct {
    ToolbarButtonType type;    // 按钮类型
    SDL_Rect rect;             // 按钮区域（位置和大小）
    const char *tooltip;       // 提示文本
    bool enabled;              // 是否启用
    bool hovered;              // 是否悬停
    bool pressed;              // 是否按下
} ToolbarButton;
```

#### Toolbar 主结构体
```c
typedef struct Toolbar {
    struct Window *app;        // 应用程序窗口引用
    SDL_Rect rect;             // 工具栏区域
    ToolbarButton buttons[BUTTON_COUNT];  // 按钮数组
    int button_count;          // 按钮数量
    
    // 历史记录管理
    char **history;            // 历史路径数组
    int history_capacity;      // 历史记录容量
    int history_count;         // 历史记录数量
    int history_index;         // 当前历史记录索引

    // 搜索功能
    char search_text[256];     // 搜索输入内容
    bool search_active;        // 是否处于搜索输入状态
    int search_cursor_pos;     // 光标位置
    
    // 地址栏功能
    char address_bar_text[1024];  // 地址栏输入内容
    bool address_bar_active;      // 是否处于地址栏输入状态
    int address_bar_cursor_pos;   // 地址栏光标位置
    bool address_bar_editing;     // 是否正在编辑地址栏
} Toolbar;
```

## 核心功能

### 1. 工具栏生命周期管理

#### 创建工具栏
```c
Toolbar* toolbar = toolbar_new(window);
if (!toolbar) {
    // 处理创建失败
    return NULL;
}
```

#### 释放工具栏
```c
toolbar_free(toolbar);
toolbar = NULL;
```

### 2. 按钮操作

#### 设置按钮状态
```c
// 启用/禁用特定按钮
toolbar_set_button_enabled(toolbar, BUTTON_BACK, true);
toolbar_set_button_enabled(toolbar, BUTTON_FORWARD, false);

// 更新所有按钮状态
toolbar_update_button_states(toolbar);
```

#### 执行按钮操作
```c
// 后退操作
bool success = toolbar_go_back(toolbar);

// 前进操作
success = toolbar_go_forward(toolbar);

// 上一级操作
success = toolbar_go_up(toolbar);

// 返回主目录
success = toolbar_go_home(toolbar);

// 刷新视图
success = toolbar_refresh(toolbar);

// 切换视图模式
success = toolbar_toggle_view(toolbar);

// 执行搜索
success = toolbar_search(toolbar, "search_term");
```

### 3. 历史记录管理

#### 通知目录变更
```c
// 当用户导航到新目录时调用
toolbar_notify_directory_changed(toolbar, "/new/path");
```

#### 历史导航
```c
// 检查是否可以后退
if (toolbar->history_index > 0) {
    // 可以后退
}

// 检查是否可以前进
if (toolbar->history_index < toolbar->history_count - 1) {
    // 可以前进
}
```

### 4. 搜索功能

#### 激活搜索
```c
// 开始搜索输入
toolbar_search_start(toolbar);

// 停止搜索
toolbar_search_stop(toolbar);
```

#### 处理搜索输入
```c
// 处理文本输入
toolbar_search_handle_text(toolbar, "search_text");

// 处理键盘输入
toolbar_search_handle_key(toolbar, SDL_SCANCODE_BACKSPACE);
```

### 5. 地址栏功能

#### 地址栏操作
```c
// 激活地址栏
toolbar_address_bar_start(toolbar);

// 停止地址栏
toolbar_address_bar_stop(toolbar);

// 处理地址栏文本输入
toolbar_address_bar_handle_text(toolbar, "new_path");

// 处理地址栏键盘输入
toolbar_address_bar_handle_key(toolbar, SDL_SCANCODE_RETURN);

// 导航到指定路径
toolbar_address_bar_navigate(toolbar, "/target/path");
```

### 6. 事件处理

#### 处理SDL事件
```c
SDL_Event event;
// ... 获取事件 ...

bool handled = toolbar_handle_event(toolbar, &event);
if (handled) {
    // 事件已被工具栏处理
}
```

#### 绘制工具栏
```c
// 在渲染循环中调用
toolbar_draw(toolbar);
```

## 使用实例

### 实例1：基本工具栏集成

```c
#include "toolbar.h"
#include "main_window.h"

int main() {
    // 创建主窗口
    Window *window = window_new("FileScope", 800, 600);
    if (!window) return -1;
    
    // 创建工具栏
    Toolbar *toolbar = toolbar_new(window);
    if (!toolbar) {
        window_free(window);
        return -1;
    }
    
    // 主循环
    SDL_Event event;
    bool running = true;
    
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
            
            // 处理工具栏事件
            toolbar_handle_event(toolbar, &event);
        }
        
        // 渲染
        SDL_SetRenderDrawColor(window->renderer, 255, 255, 255, 255);
        SDL_RenderClear(window->renderer);
        
        // 绘制工具栏
        toolbar_draw(toolbar);
        
        SDL_RenderPresent(window->renderer);
    }
    
    // 清理
    toolbar_free(toolbar);
    window_free(window);
    
    return 0;
}
```

### 实例2：自定义按钮操作

```c
// 自定义按钮操作回调
void custom_button_action(Toolbar *toolbar, ToolbarButtonType button_type) {
    switch (button_type) {
        case BUTTON_REFRESH:
            printf("执行自定义刷新操作\n");
            // 实现自定义刷新逻辑
            break;
            
        case BUTTON_VIEW:
            printf("执行自定义视图切换\n");
            // 实现自定义视图切换逻辑
            break;
            
        default:
            break;
    }
}

// 在execute_button_action函数中集成
static void execute_button_action(Toolbar *toolbar, ToolbarButton *button) {
    if (!toolbar || !button || !button->enabled) {
        return;
    }
    
    // 调用自定义操作
    custom_button_action(toolbar, button->type);
    
    // 原有的按钮操作逻辑...
}
```

### 实例3：扩展工具栏功能

```c
// 添加新的按钮类型
typedef enum {
    BUTTON_BACK,
    BUTTON_FORWARD,
    BUTTON_UP,
    BUTTON_HOME,
    BUTTON_REFRESH,
    BUTTON_SEARCH,
    BUTTON_VIEW,
    BUTTON_COPY,        // 新增复制按钮
    BUTTON_PASTE,       // 新增粘贴按钮
    BUTTON_DELETE,      // 新增删除按钮
    BUTTON_COUNT
} ToolbarButtonType;

// 在toolbar_new函数中初始化新按钮
void init_copy_button(Toolbar *toolbar, int *button_x) {
    toolbar->buttons[BUTTON_COPY].type = BUTTON_COPY;
    toolbar->buttons[BUTTON_COPY].rect.x = *button_x;
    toolbar->buttons[BUTTON_COPY].rect.y = (TOOLBAR_HEIGHT - BUTTON_SIZE) / 2;
    toolbar->buttons[BUTTON_COPY].rect.w = BUTTON_SIZE;
    toolbar->buttons[BUTTON_COPY].rect.h = BUTTON_SIZE;
    toolbar->buttons[BUTTON_COPY].tooltip = "复制";
    toolbar->buttons[BUTTON_COPY].enabled = true;
    
    *button_x += BUTTON_SIZE + BUTTON_SPACING;
}

// 在draw_toolbar_button函数中添加新按钮的绘制逻辑
case BUTTON_COPY:
    // 绘制复制图标
    draw_copy_icon(renderer, icon_x, icon_y, icon_size);
    break;
```

### 实例4：历史记录管理

```c
// 自定义历史记录管理
void custom_history_management(Toolbar *toolbar) {
    // 限制历史记录数量
    if (toolbar->history_count > 50) {
        // 删除最旧的历史记录
        free(toolbar->history[0]);
        memmove(&toolbar->history[0], &toolbar->history[1], 
                (toolbar->history_count - 1) * sizeof(char*));
        toolbar->history_count--;
        toolbar->history_index--;
    }
    
    // 添加路径到历史记录
    add_to_history(toolbar, "/new/path");
}

// 历史记录导航
void navigate_with_history(Toolbar *toolbar, int target_index) {
    if (target_index >= 0 && target_index < toolbar->history_count) {
        toolbar->history_index = target_index;
        const char *path = toolbar->history[target_index];
        
        // 导航到指定路径
        // ... 实现导航逻辑 ...
        
        // 更新按钮状态
        toolbar_update_button_states(toolbar);
    }
}
```

### 实例5：搜索功能集成

```c
// 自定义搜索处理
void custom_search_handler(Toolbar *toolbar, const char *search_term) {
    if (strlen(search_term) > 0) {
        printf("执行搜索: %s\n", search_term);
        
        // 实现搜索逻辑
        // 1. 在当前目录中搜索文件
        // 2. 更新文件列表视图
        // 3. 高亮搜索结果
        
        // 通知主窗口更新
        MainWindow *main_window = (MainWindow*)toolbar->app->user_data;
        if (main_window && main_window->file_list_view) {
            // 调用文件列表视图的搜索API
            // file_list_view_search(main_window->file_list_view, search_term);
        }
    }
}

// 集成到搜索处理函数
void toolbar_search_handle_text(Toolbar *toolbar, const char *text) {
    if (!toolbar || !text) return;
    
    // 调用自定义搜索处理
    custom_search_handler(toolbar, text);
    
    // 原有的搜索处理逻辑...
}
```

## 样式定制

### 颜色配置
```c
// 在toolbar.c中修改颜色常量
static const SDL_Color TOOLBAR_BG_COLOR = {240, 240, 240, 255};      // 背景色
static const SDL_Color BUTTON_COLOR = {200, 200, 200, 255};          // 按钮颜色
static const SDL_Color BUTTON_HOVER_COLOR = {180, 180, 180, 255};    // 悬停颜色
static const SDL_Color BUTTON_ACTIVE_COLOR = {160, 160, 160, 255};   // 激活颜色
static const SDL_Color BUTTON_DISABLED_COLOR = {220, 220, 220, 128}; // 禁用颜色
static const SDL_Color BUTTON_BORDER_COLOR = {100, 100, 100, 255};   // 边框颜色
static const SDL_Color BUTTON_ICON_COLOR = {50, 50, 50, 255};       // 图标颜色
```

### 尺寸配置
```c
// 在toolbar.h中修改尺寸常量
#define TOOLBAR_HEIGHT 40      // 工具栏高度
#define BUTTON_PADDING 5       // 按钮内边距
#define BUTTON_SIZE 30         // 按钮尺寸
#define BUTTON_SPACING 10      // 按钮间距
```

## 最佳实践

### 1. 内存管理
- 始终检查函数返回值，确保内存分配成功
- 在释放工具栏时，确保所有子资源都被正确释放
- 使用calloc初始化结构体，避免未初始化内存

### 2. 错误处理
- 在每个函数开始时验证参数有效性
- 提供有意义的错误信息和调试输出
- 实现优雅的错误恢复机制

### 3. 性能优化
- 避免在渲染循环中进行复杂的计算
- 使用适当的数据结构管理历史记录
- 实现事件过滤，只处理相关事件

### 4. 代码组织
- 将相关功能组织在独立的函数中
- 使用清晰的命名约定
- 添加详细的注释说明复杂逻辑

### 5. 扩展性
- 设计模块化架构，便于添加新功能
- 使用回调函数支持自定义操作
- 保持接口的一致性和稳定性

## 常见问题

### Q1: 如何添加新的工具栏按钮？
A1: 按照以下步骤：
1. 在`ToolbarButtonType`枚举中添加新类型
2. 在`toolbar_new`函数中初始化新按钮
3. 在`draw_toolbar_button`函数中添加绘制逻辑
4. 在`execute_button_action`函数中添加操作逻辑

### Q2: 如何处理工具栏的自定义事件？
A2: 在`toolbar_handle_event`函数中添加新的事件类型处理，或者使用回调函数机制。

### Q3: 如何修改工具栏的样式？
A3: 修改颜色常量、尺寸常量，或者在`draw_toolbar_button`函数中自定义绘制逻辑。

### Q4: 如何集成工具栏与其他模块？
A4: 使用`toolbar->app->user_data`访问主窗口，通过主窗口访问其他模块。

## 总结

Toolbar模块提供了完整的工具栏功能实现，包括按钮管理、事件处理、历史记录、搜索和地址栏等特性。通过本指南，开发者可以快速理解模块结构，掌握使用方法，并根据需要进行功能扩展和定制。

该模块设计灵活，接口清晰，为FileScope应用程序提供了强大的用户交互能力。 