# Toolbar模块快速参考

## 🚀 快速开始

```c
#include "toolbar.h"

// 创建工具栏
Toolbar* toolbar = toolbar_new(window);

// 处理事件
toolbar_handle_event(toolbar, &event);

// 绘制工具栏
toolbar_draw(toolbar);

// 释放资源
toolbar_free(toolbar);
```

## 📋 核心API

### 按钮操作
| 函数 | 说明 | 示例 |
|------|------|------|
| `toolbar_go_back()` | 后退 | `toolbar_go_back(toolbar)` |
| `toolbar_go_forward()` | 前进 | `toolbar_go_forward(toolbar)` |
| `toolbar_go_up()` | 上一级 | `toolbar_go_up(toolbar)` |
| `toolbar_go_home()` | 主目录 | `toolbar_go_home(toolbar)` |
| `toolbar_refresh()` | 刷新 | `toolbar_refresh(toolbar)` |
| `toolbar_toggle_view()` | 切换视图 | `toolbar_toggle_view(toolbar)` |

### 搜索功能
| 函数 | 说明 | 示例 |
|------|------|------|
| `toolbar_search_start()` | 开始搜索 | `toolbar_search_start(toolbar)` |
| `toolbar_search_stop()` | 停止搜索 | `toolbar_search_stop(toolbar)` |
| `toolbar_search_handle_text()` | 处理文本输入 | `toolbar_search_handle_text(toolbar, "text")` |

### 地址栏功能
| 函数 | 说明 | 示例 |
|------|------|------|
| `toolbar_address_bar_start()` | 激活地址栏 | `toolbar_address_bar_start(toolbar)` |
| `toolbar_address_bar_navigate()` | 导航到路径 | `toolbar_address_bar_navigate(toolbar, "/path")` |

### 状态管理
| 函数 | 说明 | 示例 |
|------|------|------|
| `toolbar_set_button_enabled()` | 设置按钮状态 | `toolbar_set_button_enabled(toolbar, BUTTON_BACK, true)` |
| `toolbar_update_button_states()` | 更新所有按钮状态 | `toolbar_update_button_states(toolbar)` |
| `toolbar_notify_directory_changed()` | 通知目录变更 | `toolbar_notify_directory_changed(toolbar, "/path")` |

## 🎨 按钮类型

```c
typedef enum {
    BUTTON_BACK,        // 后退
    BUTTON_FORWARD,     // 前进
    BUTTON_UP,          // 上一级
    BUTTON_HOME,        // 主目录
    BUTTON_REFRESH,     // 刷新
    BUTTON_SEARCH,      // 搜索
    BUTTON_VIEW,        // 视图切换
    BUTTON_COUNT        // 总数
} ToolbarButtonType;
```

## 🔧 添加新按钮

### 1. 扩展枚举
```c
typedef enum {
    // ... 现有按钮 ...
    BUTTON_COPY,        // 新增复制按钮
    BUTTON_COUNT
} ToolbarButtonType;
```

### 2. 初始化按钮
```c
// 在toolbar_new函数中
toolbar->buttons[BUTTON_COPY].type = BUTTON_COPY;
toolbar->buttons[BUTTON_COPY].rect.x = button_x;
toolbar->buttons[BUTTON_COPY].rect.y = (TOOLBAR_HEIGHT - BUTTON_SIZE) / 2;
toolbar->buttons[BUTTON_COPY].rect.w = BUTTON_SIZE;
toolbar->buttons[BUTTON_COPY].rect.h = BUTTON_SIZE;
toolbar->buttons[BUTTON_COPY].tooltip = "复制";
toolbar->buttons[BUTTON_COPY].enabled = true;
```

### 3. 绘制图标
```c
// 在draw_toolbar_button函数中
case BUTTON_COPY:
    // 绘制复制图标
    draw_copy_icon(renderer, icon_x, icon_y, icon_size);
    break;
```

### 4. 处理操作
```c
// 在execute_button_action函数中
case BUTTON_COPY:
    copy_selected_files(toolbar);
    break;
```

## 🎯 事件处理

### 鼠标事件
- `SDL_EVENT_MOUSE_MOTION` - 鼠标移动（悬停效果）
- `SDL_EVENT_MOUSE_BUTTON_DOWN` - 鼠标按下
- `SDL_EVENT_MOUSE_BUTTON_UP` - 鼠标释放

### 键盘事件
- `SDL_EVENT_KEY_DOWN` - 按键按下
- `SDL_EVENT_TEXT_INPUT` - 文本输入

## 🎨 样式定制

### 颜色配置
```c
static const SDL_Color TOOLBAR_BG_COLOR = {240, 240, 240, 255};      // 背景
static const SDL_Color BUTTON_COLOR = {200, 200, 200, 255};          // 按钮
static const SDL_Color BUTTON_HOVER_COLOR = {180, 180, 180, 255};    // 悬停
static const SDL_Color BUTTON_ACTIVE_COLOR = {160, 160, 160, 255};   // 激活
static const SDL_Color BUTTON_DISABLED_COLOR = {220, 220, 220, 128}; // 禁用
```

### 尺寸配置
```c
#define TOOLBAR_HEIGHT 40      // 工具栏高度
#define BUTTON_PADDING 5       // 按钮内边距
#define BUTTON_SIZE 30         // 按钮尺寸
#define BUTTON_SPACING 10      // 按钮间距
```

## 📚 历史记录

### 结构
```c
char **history;            // 历史路径数组
int history_capacity;      // 历史记录容量
int history_count;         // 历史记录数量
int history_index;         // 当前历史记录索引
```

### 操作
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

## 🔍 搜索功能

### 状态
```c
char search_text[256];     // 搜索输入内容
bool search_active;        // 是否处于搜索输入状态
int search_cursor_pos;     // 光标位置
```

### 使用流程
1. `toolbar_search_start()` - 激活搜索
2. `toolbar_search_handle_text()` - 处理输入
3. `toolbar_search_stop()` - 停止搜索

## 🏠 地址栏功能

### 状态
```c
char address_bar_text[1024];  // 地址栏输入内容
bool address_bar_active;      // 是否处于地址栏输入状态
int address_bar_cursor_pos;   // 地址栏光标位置
bool address_bar_editing;     // 是否正在编辑地址栏
```

### 使用流程
1. `toolbar_address_bar_start()` - 激活地址栏
2. `toolbar_address_bar_handle_text()` - 处理输入
3. `toolbar_address_bar_navigate()` - 执行导航
4. `toolbar_address_bar_stop()` - 停止地址栏

## ⚡ 性能优化

### 渲染优化
- 避免在渲染循环中进行复杂计算
- 使用适当的数据结构管理历史记录
- 实现事件过滤，只处理相关事件

### 内存管理
- 使用`calloc`初始化结构体
- 检查函数返回值，确保内存分配成功
- 在释放时确保所有子资源都被正确释放

## 🐛 调试技巧

### 启用调试输出
```c
printf("[DEBUG] 按钮点击: %d\n", button->type);
printf("[DEBUG] 搜索文本: %s\n", toolbar->search_text);
printf("[DEBUG] 历史记录: %d/%d\n", toolbar->history_index, toolbar->history_count);
```

### 常见问题
1. **按钮不响应**: 检查按钮是否启用，事件是否正确传递
2. **搜索不工作**: 确认搜索状态是否正确激活
3. **历史记录丢失**: 检查内存分配和释放逻辑

## 🔗 集成示例

### 与主窗口集成
```c
// 获取主窗口引用
MainWindow *main_window = (MainWindow*)toolbar->app->user_data;

// 访问文件列表视图
if (main_window && main_window->file_list_view) {
    // 执行文件操作
    file_list_view_refresh(main_window->file_list_view);
}
```

### 与文件系统集成
```c
// 通知目录变更
void on_directory_changed(const char *path) {
    toolbar_notify_directory_changed(toolbar, path);
}
```

---

**提示**: 更多详细信息请参考完整的 `toolbar_usage_guide.md` 文档。 