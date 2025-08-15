# Sidebar 模块快速参考

## 🚀 快速开始

```c
// 创建侧边栏
Sidebar *sidebar = sidebar_new(window);

// 设置选中回调
sidebar_set_item_selected_callback(sidebar, on_item_selected);

// 在主循环中处理事件和绘制
while (running) {
    while (SDL_PollEvent(&event)) {
        sidebar_handle_event(sidebar, &event);
    }
    sidebar_draw(sidebar);
}

// 清理资源
sidebar_free(sidebar);
```

## 📋 核心API

| 函数 | 功能 | 参数 | 返回值 |
|------|------|------|--------|
| `sidebar_new()` | 创建侧边栏 | `Window *app` | `Sidebar*` |
| `sidebar_free()` | 释放资源 | `Sidebar *sidebar` | `void` |
| `sidebar_handle_event()` | 处理事件 | `Sidebar*, SDL_Event*` | `bool` |
| `sidebar_draw()` | 绘制界面 | `Sidebar *sidebar` | `void` |
| `sidebar_refresh_drives()` | 刷新驱动器 | `Sidebar *sidebar` | `void` |

## 🎨 自定义样式

```c
// 浅色主题
sidebar->bg_color = (SDL_Color){255, 255, 255, 255};
sidebar->text_color = (SDL_Color){0, 0, 0, 255};
sidebar->hover_color = (SDL_Color){240, 240, 240, 255};
sidebar->selected_color = (SDL_Color){220, 220, 220, 255};

// 深色主题
sidebar->bg_color = (SDL_Color){40, 40, 40, 255};
sidebar->text_color = (SDL_Color){255, 255, 255, 255};
sidebar->hover_color = (SDL_Color){60, 60, 60, 255};
sidebar->selected_color = (SDL_Color){80, 80, 80, 255};
```

## 🔧 配置常量

```c
#define SIDEBAR_WIDTH 200              // 侧边栏宽度
#define SIDEBAR_ITEM_HEIGHT 30         // 项目高度
#define SIDEBAR_ITEM_PADDING 5         // 项目内边距
#define MAX_SIDEBAR_ITEMS 20           // 最大项目数
```

## 📁 项目类型

```c
typedef enum {
    SIDEBAR_ITEM_QUICK_ACCESS,  // 快速访问项
    SIDEBAR_ITEM_DRIVE,         // 驱动器项
    SIDEBAR_ITEM_SEPARATOR      // 分隔线
} SidebarItemType;
```

## 🖱️ 支持的事件

- **鼠标移动**: 更新悬停状态
- **左键点击**: 选择项目
- **滚轮滚动**: 垂直滚动

## 🔄 回调函数

```c
typedef void (*SidebarItemSelectedCallback)(Sidebar *sidebar, const char *path);

void on_item_selected(Sidebar *sidebar, const char *path) {
    printf("选中: %s\n", path);
    // 处理导航逻辑
}
```

## 🚨 常见问题

| 问题 | 检查项 | 解决方案 |
|------|--------|----------|
| 项目不显示 | `item_count`、坐标计算 | 验证项目添加逻辑 |
| 图标加载失败 | 文件路径、SDL_image | 检查资源文件 |
| 事件无响应 | 回调函数设置 | 确认回调已注册 |

## 📚 扩展功能

- ✅ 添加新项目类型
- ✅ 右键菜单支持
- ✅ 拖放功能
- ✅ 搜索功能
- ✅ 分组功能
- ✅ 图标缓存
- ✅ 虚拟化渲染

## 🔍 调试技巧

```c
#ifdef DEBUG_SIDEBAR
#define SIDEBAR_DEBUG(fmt, ...) printf("[SIDEBAR] " fmt "\n", ##__VA_ARGS__)
#else
#define SIDEBAR_DEBUG(fmt, ...)
#endif

// 在关键位置添加调试信息
SIDEBAR_DEBUG("项目数量: %d", sidebar->item_count);
```

## 📖 完整文档

详细使用说明请参考: [sidebar_guide.md](./sidebar_guide.md) 