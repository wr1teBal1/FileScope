## Sidebar 模块使用指南

### 概述
Sidebar 模块是 FileScope 文件管理器的核心 UI 组件之一，负责显示快速访问项、驱动器列表并处理导航交互。模块采用模块化设计，支持主题自定义、事件处理、滚动显示和动态驱动器刷新。

- **文件位置**: `app/ui/sidebar.c`
- **头文件**: `include/sidebar.h`
- **关联模块**: `renderer.h`、`file_system.h`、`path_resolver.h`、`toolbar.h`

### 依赖环境
- SDL3（渲染与事件）
- SDL3_image（PNG 图标加载）
- SDL3_ttf（文本渲染）

请确保工程已正确链接以上依赖库，并在初始化阶段完成 renderer、font 等资源的创建。

### 公共 API
- `Sidebar* sidebar_new(struct Window *app);`
  - 创建并初始化侧边栏。`app` 需提供有效的 `SDL_Renderer*` 与 `TTF_Font*`（通常存放在 `Window`/应用上下文中）。
- `void sidebar_free(Sidebar *sidebar);`
  - 释放侧边栏及其内部资源（名称、路径、图标纹理等）。
- `bool sidebar_handle_event(Sidebar *sidebar, SDL_Event *event);`
  - 处理鼠标移动、点击与滚轮事件；返回值指示 UI 是否需要重绘（如悬停/选中/滚动发生变化）。
- `void sidebar_draw(Sidebar *sidebar);`
  - 按当前状态绘制侧边栏背景、分隔线、项目图标与文本。
- `void sidebar_set_item_selected_callback(Sidebar *sidebar, SidebarItemSelectedCallback callback);`
  - 设置项目选中回调；当用户点击有效项目时触发，回调参数为对应项目路径。
- `void sidebar_refresh_drives(Sidebar *sidebar);`
  - 重新枚举系统驱动器并更新驱动器列表项目（会释放旧驱动器项的资源并重排布局）。

提示：内部使用（`static`）的函数如 `sidebar_add_item`、`sidebar_add_quick_access_items` 等为模块私有，不建议从模块外部调用。

### 数据结构（摘要）
- `Sidebar`：包含渲染区域、项目数组、选中/悬停索引、滚动偏移、颜色主题与回调等。
- `SidebarItem`：包含类型（快速访问/驱动器/分隔线）、名称、路径、图标纹理、绘制矩形与状态。
- `DriveInfo`：用于驱动器枚举（盘符、标签、文件系统类型、容量等）。
- `SpecialFolder`：特殊文件夹（桌面、文档、下载、图片、音乐、视频）。

### 快速上手
1) 创建与注册回调
```c
// 假设已有有效的 Window* app，且包含 renderer 与 font
Sidebar *sidebar = sidebar_new(app);
if (!sidebar) { /* 错误处理 */ }

// 选中回调：将路径交给你的文件视图/导航器
static void on_sidebar_item_selected(Sidebar *sb, const char *path) {
    if (!sb || !path) return;

    char actual_path[512];
    // 处理驱动器路径（如 "E:" → "E:\\"）
    if (strlen(path) == 2 && path[1] == ':') {
        snprintf(actual_path, sizeof(actual_path), "%s\\", path);
    } else {
        strncpy(actual_path, path, sizeof(actual_path) - 1);
        actual_path[sizeof(actual_path) - 1] = '\0';
    }

    // 将 actual_path 交给你的文件列表视图进行加载
    // file_list_view_load_directory(view, actual_path);
}

sidebar_set_item_selected_callback(sidebar, on_sidebar_item_selected);
```

2) 事件转发
```c
SDL_Event e;
while (SDL_PollEvent(&e)) {
    // 将鼠标相关事件交给侧边栏处理
    bool ui_changed = sidebar_handle_event(sidebar, &e);
    if (ui_changed) {
        // 触发重绘（依据你的渲染架构）
    }

    // ... 你的其他事件处理 ...
}
```

3) 绘制调用
```c
// 在你的渲染帧中调用
sidebar_draw(sidebar);
```

4) 动态刷新驱动器
```c
// 当系统驱动器发生变化或你需要主动刷新时调用
sidebar_refresh_drives(sidebar);
```

5) 释放资源
```c
sidebar_free(sidebar);
sidebar = NULL;
```

### 功能要点
- **快速访问项**：模块启动时自动添加（桌面、文档、下载、图片、音乐、视频）。
- **驱动器列表**：使用 `get_drives` 自动枚举；显示盘符与可用标签（如 `E: (Data)`）。
- **路径解析**：优先通过 `path_resolver` 解析虚拟路径，失败时回退到 `get_special_folder_path`。
- **事件处理**：
  - 移动：更新悬停项索引并请求重绘
  - 点击：更新选中项索引并触发选中回调
  - 滚轮：按步长（20 像素/格）滚动，并进行边界裁剪
- **渲染细节（SDL3）**：
  - 使用 `SDL_FRect` 与 `SDL_RenderFillRect` 绘制背景
  - 使用 `SDL_RenderLine` 绘制分隔线与边框
  - 文本：`TTF_RenderText_Blended` → `SDL_CreateTextureFromSurface` → `SDL_RenderTexture`
  - 图标：优先 `IMG_Load`；失败则创建 `SDL_Surface` 并上色为占位图，再转纹理
- **主题自定义**：通过修改 `Sidebar` 的 `bg_color`、`text_color`、`hover_color`、`selected_color`、`separator_color` 实现不同主题（浅色/深色）。

### 颜色主题示例
```c
// 深色主题示例（在 sidebar_new 后调整）
sidebar->bg_color        = (SDL_Color){40, 40, 40, 255};
sidebar->text_color      = (SDL_Color){255, 255, 255, 255};
sidebar->hover_color     = (SDL_Color){60, 60, 60, 255};
sidebar->selected_color  = (SDL_Color){80, 80, 80, 255};
sidebar->separator_color = (SDL_Color){100, 100, 100, 255};
```

### 使用建议
- 在主渲染循环中保证 `sidebar_draw` 在 `renderer` 有效的前提下调用。
- 将 `sidebar_handle_event` 的返回值用于判断是否需要重绘，避免无效重绘。
- 当窗口大小变化时，确保外部布局逻辑更新 `Sidebar.rect` 并按需触发重绘。
- 若你扩展模块以添加自定义项目类型，请在内部新增枚举分支、图标加载与绘制逻辑，并避免直接调用模块私有（`static`）函数。

### 常见问题排查
- 看不到文字：确认 `sidebar->app->font` 有效且 SDL3_ttf 初始化成功。
- 图标缺失：确认 `assets/icons/` 路径下的 PNG 存在；或检查 `SDL3_image` 初始化；模块会回退为彩色占位图。
- 滚动无效：检查 `SDL_EVENT_MOUSE_WHEEL` 是否被正确转发，并确认 `item_count * 行高 > 视口高度`。
- 点击无反应：确认点击坐标确实落在侧边栏 `rect` 范围内，并检查回调是否已设置。

### 版本与实现对齐说明
- 本模块实现基于 SDL3 API：请使用 `SDL_RenderTexture`、`SDL_CreateSurface`/`SDL_DestroySurface` 等 API。
- 选中/悬停态通过 `selected_index / hover_index` 判定，而非自定义的 `SIDEBAR_STATE_SELECTED`。
- 模块内部已在 `sidebar_refresh_drives` 末尾重排项目位置；无需额外手动更新。

如需进一步扩展（例如新增项目类型、网络驱动器等），建议在 `sidebar.c` 内部完成：
- 扩展 `SidebarItemType`
- 在 `load_icon` 中添加图标加载分支
- 在绘制分支中处理新类型的渲染

以上即为 Sidebar 模块的对齐版使用指南，可直接应用于 FileScope 项目。 

### 关键代码片段与可复制修复（Paste-to-Fix）
以下片段与当前实现对齐，若你不慎删除了对应函数或其部分逻辑，可直接复制回 `app/ui/sidebar.c` 对应位置进行恢复。

#### 1) sidebar_draw
```c
void sidebar_draw(Sidebar *sidebar) {
    if (!sidebar || !sidebar->app || !sidebar->app->renderer) {
        return;
    }

    SDL_Renderer *renderer = sidebar->app->renderer;

    // 背景
    SDL_SetRenderDrawColor(renderer,
                           sidebar->bg_color.r,
                           sidebar->bg_color.g,
                           sidebar->bg_color.b,
                           sidebar->bg_color.a);
    SDL_FRect frect = {
        (float)sidebar->rect.x,
        (float)sidebar->rect.y,
        (float)sidebar->rect.w,
        (float)sidebar->rect.h
    };
    SDL_RenderFillRect(renderer, &frect);

    // 右边框
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    SDL_RenderLine(renderer,
                   (float)(sidebar->rect.x + sidebar->rect.w - 1),
                   (float)sidebar->rect.y,
                   (float)(sidebar->rect.x + sidebar->rect.w - 1),
                   (float)(sidebar->rect.y + sidebar->rect.h));

    // 列表项
    for (int i = 0; i < sidebar->item_count; i++) {
        sidebar_draw_item(sidebar, i);
    }
}
```

#### 2) sidebar_draw_item
```c
static void sidebar_draw_item(Sidebar *sidebar, int index) {
    if (!sidebar || index < 0 || index >= sidebar->item_count) {
        return;
    }

    SDL_Renderer *renderer = sidebar->app->renderer;
    SidebarItem *item = &sidebar->items[index];

    // 考虑滚动
    int y = item->rect.y - sidebar->scroll_offset;

    // 不可见裁剪
    if (y + item->rect.h < sidebar->rect.y || y >= sidebar->rect.y + sidebar->rect.h) {
        return;
    }

    SDL_Rect draw_rect = {
        item->rect.x,
        y,
        item->rect.w,
        item->rect.h
    };

    if (item->type == SIDEBAR_ITEM_SEPARATOR) {
        SDL_SetRenderDrawColor(renderer,
                               sidebar->separator_color.r,
                               sidebar->separator_color.g,
                               sidebar->separator_color.b,
                               sidebar->separator_color.a);
        SDL_RenderLine(renderer,
                       (float)(draw_rect.x + SIDEBAR_ITEM_PADDING),
                       (float)draw_rect.y,
                       (float)(draw_rect.x + draw_rect.w - SIDEBAR_ITEM_PADDING * 2),
                       (float)draw_rect.y);
        return;
    }

    // 背景（按选中/悬停）
    SDL_Color bg_color = sidebar->bg_color;
    if (index == sidebar->selected_index) {
        bg_color = sidebar->selected_color;
    } else if (index == sidebar->hover_index) {
        bg_color = sidebar->hover_color;
    }

    SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, bg_color.a);
    SDL_FRect fdraw_rect = {
        (float)draw_rect.x,
        (float)draw_rect.y,
        (float)draw_rect.w,
        (float)draw_rect.h
    };
    SDL_RenderFillRect(renderer, &fdraw_rect);

    // 图标
    if (item->icon) {
        SDL_FRect icon_rect = {
            (float)(draw_rect.x + SIDEBAR_ITEM_PADDING),
            (float)(draw_rect.y + (draw_rect.h - 16) / 2),
            16.0f,
            16.0f
        };
        SDL_RenderTexture(renderer, item->icon, NULL, &icon_rect);
    }

    // 文本（SDL_ttf）
    if (item->name) {
        SDL_Surface *text_surface = TTF_RenderText_Blended(
            sidebar->app->font,
            item->name,
            strlen(item->name),
            sidebar->text_color
        );
        if (text_surface) {
            SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
            if (text_texture) {
                SDL_FRect text_rect;
                text_rect.x = (float)(draw_rect.x + SIDEBAR_ITEM_PADDING + (item->icon ? 20 : 0));
                text_rect.y = (float)(draw_rect.y + (draw_rect.h - text_surface->h) / 2);
                text_rect.w = (float)text_surface->w;
                text_rect.h = (float)text_surface->h;
                SDL_RenderTexture(renderer, text_texture, NULL, &text_rect);
                SDL_DestroyTexture(text_texture);
            }
            SDL_DestroySurface(text_surface);
        }
    }
}
```

#### 3) sidebar_handle_event
```c
bool sidebar_handle_event(Sidebar *sidebar, SDL_Event *event) {
    if (!sidebar || !event) return false;

    switch (event->type) {
        case SDL_EVENT_MOUSE_MOTION: {
            int x = event->motion.x;
            int y = event->motion.y;
            if (x >= sidebar->rect.x && x < sidebar->rect.x + sidebar->rect.w &&
                y >= sidebar->rect.y && y < sidebar->rect.y + sidebar->rect.h) {
                int item_index = sidebar_get_item_at(sidebar, x, y);
                if (item_index != sidebar->hover_index) {
                    sidebar->hover_index = item_index;
                    return true;
                }
            } else {
                sidebar->hover_index = -1;
                return true;
            }
            break;
        }
        case SDL_EVENT_MOUSE_BUTTON_DOWN: {
            if (event->button.button == SDL_BUTTON_LEFT) {
                int x = event->button.x;
                int y = event->button.y;
                if (x >= sidebar->rect.x && x < sidebar->rect.x + sidebar->rect.w &&
                    y >= sidebar->rect.y && y < sidebar->rect.y + sidebar->rect.h) {
                    int item_index = sidebar_get_item_at(sidebar, x, y);
                    if (item_index >= 0 && item_index < sidebar->item_count) {
                        sidebar->selected_index = item_index;
                        if (sidebar->on_item_selected && sidebar->items[item_index].path) {
                            sidebar->on_item_selected(sidebar, sidebar->items[item_index].path);
                        }
                        return true;
                    }
                }
            }
            break;
        }
        case SDL_EVENT_MOUSE_WHEEL: {
            if (event->wheel.y != 0) {
                sidebar->scroll_offset += event->wheel.y * 20;
                int max_scroll = (sidebar->item_count * SIDEBAR_ITEM_HEIGHT) - sidebar->rect.h;
                if (max_scroll < 0) max_scroll = 0;
                if (sidebar->scroll_offset < 0) sidebar->scroll_offset = 0;
                if (sidebar->scroll_offset > max_scroll) sidebar->scroll_offset = max_scroll;
                return true;
            }
            break;
        }
    }
    return false;
}
```

#### 4) get_special_folder_path_wrapper
```c
static bool get_special_folder_path_wrapper(SpecialFolder folder, char *path, size_t path_size) {
    if (!path || path_size == 0) {
        return false;
    }

    const char *virtual_path = NULL;
    switch (folder) {
        case FOLDER_DESKTOP:   virtual_path = "桌面"; break;
        case FOLDER_DOCUMENTS: virtual_path = "我的文档"; break;
        case FOLDER_DOWNLOADS: virtual_path = "下载"; break;
        case FOLDER_PICTURES:  virtual_path = "图片"; break;
        case FOLDER_MUSIC:     virtual_path = "音乐"; break;
        case FOLDER_VIDEOS:    virtual_path = "视频"; break;
        default: return false;
    }

    PathInfo *info = path_resolve(virtual_path, NULL);
    if (!info || !info->is_valid) {
        if (info) path_info_free(info);
        return get_special_folder_path(folder, path, path_size);
    }

    strncpy(path, info->absolute_path, path_size - 1);
    path[path_size - 1] = '\0';
    path_info_free(info);
    return true;
}
```

#### 5) load_icon（包含 SDL3 正确用法与回退）
```c
static SDL_Texture* load_icon(Sidebar *sidebar, SidebarItemType type) {
    if (!sidebar || !sidebar->app || !sidebar->app->renderer) {
        return NULL;
    }

    SDL_Renderer *renderer = sidebar->app->renderer;
    SDL_Texture *texture = NULL;

    const char *icon_path = NULL;
    switch (type) {
        case SIDEBAR_ITEM_QUICK_ACCESS: icon_path = "assets/icons/folder.png"; break;
        case SIDEBAR_ITEM_DRIVE:        icon_path = "assets/icons/drive.png";  break;
        default: return NULL;
    }

    SDL_Surface *surface = IMG_Load(icon_path);
    if (!surface) {
        surface = SDL_CreateSurface(16, 16, SDL_PIXELFORMAT_RGBA32);
        if (!surface) return NULL;

        const SDL_PixelFormatDetails *fmt = SDL_GetPixelFormatDetails(surface->format);
        Uint32 color = 0;
        if (type == SIDEBAR_ITEM_QUICK_ACCESS) {
            color = SDL_MapRGBA(fmt, 255, 200, 0, 255);   // 黄色
        } else {
            color = SDL_MapRGBA(fmt, 100, 150, 200, 255); // 蓝色
        }
        SDL_FillSurfaceRect(surface, NULL, color);
    }

    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);
    return texture;
}
```

#### 6) sidebar_refresh_drives（含位置重排）
```c
void sidebar_refresh_drives(Sidebar *sidebar) {
    if (!sidebar) return;

    // 找第一个分隔线
    int separator_index = -1;
    for (int i = 0; i < sidebar->item_count; i++) {
        if (sidebar->items[i].type == SIDEBAR_ITEM_SEPARATOR) {
            separator_index = i; break;
        }
    }
    if (separator_index < 0) {
        sidebar_add_separator(sidebar);
        separator_index = sidebar->item_count - 1;
    }

    // 移除旧驱动器项
    int i = separator_index + 1;
    while (i < sidebar->item_count) {
        if (sidebar->items[i].type == SIDEBAR_ITEM_DRIVE) {
            if (sidebar->items[i].name) free(sidebar->items[i].name);
            if (sidebar->items[i].path) free(sidebar->items[i].path);
            if (sidebar->items[i].icon) SDL_DestroyTexture(sidebar->items[i].icon);
            for (int j = i; j < sidebar->item_count - 1; j++) {
                sidebar->items[j] = sidebar->items[j + 1];
            }
            sidebar->item_count--;
        } else {
            i++;
        }
    }

    // 重新枚举驱动器
    sidebar_add_drives(sidebar);

    // 重排位置
    for (int k = 0; k < sidebar->item_count; k++) {
        sidebar->items[k].rect.x = sidebar->rect.x;
        sidebar->items[k].rect.y = sidebar->rect.y + SIDEBAR_ITEM_PADDING + k * SIDEBAR_ITEM_HEIGHT;
    }
}
```

#### 7) sidebar_get_item_at
```c
static int sidebar_get_item_at(Sidebar *sidebar, int x, int y) {
    if (!sidebar) return -1;
    y += sidebar->scroll_offset;
    for (int i = 0; i < sidebar->item_count; i++) {
        SidebarItem *item = &sidebar->items[i];
        if (x >= item->rect.x && x < item->rect.x + item->rect.w &&
            y >= item->rect.y && y < item->rect.y + item->rect.h) {
            return i;
        }
    }
    return -1;
}
```

---

### 丰富的代码解读（What & Why）
- **可见性裁剪（sidebar_draw_item）**：
  - 在滚动视图中，仅绘制可见区域可以显著减少渲染调用，`if (y + h < top || y >= bottom) return;` 是最直接的裁剪方式。
- **状态驱动的背景色**：
  - 通过 `selected_index/hover_index` 与循环索引对比，避免在数据结构中复制状态，简化状态一致性维护。
- **SDL3 FRect 与 RenderTexture**：
  - SDL3 推荐使用浮点矩形与 `SDL_RenderTexture`，与 SDL2 的 `SDL_RenderCopy` 区分清晰；文档中的代码已统一至 SDL3。
- **文本渲染链路**：
  - `TTF_RenderText_Blended` 直接生成 `SDL_Surface`，再转 `SDL_Texture`。释放顺序与所有权：`Surface` → `Texture`，务必分别销毁，防止泄漏。
- **图标加载回退**：
  - PNG 缺失时退化为纯色占位，颜色通过 `SDL_MapRGBA(fmt, r,g,b,a)` 计算像素值（注意 SDL3 无需传 `NULL`）。
- **驱动器刷新策略**：
  - 以第一条分隔线为边界，分隔线之下全部是驱动器项；刷新时先“清空段”，再重新枚举，可避免与“快速访问”混淆。
- **滚轮边界裁剪**：
  - 通过 `max_scroll = item_count*行高 - 视口高` 计算最大滚动偏移；小于 0 表明无需滚动，统一 clamp 可避免负值带来的坐标错位。

### 常见错误与快速修复
- **症状：文字不显示**
  - 检查 `sidebar->app->font` 是否有效；确认 `TTF_RenderText_Blended` 返回非空；渲染器与纹理创建是否成功。
- **症状：图标全是方块**
  - 首先确认 `assets/icons/*.png` 存在与路径正确；若依然为方块，这是回退逻辑生效，说明 `IMG_Load` 失败，请检查 `SDL3_image` 初始化或资源路径。
- **症状：编译期错误（找不到 SDL_RenderCopy / SDL_FreeSurface）**
  - 这是 SDL2/SDL3 API 混用：将 `SDL_RenderCopy` 替换为 `SDL_RenderTexture`，`SDL_FreeSurface` 替换为 `SDL_DestroySurface`，`SDL_CreateRGBSurface` 替换为 `SDL_CreateSurface`。
- **症状：颜色异常或崩溃**
  - 检查 `SDL_MapRGBA` 参数顺序与类型，SDL3 签名为 `(const SDL_PixelFormatDetails*, Uint8 r, Uint8 g, Uint8 b, Uint8 a)`，勿传递多余 `NULL` 参数。

### 自检与恢复流程（删改后如何快速复原）
1. 定位缺失函数：在 `app/ui/sidebar.c` 搜索函数名（如 `sidebar_draw_item`）。
2. 从“关键代码片段与可复制修复”中复制对应实现，粘贴回原位置。
3. 对照“丰富的代码解读”逐项确认：
   - 渲染 API 是否统一为 SDL3（`SDL_RenderTexture`、`SDL_CreateSurface`、`SDL_DestroySurface`）。
   - 滚动裁剪、状态颜色与坐标换算是否完整。
4. 重新编译并运行；若仍有问题，参考“常见错误与快速修复”进行排查。

以上附加章节确保即便误删某段实现，也能凭本指南中的“粘贴即用”片段与解读快速恢复功能与行为一致性。 