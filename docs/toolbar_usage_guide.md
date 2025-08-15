# Toolbar模块使用指南

## 概述

Toolbar模块是FileScope应用程序的核心UI组件之一，负责提供文件浏览的导航和控制功能。该模块实现了完整的工具栏界面，包括按钮管理、事件处理、历史记录、搜索功能和地址栏等特性。

**文件位置**: `src/toolbar.c` 和 `include/toolbar.h`

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
    BUTTON_COUNT        // 按钮总数
} ToolbarButtonType;
```

#### ToolbarButton 结构体
```c
typedef struct {
    ToolbarButtonType type;    // 按钮类型
    SDL_Rect rect;             // 按钮区域
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

**目标**: 创建一个基本的工具栏应用程序，展示工具栏的核心功能

**文件位置**: `examples/basic_toolbar.c`

```c
#include "toolbar.h"
#include "main_window.h"
#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

int main() {
    // 步骤1: 初始化SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL初始化失败: %s\n", SDL_GetError());
        return -1;
    }
    
    // 步骤2: 创建主窗口
    Window *window = window_new("FileScope - 工具栏示例", 800, 600);
    if (!window) {
        fprintf(stderr, "窗口创建失败\n");
        SDL_Quit();
        return -1;
    }
    
    // 步骤3: 创建工具栏
    Toolbar *toolbar = toolbar_new(window);
    if (!toolbar) {
        fprintf(stderr, "工具栏创建失败\n");
        window_free(window);
        SDL_Quit();
        return -1;
    }
    
    printf("工具栏创建成功，按钮数量: %d\n", toolbar->button_count);
    
    // 步骤4: 设置初始按钮状态
    toolbar_update_button_states(toolbar);
    
    // 步骤5: 主事件循环
    SDL_Event event;
    bool running = true;
    
    while (running) {
        // 处理事件
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    running = false;
                    break;
                    
                case SDL_EVENT_KEYDOWN:
                    // 处理键盘快捷键
                    if (event.key.keysym.sym == SDLK_F5) {
                        printf("手动刷新工具栏状态\n");
                        toolbar_update_button_states(toolbar);
                    } else if (event.key.keysym.sym == SDLK_F1) {
                        printf("显示工具栏信息\n");
                        printf("工具栏位置: (%d, %d, %d, %d)\n", 
                               toolbar->rect.x, toolbar->rect.y, 
                               toolbar->rect.w, toolbar->rect.h);
                        printf("历史记录数量: %d\n", toolbar->history_count);
                    }
                    break;
                    
                case SDL_EVENT_MOUSEBUTTONDOWN:
                    // 显示鼠标点击信息
                    printf("鼠标点击: (%d, %d)\n", event.button.x, event.button.y);
                    break;
                    
                default:
                    // 处理工具栏事件
                    bool handled = toolbar_handle_event(toolbar, &event);
                    if (handled) {
                        printf("工具栏处理了事件类型: %d\n", event.type);
                    }
                    break;
            }
        }
        
        // 步骤6: 渲染循环
        SDL_SetRenderDrawColor(window->renderer, 255, 255, 255, 255);
        SDL_RenderClear(window->renderer);
        
        // 绘制工具栏
        toolbar_draw(toolbar);
        
        // 更新显示
        SDL_RenderPresent(window->renderer);
        
        // 控制帧率
        SDL_Delay(16); // 约60FPS
    }
    
    // 步骤7: 清理资源
    printf("正在清理资源...\n");
    toolbar_free(toolbar);
    window_free(window);
    SDL_Quit();
    
    printf("程序正常退出\n");
    return 0;
}
```

**编译和运行**:
```bash
# 创建示例目录
mkdir examples && cd examples

# 创建Makefile
cat > Makefile << 'EOF'
CC = gcc
CFLAGS = -Wall -Wextra -std=c99
LDFLAGS = -lSDL2
INCLUDES = -I../include
SOURCES = basic_toolbar.c ../src/toolbar.c ../src/window.c
TARGET = basic_toolbar

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(INCLUDES) $(SOURCES) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)
EOF

# 编译
make

# 运行
./basic_toolbar
```

**功能说明**:
- 创建基本的工具栏应用程序
- 支持鼠标和键盘事件处理
- 提供调试信息和状态显示
- 实现完整的资源管理
- 支持F5刷新和F1信息显示快捷键

**关键点**:
- SDL初始化和清理
- 窗口和工具栏的创建顺序
- 事件循环的处理流程
- 渲染循环的实现
- 资源的正确释放顺序

### 实例2：自定义按钮操作

**目标**: 为工具栏按钮添加自定义操作逻辑，实现特定的业务功能

**文件位置**: `examples/custom_button_actions.c`

```c
#include "toolbar.h"
#include "main_window.h"
#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

// 自定义按钮操作回调函数
void custom_button_action(Toolbar *toolbar, ToolbarButtonType button_type) {
    if (!toolbar) return;
    
    switch (button_type) {
        case BUTTON_REFRESH:
            printf("执行自定义刷新操作\n");
            custom_refresh_operation(toolbar);
            break;
            
        case BUTTON_VIEW:
            printf("执行自定义视图切换\n");
            custom_view_toggle(toolbar);
            break;
            
        case BUTTON_SEARCH:
            printf("执行自定义搜索操作\n");
            custom_search_operation(toolbar);
            break;
            
        case BUTTON_HOME:
            printf("执行自定义主目录操作\n");
            custom_home_operation(toolbar);
            break;
            
        default:
            // 其他按钮使用默认操作
            break;
    }
}

// 自定义刷新操作实现
void custom_refresh_operation(Toolbar *toolbar) {
    printf("正在执行自定义刷新...\n");
    
    // 1. 显示刷新进度指示器
    printf("显示刷新进度指示器\n");
    
    // 2. 执行自定义刷新逻辑
    // 例如：重新扫描目录、更新文件列表、清除缓存等
    printf("重新扫描目录内容\n");
    printf("更新文件列表视图\n");
    printf("清除缓存数据\n");
    
    // 3. 延迟模拟刷新过程
    SDL_Delay(1000);
    
    // 4. 隐藏刷新指示器
    printf("隐藏刷新进度指示器\n");
    
    // 5. 通知主窗口更新
    if (toolbar->app && toolbar->app->user_data) {
        MainWindow *main_window = (MainWindow*)toolbar->app->user_data;
        printf("通知主窗口刷新完成\n");
        // 调用主窗口的刷新方法
        // main_window_refresh(main_window);
    }
    
    printf("自定义刷新完成\n");
}

// 自定义视图切换实现
void custom_view_toggle(Toolbar *toolbar) {
    static int current_view_mode = 0;
    const char *view_modes[] = {"列表视图", "图标视图", "详细信息视图", "缩略图视图"};
    
    // 循环切换视图模式
    current_view_mode = (current_view_mode + 1) % 4;
    
    printf("切换到视图模式: %s\n", view_modes[current_view_mode]);
    
    // 更新工具栏按钮状态
    toolbar_update_button_states(toolbar);
    
    // 通知主窗口切换视图
    if (toolbar->app && toolbar->app->user_data) {
        MainWindow *main_window = (MainWindow*)toolbar->app->user_data;
        printf("通知主窗口切换视图模式: %d\n", current_view_mode);
        // 调用主窗口的视图切换方法
        // main_window_set_view_mode(main_window, current_view_mode);
    }
}

// 自定义搜索操作实现
void custom_search_operation(Toolbar *toolbar) {
    printf("激活自定义搜索功能\n");
    
    // 1. 启动搜索模式
    toolbar_search_start(toolbar);
    
    // 2. 显示搜索选项
    printf("搜索选项:\n");
    printf("1. 按文件名搜索\n");
    printf("2. 按文件内容搜索\n");
    printf("3. 按文件大小搜索\n");
    printf("4. 按修改日期搜索\n");
    
    // 3. 设置搜索过滤器
    printf("设置默认搜索过滤器: *.txt, *.doc, *.pdf\n");
}

// 自定义主目录操作实现
void custom_home_operation(Toolbar *toolbar) {
    printf("执行自定义主目录操作\n");
    
    // 1. 获取用户主目录
    const char *home_path = getenv("HOME");
    if (!home_path) {
        home_path = getenv("USERPROFILE"); // Windows支持
    }
    
    if (home_path) {
        printf("用户主目录: %s\n", home_path);
        
        // 2. 导航到主目录
        printf("导航到主目录\n");
        
        // 3. 更新地址栏
        strncpy(toolbar->address_bar_text, home_path, sizeof(toolbar->address_bar_text) - 1);
        toolbar->address_bar_text[sizeof(toolbar->address_bar_text) - 1] = '\0';
        
        // 4. 添加到历史记录
        toolbar_notify_directory_changed(toolbar, home_path);
        
        printf("已导航到主目录\n");
    } else {
        printf("无法获取用户主目录\n");
    }
}

// 集成到工具栏事件处理
void integrate_custom_actions(Toolbar *toolbar) {
    if (!toolbar) return;
    
    printf("集成自定义按钮操作到工具栏\n");
    
    // 设置自定义操作标志
    toolbar->custom_actions_enabled = true;
    
    // 注册自定义操作回调
    toolbar->custom_action_callback = custom_button_action;
    
    printf("自定义按钮操作集成完成\n");
}

// 主函数
int main() {
    // 初始化SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL初始化失败: %s\n", SDL_GetError());
        return -1;
    }
    
    // 创建主窗口
    Window *window = window_new("自定义按钮操作示例", 800, 600);
    if (!window) {
        fprintf(stderr, "窗口创建失败\n");
        SDL_Quit();
        return -1;
    }
    
    // 创建工具栏
    Toolbar *toolbar = toolbar_new(window);
    if (!toolbar) {
        fprintf(stderr, "工具栏创建失败\n");
        window_free(window);
        SDL_Quit();
        return -1;
    }
    
    // 集成自定义操作
    integrate_custom_actions(toolbar);
    
    // 主事件循环
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
        toolbar_draw(toolbar);
        SDL_RenderPresent(window->renderer);
        
        SDL_Delay(16);
    }
    
    // 清理
    toolbar_free(toolbar);
    window_free(window);
    SDL_Quit();
    
    return 0;
}
```

**编译和运行**:
```bash
# 编译
gcc -Wall -Wextra -std=c99 -I../include custom_button_actions.c ../src/toolbar.c ../src/window.c -o custom_button_actions -lSDL2

# 运行
./custom_button_actions
```

**功能说明**:
- 为不同按钮类型实现自定义操作
- 支持刷新、视图切换、搜索、主目录等自定义功能
- 提供详细的调试输出和状态信息
- 实现与主窗口的集成通信

**关键点**:
- 自定义操作回调函数的注册
- 按钮操作的具体实现逻辑
- 与主窗口的通信机制
- 状态更新和界面刷新

### 实例3：扩展工具栏功能

**目标**: 为工具栏添加新的按钮类型和功能，扩展工具栏的能力

**文件位置**: `examples/extended_toolbar.c`

#### 3.1 扩展按钮类型

```c
// 扩展的按钮类型枚举
typedef enum {
    BUTTON_BACK,        // 后退按钮
    BUTTON_FORWARD,     // 前进按钮
    BUTTON_UP,          // 上一级按钮
    BUTTON_HOME,        // 主目录按钮
    BUTTON_REFRESH,     // 刷新按钮
    BUTTON_SEARCH,      // 搜索按钮
    BUTTON_VIEW,        // 视图切换按钮
    BUTTON_COPY,        // 新增复制按钮
    BUTTON_PASTE,       // 新增粘贴按钮
    BUTTON_DELETE,      // 新增删除按钮
    BUTTON_CUT,         // 新增剪切按钮
    BUTTON_SELECT_ALL,  // 新增全选按钮
    BUTTON_COUNT        // 按钮总数
} ToolbarButtonType;
```

#### 3.2 扩展工具栏结构体

```c
typedef struct Toolbar {
    struct Window *app;        // 应用程序窗口引用
    SDL_Rect rect;             // 工具栏区域
    ToolbarButton buttons[BUTTON_COUNT];  // 按钮数组
    int button_count;          // 按钮数量
    
    // 原有功能字段...
    
    // 新增功能字段
    bool has_selection;        // 是否有选中的文件/文件夹
    bool clipboard_has_data;   // 剪贴板是否有数据
    char clipboard_path[1024]; // 剪贴板中的路径
    int selected_count;        // 选中的项目数量
} Toolbar;
```

#### 3.3 初始化新按钮

```c
// 初始化复制按钮
void init_copy_button(Toolbar *toolbar, int *button_x) {
    toolbar->buttons[BUTTON_COPY].type = BUTTON_COPY;
    toolbar->buttons[BUTTON_COPY].rect.x = *button_x;
    toolbar->buttons[BUTTON_COPY].rect.y = (TOOLBAR_HEIGHT - BUTTON_SIZE) / 2;
    toolbar->buttons[BUTTON_COPY].rect.w = BUTTON_SIZE;
    toolbar->buttons[BUTTON_COPY].rect.h = BUTTON_SIZE;
    toolbar->buttons[BUTTON_COPY].tooltip = "复制 (Ctrl+C)";
    toolbar->buttons[BUTTON_COPY].enabled = false; // 初始状态禁用
    
    *button_x += BUTTON_SIZE + BUTTON_SPACING;
}

// 初始化粘贴按钮
void init_paste_button(Toolbar *toolbar, int *button_x) {
    toolbar->buttons[BUTTON_PASTE].type = BUTTON_PASTE;
    toolbar->buttons[BUTTON_PASTE].rect.x = *button_x;
    toolbar->buttons[BUTTON_PASTE].rect.y = (TOOLBAR_HEIGHT - BUTTON_SIZE) / 2;
    toolbar->buttons[BUTTON_PASTE].rect.w = BUTTON_SIZE;
    toolbar->buttons[BUTTON_PASTE].rect.h = BUTTON_SIZE;
    toolbar->buttons[BUTTON_PASTE].tooltip = "粘贴 (Ctrl+V)";
    toolbar->buttons[BUTTON_PASTE].enabled = false; // 初始状态禁用
    
    *button_x += BUTTON_SIZE + BUTTON_SPACING;
}

// 初始化删除按钮
void init_delete_button(Toolbar *toolbar, int *button_x) {
    toolbar->buttons[BUTTON_DELETE].type = BUTTON_DELETE;
    toolbar->buttons[BUTTON_DELETE].rect.x = *button_x;
    toolbar->buttons[BUTTON_DELETE].rect.y = (TOOLBAR_HEIGHT - BUTTON_SIZE) / 2;
    toolbar->buttons[BUTTON_DELETE].rect.w = BUTTON_SIZE;
    toolbar->buttons[BUTTON_DELETE].rect.h = BUTTON_SIZE;
    toolbar->buttons[BUTTON_DELETE].tooltip = "删除 (Del)";
    toolbar->buttons[BUTTON_DELETE].enabled = false; // 初始状态禁用
    
    *button_x += BUTTON_SIZE + BUTTON_SPACING;
}
```

#### 3.4 添加按钮绘制逻辑

```c
// 绘制复制图标
void draw_copy_icon(SDL_Renderer *renderer, int x, int y, int size) {
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    
    // 绘制两个重叠的矩形表示复制
    SDL_Rect rect1 = {x + size/4, y + size/4, size/2, size/2};
    SDL_Rect rect2 = {x + size/2, y + size/2, size/2, size/2};
    
    SDL_RenderFillRect(renderer, &rect1);
    SDL_RenderFillRect(renderer, &rect2);
    
    // 绘制边框
    SDL_RenderDrawRect(renderer, &rect1);
    SDL_RenderDrawRect(renderer, &rect2);
}

// 绘制粘贴图标
void draw_paste_icon(SDL_Renderer *renderer, int x, int y, int size) {
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    
    // 绘制剪贴板图标
    SDL_Rect rect = {x + size/4, y + size/6, size/2, size*2/3};
    SDL_RenderFillRect(renderer, &rect);
    
    // 绘制顶部夹子
    SDL_Rect clip = {x + size/3, y + size/6, size/3, size/6};
    SDL_RenderFillRect(renderer, &clip);
}

// 绘制删除图标
void draw_delete_icon(SDL_Renderer *renderer, int x, int y, int size) {
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    
    // 绘制垃圾桶图标
    SDL_Rect rect = {x + size/4, y + size/3, size/2, size/2};
    SDL_RenderFillRect(renderer, &rect);
    
    // 绘制垃圾桶盖子
    SDL_Rect lid = {x + size/6, y + size/4, size*2/3, size/6};
    SDL_RenderFillRect(renderer, &lid);
}
```

#### 3.5 实现按钮操作逻辑

```c
// 执行复制操作
void execute_copy_operation(Toolbar *toolbar) {
    if (!toolbar->has_selection) {
        printf("没有选中的项目，无法复制\n");
        return;
    }
    
    printf("执行复制操作，选中项目数量: %d\n", toolbar->selected_count);
    
    // 1. 获取选中的文件列表
    // 2. 将文件信息复制到剪贴板
    // 3. 设置剪贴板状态
    toolbar->clipboard_has_data = true;
    strcpy(toolbar->clipboard_path, "复制到剪贴板");
    
    // 4. 通知主窗口更新
    if (toolbar->app && toolbar->app->user_data) {
        MainWindow *main_window = (MainWindow*)toolbar->app->user_data;
        printf("通知主窗口执行复制操作\n");
        // 调用主窗口的复制方法
        // main_window_copy_files(main_window);
    }
    
    printf("复制操作完成\n");
}

// 执行粘贴操作
void execute_paste_operation(Toolbar *toolbar) {
    if (!toolbar->clipboard_has_data) {
        printf("剪贴板中没有数据，无法粘贴\n");
        return;
    }
    
    printf("执行粘贴操作\n");
    
    // 1. 获取剪贴板中的文件信息
    // 2. 在当前目录中粘贴文件
    // 3. 更新文件列表
    
    // 4. 通知主窗口更新
    if (toolbar->app && toolbar->app->user_data) {
        MainWindow *main_window = (MainWindow*)toolbar->app->user_data;
        printf("通知主窗口执行粘贴操作\n");
        // 调用主窗口的粘贴方法
        // main_window_paste_files(main_window);
    }
    
    printf("粘贴操作完成\n");
}

// 执行删除操作
void execute_delete_operation(Toolbar *toolbar) {
    if (!toolbar->has_selection) {
        printf("没有选中的项目，无法删除\n");
        return;
    }
    
    printf("执行删除操作，选中项目数量: %d\n", toolbar->selected_count);
    
    // 1. 显示删除确认对话框
    printf("显示删除确认对话框\n");
    
    // 2. 删除选中的文件/文件夹
    // 3. 更新文件列表
    
    // 4. 通知主窗口更新
    if (toolbar->app && toolbar->app->user_data) {
        MainWindow *main_window = (MainWindow*)toolbar->app->user_data;
        printf("通知主窗口执行删除操作\n");
        // 调用主窗口的删除方法
        // main_window_delete_files(main_window);
    }
    
    printf("删除操作完成\n");
}
```

#### 3.6 更新按钮状态管理

```c
// 更新按钮状态管理函数
void toolbar_update_button_states(Toolbar *toolbar) {
    if (!toolbar) return;
    
    // 更新原有按钮状态
    toolbar->buttons[BUTTON_BACK].enabled = (toolbar->history_index > 0);
    toolbar->buttons[BUTTON_FORWARD].enabled = (toolbar->history_index < toolbar->history_count - 1);
    toolbar->buttons[BUTTON_UP].enabled = true;
    toolbar->buttons[BUTTON_HOME].enabled = true;
    toolbar->buttons[BUTTON_REFRESH].enabled = true;
    toolbar->buttons[BUTTON_SEARCH].enabled = true;
    toolbar->buttons[BUTTON_VIEW].enabled = true;
    
    // 更新新增按钮状态
    toolbar->buttons[BUTTON_COPY].enabled = toolbar->has_selection;
    toolbar->buttons[BUTTON_PASTE].enabled = toolbar->clipboard_has_data;
    toolbar->buttons[BUTTON_DELETE].enabled = toolbar->has_selection;
    toolbar->buttons[BUTTON_CUT].enabled = toolbar->has_selection;
    toolbar->buttons[BUTTON_SELECT_ALL].enabled = true;
}
```

#### 3.7 完整的扩展示例

```c
#include "toolbar.h"
#include "main_window.h"
#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

int main() {
    // 初始化SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL初始化失败: %s\n", SDL_GetError());
        return -1;
    }
    
    // 创建主窗口
    Window *window = window_new("扩展工具栏示例", 1000, 600);
    if (!window) {
        fprintf(stderr, "窗口创建失败\n");
        SDL_Quit();
        return -1;
    }
    
    // 创建工具栏
    Toolbar *toolbar = toolbar_new(window);
    if (!toolbar) {
        fprintf(stderr, "工具栏创建失败\n");
        window_free(window);
        SDL_Quit();
        return -1;
    }
    
    printf("扩展工具栏创建成功，按钮数量: %d\n", toolbar->button_count);
    
    // 模拟文件选择状态
    toolbar->has_selection = true;
    toolbar->selected_count = 3;
    toolbar->clipboard_has_data = true;
    
    // 更新按钮状态
    toolbar_update_button_states(toolbar);
    
    // 主事件循环
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
        toolbar_draw(toolbar);
        SDL_RenderPresent(window->renderer);
        
        SDL_Delay(16);
    }
    
    // 清理
    toolbar_free(toolbar);
    window_free(window);
    SDL_Quit();
    
    return 0;
}
```

**编译和运行**:
```bash
# 编译
gcc -Wall -Wextra -std=c99 -I../include extended_toolbar.c ../src/toolbar.c ../src/window.c -o extended_toolbar -lSDL2

# 运行
./extended_toolbar
```

**功能说明**:
- 添加了复制、粘贴、删除、剪切、全选等新按钮
- 实现了新按钮的图标绘制和操作逻辑
- 提供了完整的按钮状态管理
- 支持剪贴板操作和文件选择状态

**关键点**:
- 新按钮的完整添加流程
- 按钮状态的条件管理
- 图标绘制的实现方法
- 操作逻辑的集成方式

### 实例4：高级历史记录管理

**目标**: 实现高级历史记录功能，包括历史记录清理、搜索和智能导航

**文件位置**: `examples/advanced_history.c`

```c
#include "toolbar.h"
#include "main_window.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <SDL2/SDL.h>

// 高级历史记录管理
void advanced_history_management(Toolbar *toolbar) {
    if (!toolbar) return;
    
    printf("开始高级历史记录管理...\n");
    
    // 1. 限制历史记录数量，防止内存过度使用
    if (toolbar->history_count > 50) {
        printf("历史记录数量超过限制，正在清理...\n");
        
        int items_to_remove = toolbar->history_count - 50;
        for (int i = 0; i < items_to_remove; i++) {
            free(toolbar->history[0]);
            memmove(&toolbar->history[0], &toolbar->history[1], 
                    (toolbar->history_count - 1) * sizeof(char*));
            toolbar->history_count--;
            toolbar->history_index--;
        }
        
        printf("已清理 %d 条历史记录，当前数量: %d\n", 
               items_to_remove, toolbar->history_count);
    }
    
    // 2. 去重处理，避免重复的历史记录
    remove_duplicate_history(toolbar);
    
    // 3. 更新按钮状态
    toolbar_update_button_states(toolbar);
    
    printf("高级历史记录管理完成\n");
}

// 移除重复的历史记录
void remove_duplicate_history(Toolbar *toolbar) {
    if (!toolbar || toolbar->history_count <= 1) return;
    
    int removed_count = 0;
    
    for (int i = 0; i < toolbar->history_count - 1; i++) {
        for (int j = i + 1; j < toolbar->history_count; j++) {
            if (strcmp(toolbar->history[i], toolbar->history[j]) == 0) {
                // 发现重复记录，删除后面的
                free(toolbar->history[j]);
                
                // 移动后面的记录
                for (int k = j; k < toolbar->history_count - 1; k++) {
                    toolbar->history[k] = toolbar->history[k + 1];
                }
                
                toolbar->history_count--;
                removed_count++;
                
                // 调整当前索引
                if (toolbar->history_index >= j) {
                    toolbar->history_index--;
                }
                
                j--; // 重新检查当前位置
            }
        }
    }
    
    if (removed_count > 0) {
        printf("已移除 %d 条重复的历史记录\n", removed_count);
    }
}

// 历史记录搜索
void search_history(Toolbar *toolbar, const char *search_term) {
    if (!toolbar || !search_term) return;
    
    printf("搜索历史记录: %s\n", search_term);
    
    // 创建搜索结果数组
    int *search_results = malloc(toolbar->history_count * sizeof(int));
    int result_count = 0;
    
    if (!search_results) {
        printf("内存分配失败，无法搜索历史记录\n");
        return;
    }
    
    // 执行搜索
    for (int i = 0; i < toolbar->history_count; i++) {
        if (strstr(toolbar->history[i], search_term) != NULL) {
            search_results[result_count] = i;
            result_count++;
        }
    }
    
    // 显示搜索结果
    if (result_count > 0) {
        printf("找到 %d 条匹配的历史记录:\n", result_count);
        for (int i = 0; i < result_count; i++) {
            int index = search_results[i];
            printf("  [%d] %s\n", index, toolbar->history[index]);
        }
        
        // 提供选择导航的选项
        if (result_count == 1) {
            printf("自动导航到唯一匹配项\n");
            navigate_to_history_index(toolbar, search_results[0]);
        } else {
            printf("请选择要导航的历史记录项 (0-%d): ", result_count - 1);
            // 这里可以实现用户选择逻辑
        }
    } else {
        printf("未找到匹配的历史记录\n");
    }
    
    // 清理搜索结果
    free(search_results);
}

// 导航到指定的历史记录索引
void navigate_to_history_index(Toolbar *toolbar, int target_index) {
    if (!toolbar) return;
    
    if (target_index >= 0 && target_index < toolbar->history_count) {
        printf("导航到历史记录索引: %d\n", target_index);
        
        // 更新历史记录索引
        toolbar->history_index = target_index;
        const char *path = toolbar->history[target_index];
        
        printf("目标路径: %s\n", path);
        
        // 验证路径是否仍然有效
        if (validate_path(path)) {
            // 执行导航
            execute_navigation(toolbar, path);
            
            // 更新按钮状态
            toolbar_update_button_states(toolbar);
            
            printf("导航成功完成\n");
        } else {
            printf("路径无效，无法导航: %s\n", path);
            
            // 从历史记录中移除无效路径
            remove_invalid_history_item(toolbar, target_index);
        }
    } else {
        printf("无效的历史记录索引: %d (范围: 0-%d)\n", 
               target_index, toolbar->history_count - 1);
    }
}

// 验证路径是否有效
bool validate_path(const char *path) {
    if (!path) return false;
    
    // 检查路径是否为空
    if (strlen(path) == 0) return false;
    
    // 检查路径是否包含非法字符
    if (strchr(path, '<') || strchr(path, '>') || strchr(path, '|')) {
        return false;
    }
    
    // 这里可以添加更多的路径验证逻辑
    // 例如：检查文件系统权限、路径是否存在等
    
    return true;
}

// 执行导航操作
void execute_navigation(Toolbar *toolbar, const char *path) {
    if (!toolbar || !path) return;
    
    printf("正在导航到: %s\n", path);
    
    // 1. 通知主窗口开始导航
    if (toolbar->app && toolbar->app->user_data) {
        MainWindow *main_window = (MainWindow*)toolbar->app->user_data;
        printf("通知主窗口导航到: %s\n", path);
        // 调用主窗口的导航方法
        // main_window_navigate_to(main_window, path);
    }
    
    // 2. 更新地址栏显示
    strncpy(toolbar->address_bar_text, path, sizeof(toolbar->address_bar_text) - 1);
    toolbar->address_bar_text[sizeof(toolbar->address_bar_text) - 1] = '\0';
    
    // 3. 重置搜索和地址栏状态
    toolbar->search_active = false;
    toolbar->address_bar_active = false;
    toolbar->address_bar_editing = false;
    
    printf("导航操作完成\n");
}

// 移除无效的历史记录项
void remove_invalid_history_item(Toolbar *toolbar, int index) {
    if (!toolbar || index < 0 || index >= toolbar->history_count) return;
    
    printf("正在移除无效的历史记录项: %d\n", index);
    
    // 释放内存
    free(toolbar->history[index]);
    
    // 移动后面的记录
    for (int i = index; i < toolbar->history_count - 1; i++) {
        toolbar->history[i] = toolbar->history[i + 1];
    }
    
    toolbar->history_count--;
    
    // 调整当前索引
    if (toolbar->history_index >= index) {
        toolbar->history_index--;
    }
    
    printf("无效历史记录项已移除，当前数量: %d\n", toolbar->history_count);
}

// 历史记录统计信息
void print_history_statistics(Toolbar *toolbar) {
    if (!toolbar) return;
    
    printf("\n=== 历史记录统计信息 ===\n");
    printf("总记录数: %d\n", toolbar->history_count);
    printf("数组容量: %d\n", toolbar->history_capacity);
    printf("当前索引: %d\n", toolbar->history_index);
    printf("内存使用: %.2f KB\n", 
           (toolbar->history_count * sizeof(char*) + 
            toolbar->history_count * 100) / 1024.0);
    
    if (toolbar->history_count > 0) {
        printf("最早记录: %s\n", toolbar->history[0]);
        printf("最新记录: %s\n", toolbar->history[toolbar->history_count - 1]);
        printf("当前记录: %s\n", toolbar->history[toolbar->history_index]);
    }
    
    printf("========================\n\n");
}

// 主函数
int main() {
    // 初始化SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL初始化失败: %s\n", SDL_GetError());
        return -1;
    }
    
    // 创建主窗口
    Window *window = window_new("高级历史记录管理示例", 800, 600);
    if (!window) {
        fprintf(stderr, "窗口创建失败\n");
        SDL_Quit();
        return -1;
    }
    
    // 创建工具栏
    Toolbar *toolbar = toolbar_new(window);
    if (!toolbar) {
        fprintf(stderr, "工具栏创建失败\n");
        window_free(window);
        SDL_Quit();
        return -1;
    }
    
    printf("工具栏创建成功\n");
    
    // 添加一些测试历史记录
    toolbar_notify_directory_changed(toolbar, "/home/user");
    toolbar_notify_directory_changed(toolbar, "/home/user/documents");
    toolbar_notify_directory_changed(toolbar, "/home/user/pictures");
    toolbar_notify_directory_changed(toolbar, "/home/user/music");
    toolbar_notify_directory_changed(toolbar, "/home/user/videos");
    
    // 显示初始统计信息
    print_history_statistics(toolbar);
    
    // 执行高级历史记录管理
    advanced_history_management(toolbar);
    
    // 搜索历史记录
    search_history(toolbar, "user");
    
    // 显示最终统计信息
    print_history_statistics(toolbar);
    
    // 主事件循环
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
        toolbar_draw(toolbar);
        SDL_RenderPresent(window->renderer);
        
        SDL_Delay(16);
    }
    
    // 清理
    toolbar_free(toolbar);
    window_free(window);
    SDL_Quit();
    
    return 0;
}
```

**编译和运行**:
```bash
# 编译
gcc -Wall -Wextra -std=c99 -I../include advanced_history.c ../src/toolbar.c ../src/window.c -o advanced_history -lSDL2

# 运行
./advanced_history
```

**功能说明**:
- 实现历史记录数量限制和自动清理
- 支持历史记录去重处理
- 提供历史记录搜索功能
- 实现智能路径验证和导航
- 显示详细的历史记录统计信息

**关键点**:
- 内存管理和历史记录清理
- 重复记录的检测和移除
- 路径验证和错误处理
- 搜索功能的实现
- 统计信息的收集和显示

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