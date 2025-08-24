# Toolbar 模块使用与扩展指南（可复制修复版）

## 目标与读者
- 本文档面向需要维护/扩展 `Toolbar` 的开发者。
- 提供：
  - 未来修改与新增功能的清晰步骤与约束（稳定扩展）。
  - 对当前实现的详细解读（按职责与代码路径组织）。
  - 若误删实现中的某一段，可直接从文档复制相应“修复片段”粘贴回去（最小可用修复）。

- 相关文件：`app/ui/toolbar.c`、`include/toolbar.h`

## 快速索引
- 架构与职责
- 扩展改造手册（新增按钮/交互的标准流程）
- 代码详解（带行号引用，便于精确定位）
- 粘贴修复片段（误删后的最小可用实现）
- 自检清单与故障定位

---

## 架构与职责
- 视图层（绘制）
  - 绘制工具栏背景、边框、按钮图标、地址栏、搜索框与光标/清除按钮。
- 交互层（事件）
  - 命中检测、按钮按下/抬起；搜索/地址栏的互斥激活；文本输入与按键处理。
- 业务层（状态）
  - 历史记录（路径数组、当前索引）；导航行为（后退/前进/上级/主目录/刷新/视图切换）；
  - 搜索过滤调用与地址栏导航。

- 布局（当前约定）
  - 左侧：后退、前进、刷新、视图；`UP/HOME` 不布局（`enabled=false, rect=0`）。
  - 中部：地址栏（宽度动态 140–320）。
  - 右侧：搜索按钮 + 搜索框（固定宽 220）。

- 常量
```c
#define TOOLBAR_HEIGHT 40
#define BUTTON_PADDING 5
#define BUTTON_SIZE 30
#define BUTTON_SPACING 10
```

---

## 扩展改造手册（标准流程）
- 新增按钮（推荐 5 步）
  1. 在 `ToolbarButtonType` 添加新枚举，文案/tooltip一并确定。
  2. 在 `toolbar_new` 中设置 `rect/tooltip/enabled`；若暂不显示，可先 `enabled=false` 且 `rect=0`。
  3. 在 `draw_toolbar_button` 的 `switch(type)` 中绘制简笔图标（与现有风格一致）。
  4. 在 `execute_button_action` 添加对应行为；如需外部可调用，增加一个对外 API 代理到内部行为。
  5. 在 `toolbar_update_button_states` 集中管理启用条件（便于统一测试）。

- 交互新增（如新输入框）
  - 复用“互斥激活”模式（参考搜索/地址栏），在激活时 `SDL_StartTextInput`，停止时 `SDL_StopTextInput`；
  - 绘制：背景/边框/文本/光标的顺序与现有输入框一致。

- 布局变更
  - 地址栏宽度取中部可用空间并裁剪到 [140,320]；
  - 搜索框宽固定 220，搜索按钮位置在 `toolbar_draw` 内根据搜索框位置同步，不要在初始化时写死。

- 历史策略
  - 新路径写入前截断“前进分支”，容量不够翻倍扩容；
  - 外部导航后务必调用 `toolbar_notify_directory_changed`，保证后退/前进状态及时更新。

---

## 代码详解（带源码定位）

- 数据结构与按钮绘制
```72:152:FileScope/app/ui/toolbar.c
static void draw_toolbar_button(Toolbar *toolbar, ToolbarButton *button) {
    // 根据 enabled/hovered/pressed 切换颜色 → 绘制背景与边框
    // switch(button->type) 绘制图标（箭头、网格、放大镜、房屋、圆形刷新等）
}
```

- 命中检测（按钮）
```294:309:FileScope/app/ui/toolbar.c
static ToolbarButton*  find_button_at_point(Toolbar *toolbar, int x, int y) {
    for (int i = 0; i < toolbar->button_count; i++) {
        ToolbarButton *button = &toolbar->buttons[i];
        if (x >= button->rect.x && x < button->rect.x + button->rect.w &&
            y >= button->rect.y && y < button->rect.y + button->rect.h) {
            return button;
        }
    }
    return NULL;
}
```

- 历史写入与导航
```311:350:FileScope/app/ui/toolbar.c
static void add_to_history(Toolbar *toolbar, const char *path) {
    // 截断前进分支 → 扩容（2x） → strdup(path) → count/index 推进 → 更新后退/前进可用性
}
```
```352:385:FileScope/app/ui/toolbar.c
static void navigate_back(Toolbar *toolbar) {
    // history_index-- → 取路径 → file_list_view_load_directory(...)
}
static void navigate_forward(Toolbar *toolbar) {
    // history_index++ → 取路径 → file_list_view_load_directory(...)
}
```

- 刷新与行为分发
```458:520:FileScope/app/ui/toolbar.c
static void refresh_view(Toolbar *toolbar) { /* 以当前路径重载 */ }
static void execute_button_action(Toolbar *toolbar, ToolbarButton *button) {
    // BACK/FORWARD/UP/HOME/REFRESH/SEARCH/VIEW 的行为切换
}
```

- 生命周期与初始化（布局逻辑）
```522:661:FileScope/app/ui/toolbar.c
Toolbar* toolbar_new(struct Window *app) {
    // 设置 rect → 左侧按钮布局（UP/HOME禁用且rect=0）→ 右侧搜索按钮+框 → 中部地址栏
    // 初始化历史数组与搜索/地址栏状态
}
```

- 事件分发（互斥激活 + 按钮按压）
```683:868:FileScope/app/ui/toolbar.c
bool toolbar_handle_event(Toolbar *toolbar, SDL_Event *event) {
    // 鼠标移动：hover 切换；
    // 鼠标按下：命中地址栏/搜索框/按钮；地址栏与搜索互斥；搜索框内处理清除按钮；
    // 鼠标抬起：若仍在按钮上则执行 execute_button_action；
    // 键盘与文本：将输入路由到 search 或 address。
}
```

- 绘制流水线（背景→按钮→地址栏→搜索框）
```879:1074:FileScope/app/ui/toolbar.c
void toolbar_draw(Toolbar *toolbar) {
    // 背景/边框 → 逐个按钮 → 地址栏（文本/光标/占位）→ 搜索框（文本/光标/清除按钮）
    // 搜索按钮位置在此与搜索框联动同步
}
```

- 搜索/地址栏交互
```1076:1209:FileScope/app/ui/toolbar.c
// 搜索：start/stop/handle_text/handle_key/search（实时过滤与Ctrl+L清空）
```
```1210:1285:FileScope/app/ui/toolbar.c
// 地址栏：start/stop/handle_text/handle_key/navigate（回车导航并入栈）
```

---

## 粘贴修复片段（误删后的最小可用实现）
- 适用场景：实现中某段被误删/破坏，优先用下列片段恢复（均与当前代码风格/接口对齐）。

- 历史写入（替换/补回）
```c
static void add_to_history(Toolbar *toolbar, const char *path) {
    if (!toolbar || !path) return;
    for (int i = toolbar->history_index + 1; i < toolbar->history_count; i++) {
        free(toolbar->history[i]);
        toolbar->history[i] = NULL;
    }
    toolbar->history_count = toolbar->history_index + 1;
    if (toolbar->history_count >= toolbar->history_capacity) {
        int new_capacity = toolbar->history_capacity * 2;
        char **new_history = (char**)realloc(toolbar->history, new_capacity * sizeof(char*));
        if (!new_history) return;
        toolbar->history = new_history;
        toolbar->history_capacity = new_capacity;
    }
    toolbar->history[toolbar->history_count] = strdup(path);
    if (!toolbar->history[toolbar->history_count]) return;
    toolbar->history_count++;
    toolbar->history_index = toolbar->history_count - 1;
    toolbar->buttons[BUTTON_BACK].enabled = (toolbar->history_index > 0);
    toolbar->buttons[BUTTON_FORWARD].enabled = (toolbar->history_index < toolbar->history_count - 1);
}
```

- 导航（后退/前进）
```c
static void navigate_back(Toolbar *toolbar) {
    if (!toolbar || toolbar->history_index <= 0 || !toolbar->app || !toolbar->app->user_data) return;
    toolbar->history_index--;
    const char *path = toolbar->history[toolbar->history_index];
    MainWindow *main_window = (MainWindow*)toolbar->app->user_data;
    file_list_view_load_directory(main_window->file_list_view, path);
    toolbar->buttons[BUTTON_BACK].enabled = (toolbar->history_index > 0);
    toolbar->buttons[BUTTON_FORWARD].enabled = (toolbar->history_index < toolbar->history_count - 1);
}

static void navigate_forward(Toolbar *toolbar) {
    if (!toolbar || toolbar->history_index >= toolbar->history_count - 1 || !toolbar->app || !toolbar->app->user_data) return;
    toolbar->history_index++;
    const char *path = toolbar->history[toolbar->history_index];
    MainWindow *main_window = (MainWindow*)toolbar->app->user_data;
    file_list_view_load_directory(main_window->file_list_view, path);
    toolbar->buttons[BUTTON_BACK].enabled = (toolbar->history_index > 0);
    toolbar->buttons[BUTTON_FORWARD].enabled = (toolbar->history_index < toolbar->history_count - 1);
}
```

- 刷新与行为分发（最简版）
```c
static void refresh_view(Toolbar *toolbar) {
    if (!toolbar || !toolbar->app || !toolbar->app->user_data) return;
    MainWindow *mw = (MainWindow*)toolbar->app->user_data;
    if (!mw || !mw->file_list_view || !mw->file_list_view->current_path) return;
    file_list_view_load_directory(mw->file_list_view, mw->file_list_view->current_path);
}

static void execute_button_action(Toolbar *toolbar, ToolbarButton *button) {
    if (!toolbar || !button || !button->enabled) return;
    switch (button->type) {
        case BUTTON_BACK:    navigate_back(toolbar); break;
        case BUTTON_FORWARD: navigate_forward(toolbar); break;
        case BUTTON_UP:      /* 可选 */ break;
        case BUTTON_HOME:    /* 可选 */ break;
        case BUTTON_REFRESH: refresh_view(toolbar); break;
        case BUTTON_SEARCH:  toolbar_search_start(toolbar); break;
        case BUTTON_VIEW: {
            MainWindow *mw = (MainWindow*)toolbar->app->user_data;
            if (mw && mw->file_list_view) file_list_view_toggle_mode(mw->file_list_view);
        } break;
        default: break;
    }
}
```

- 绘制主流程（骨架）
```c
void toolbar_draw(Toolbar *toolbar) {
    if (!toolbar || !toolbar->app || !toolbar->app->renderer) return;
    SDL_Renderer *renderer = toolbar->app->renderer;
    // 背景与边框
    SDL_SetRenderDrawColor(renderer, 240,240,240,255);
    SDL_FRect t = {(float)toolbar->rect.x,(float)toolbar->rect.y,(float)toolbar->rect.w,(float)toolbar->rect.h};
    SDL_RenderFillRect(renderer, &t);
    SDL_SetRenderDrawColor(renderer, 100,100,100,255);
    SDL_RenderRect(renderer, &t);
    // 按钮
    for (int i = 0; i < toolbar->button_count; i++) draw_toolbar_button(toolbar, &toolbar->buttons[i]);
    // 计算区域：地址栏（中）+ 搜索（右）
    int search_w=220, sh=BUTTON_SIZE;
    int sx = toolbar->rect.x + toolbar->rect.w - search_w - BUTTON_PADDING;
    int sy = toolbar->rect.y + (toolbar->rect.h - sh)/2;
    toolbar->buttons[BUTTON_SEARCH].rect.x = sx - BUTTON_SPACING - BUTTON_SIZE;
    toolbar->buttons[BUTTON_SEARCH].rect.y = sy;
    toolbar->buttons[BUTTON_SEARCH].rect.w = BUTTON_SIZE;
    toolbar->buttons[BUTTON_SEARCH].rect.h = BUTTON_SIZE;
    int left_group_right = toolbar->buttons[BUTTON_VIEW].rect.x + toolbar->buttons[BUTTON_VIEW].rect.w;
    int addr_left = left_group_right + BUTTON_SPACING;
    int addr_right = toolbar->buttons[BUTTON_SEARCH].rect.x - BUTTON_SPACING;
    int addr_w = addr_right - addr_left; if (addr_w > 320) addr_w=320; if (addr_w < 140) addr_w=140;
    int addr_h = BUTTON_SIZE;
    int addr_x = addr_left;
    int addr_y = toolbar->rect.y + (toolbar->rect.h - addr_h)/2;
    // 地址栏背景/边框/文本/光标
    SDL_FRect address_box = { 
        (float)address_box_x, (float)address_box_y, 
        (float)address_box_w, (float)address_box_h 
    };
    
    // 地址栏背景与边框颜色选择
    SDL_Color addr_bg = toolbar->address_bar_active ? 
        (SDL_Color){255, 255, 240, 255} : (SDL_Color){255, 255, 255, 255};
    SDL_Color addr_border = toolbar->address_bar_active ? 
        (SDL_Color){0, 120, 215, 255} : (SDL_Color){100, 100, 100, 255};
    
    // 绘制地址栏背景
    SDL_SetRenderDrawColor(renderer, addr_bg.r, addr_bg.g, addr_bg.b, addr_bg.a);
    SDL_RenderFillRect(renderer, &address_box);
    
    // 绘制地址栏边框
    SDL_SetRenderDrawColor(renderer, addr_border.r, addr_border.g, addr_border.b, addr_border.a);
    SDL_RenderRect(renderer, &address_box);
    
    // 地址栏文本渲染
    SDL_Color addr_text_color = (SDL_Color){30, 30, 30, 255};
    const char *addr_text_to_show = NULL;
    
    // 确定显示文本：优先显示用户输入，否则显示当前路径
    if (toolbar->address_bar_active || toolbar->address_bar_text[0] != '\0') {
        addr_text_to_show = toolbar->address_bar_text;
    } else {
        if (toolbar->app && toolbar->app->user_data) {
            MainWindow *mw = (MainWindow*)toolbar->app->user_data;
            if (mw && mw->file_list_view && mw->file_list_view->current_path) {
                addr_text_to_show = mw->file_list_view->current_path;
            }
        }
    }
    
    // 渲染地址栏文本
    if (addr_text_to_show && toolbar->app->font) {
        size_t addr_len = strlen(addr_text_to_show);
        SDL_Surface *addr_surface = TTF_RenderText_Blended(toolbar->app->font, addr_text_to_show, addr_len, addr_text_color);
        if (addr_surface) {
            SDL_Texture *addr_tex = SDL_CreateTextureFromSurface(renderer, addr_surface);
            if (addr_tex) {
                int text_x = address_box_x + 8;
                SDL_FRect dst = { 
                    (float)text_x, 
                    (float)(address_box_y + (address_box_h - addr_surface->h) / 2),
                    (float)addr_surface->w, 
                    (float)addr_surface->h 
                };
                // 裁剪显示，避免超出
                if (dst.w > address_box.w - 16) dst.w = address_box.w - 16;
                SDL_RenderTexture(renderer, addr_tex, NULL, &dst);
                SDL_DestroyTexture(addr_tex);
            }
            SDL_DestroySurface(addr_surface);
        }
    }
    
    // 绘制地址栏光标
    if (toolbar->address_bar_active) {
        int caret_x = address_box_x + 8;
        if (toolbar->address_bar_text[0] != '\0' && toolbar->app->font) {
            char temp_addr[1024];
            int cp = toolbar->address_bar_cursor_pos;
            if (cp < 0) cp = 0;
            if (cp > (int)strlen(toolbar->address_bar_text)) cp = (int)strlen(toolbar->address_bar_text);
            strncpy(temp_addr, toolbar->address_bar_text, cp);
            temp_addr[cp] = '\0';
            SDL_Surface *pre_surface = TTF_RenderText_Blended(toolbar->app->font, temp_addr, strlen(temp_addr), addr_text_color);
            if (pre_surface) {
                caret_x += pre_surface->w;
                SDL_DestroySurface(pre_surface);
            }
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderLine(renderer, caret_x, address_box_y + 4, caret_x, address_box_y + address_box_h - 4);
    }
    
    // 搜索框背景/边框/文本/光标/清除按钮
    SDL_FRect search_box = {
        (float)search_box_x, (float)search_box_y, 
        (float)search_box_w, (float)search_box_h
    };
    
    // 搜索框背景与边框颜色选择
    SDL_Color search_bg_color = toolbar->search_active ? 
        (SDL_Color){255, 255, 240, 255} : (SDL_Color){255, 255, 255, 255};
    SDL_Color search_border_color = toolbar->search_active ? 
        (SDL_Color){0, 120, 215, 255} : (SDL_Color){100, 100, 100, 255};
    
    // 绘制搜索框背景
    SDL_SetRenderDrawColor(renderer, search_bg_color.r, search_bg_color.g, search_bg_color.b, search_bg_color.a);
    SDL_RenderFillRect(renderer, &search_box);
    
    // 绘制搜索框边框
    SDL_SetRenderDrawColor(renderer, search_border_color.r, search_border_color.g, search_border_color.b, search_border_color.a);
    SDL_RenderRect(renderer, &search_box);
    
    // 搜索框文本渲染
    if (toolbar->search_active || toolbar->search_text[0] != '\0') {
        SDL_Color text_color = {30, 30, 30, 255};
        if (toolbar->app->font && toolbar->search_text[0] != '\0') {
            size_t text_len = strlen(toolbar->search_text);
            SDL_Surface *text_surface = TTF_RenderText_Blended(toolbar->app->font, toolbar->search_text, text_len, text_color);
            if (text_surface) {
                SDL_Texture *text_tex = SDL_CreateTextureFromSurface(renderer, text_surface);
                if (text_tex) {
                    SDL_FRect dst = {
                        (float)(search_box_x + 8), 
                        (float)(search_box_y + (search_box_h - text_surface->h) / 2),
                        (float)text_surface->w, 
                        (float)text_surface->h
                    };
                    SDL_RenderTexture(renderer, text_tex, NULL, &dst);
                    SDL_DestroyTexture(text_tex);
                }
                SDL_DestroySurface(text_surface);
            }
        }
        
        // 绘制搜索框光标
        if (toolbar->search_active) {
            int cursor_x = search_box_x + 8;
            if (toolbar->search_text[0] != '\0' && toolbar->app->font) {
                char temp_text[256];
                strncpy(temp_text, toolbar->search_text, toolbar->search_cursor_pos);
                temp_text[toolbar->search_cursor_pos] = '\0';
                
                SDL_Surface *cursor_surface = TTF_RenderText_Blended(toolbar->app->font, temp_text, strlen(temp_text), text_color);
                if (cursor_surface) {
                    cursor_x += cursor_surface->w;
                    SDL_DestroySurface(cursor_surface);
                }
            }
            
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderLine(renderer, cursor_x, search_box_y + 4, cursor_x, search_box_y + search_box_h - 4);
        }
        
        // 绘制清除搜索按钮（X）
        if (toolbar->search_text[0] != '\0') {
            int clear_button_size = 16;
            int clear_button_x = search_box_x + search_box_w - clear_button_size - 4;
            int clear_button_y = search_box_y + (search_box_h - clear_button_size) / 2;
            
            // 绘制清除按钮背景（圆形）
            SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
            SDL_FRect clear_button = {
                (float)clear_button_x, (float)clear_button_y, 
                (float)clear_button_size, (float)clear_button_size
            };
            SDL_RenderFillRect(renderer, &clear_button);
            
            // 绘制X符号
            SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
            SDL_RenderLine(renderer, 
                          clear_button_x + 4, clear_button_y + 4, 
                          clear_button_x + clear_button_size - 4, clear_button_y + clear_button_size - 4);
            SDL_RenderLine(renderer, 
                          clear_button_x + clear_button_size - 4, clear_button_y + 4, 
                          clear_button_x + 4, clear_button_y + clear_button_size - 4);
        }
    } else {
        // 显示搜索框占位符
        SDL_Color ph_color = {180, 180, 180, 255};
        if (toolbar->app->font) {
            size_t ph_len = strlen("搜索...");
            SDL_Surface *ph_surface = TTF_RenderText_Blended(toolbar->app->font, "搜索...", ph_len, ph_color);
            if (ph_surface) {
                SDL_Texture *ph_tex = SDL_CreateTextureFromSurface(renderer, ph_surface);
                if (ph_tex) {
                    SDL_FRect dst = {
                        (float)(search_box_x + 8), 
                        (float)(search_box_y + (search_box_h - ph_surface->h) / 2),
                        (float)ph_surface->w, 
                        (float)ph_surface->h
                    };
                    SDL_RenderTexture(renderer, ph_tex, NULL, &dst);
                    SDL_DestroyTexture(ph_tex);
                }
                SDL_DestroySurface(ph_surface);
            }
        }
    }
}
```

- 搜索与地址栏（接口骨架）
```c
void toolbar_search_start(Toolbar *toolbar) {
    if (!toolbar) return;
    toolbar->search_active = true;
    toolbar->search_cursor_pos = (int)strlen(toolbar->search_text);
    if (toolbar->app && toolbar->app->window) SDL_StartTextInput(toolbar->app->window);
}
void toolbar_search_stop(Toolbar *toolbar) {
    if (!toolbar) return;
    toolbar->search_active = false;
    if (toolbar->app && toolbar->app->window && !toolbar->address_bar_active) SDL_StopTextInput(toolbar->app->window);
}
void toolbar_search_handle_text(Toolbar *toolbar, const char *text) {
    if (!toolbar || !toolbar->search_active || !text) return;
    size_t len = strlen(toolbar->search_text), tlen = strlen(text);
    if (len + tlen < sizeof(toolbar->search_text)-1) {
        memmove(&toolbar->search_text[toolbar->search_cursor_pos + tlen], &toolbar->search_text[toolbar->search_cursor_pos], len - toolbar->search_cursor_pos + 1);
        memcpy(&toolbar->search_text[toolbar->search_cursor_pos], text, tlen);
        toolbar->search_cursor_pos += (int)tlen;
        toolbar_search(toolbar, toolbar->search_text);
    }
}
void toolbar_search_handle_key(Toolbar *toolbar, SDL_Scancode sc) {
    if (!toolbar || !toolbar->search_active) return;
    size_t len = strlen(toolbar->search_text);
    switch (sc) {
        case SDL_SCANCODE_BACKSPACE:
            if (toolbar->search_cursor_pos > 0 && len > 0) {
                memmove(&toolbar->search_text[toolbar->search_cursor_pos-1], &toolbar->search_text[toolbar->search_cursor_pos], len - toolbar->search_cursor_pos + 1);
                toolbar->search_cursor_pos--; toolbar_search(toolbar, toolbar->search_text);
            } break;
        case SDL_SCANCODE_RETURN:
            if (toolbar->search_text[0] != '\0') toolbar_search(toolbar, toolbar->search_text);
            toolbar_search_stop(toolbar); break;
        case SDL_SCANCODE_ESCAPE: toolbar_search_stop(toolbar); break;
        case SDL_SCANCODE_LEFT: if (toolbar->search_cursor_pos > 0) toolbar->search_cursor_pos--; break;
        case SDL_SCANCODE_RIGHT: if (toolbar->search_cursor_pos < (int)len) toolbar->search_cursor_pos++; break;
        case SDL_SCANCODE_L: if (SDL_GetModState() & SDL_KMOD_CTRL) { toolbar->search_text[0]='\0'; toolbar->search_cursor_pos=0; toolbar_search(toolbar, ""); } break;
        default: break;
    }
}
```
```c
void toolbar_address_bar_start(Toolbar *toolbar) {
    if (!toolbar) return;
    toolbar->address_bar_active = true;
    toolbar->address_bar_cursor_pos = (int)strlen(toolbar->address_bar_text);
    if (toolbar->app && toolbar->app->window) SDL_StartTextInput(toolbar->app->window);
}
void toolbar_address_bar_stop(Toolbar *toolbar) {
    if (!toolbar) return;
    toolbar->address_bar_active = false;
    if (toolbar->app && toolbar->app->window && !toolbar->search_active) SDL_StopTextInput(toolbar->app->window);
}
void toolbar_address_bar_handle_text(Toolbar *toolbar, const char *text) {
    if (!toolbar || !toolbar->address_bar_active || !text) return;
    size_t len = strlen(toolbar->address_bar_text), tlen = strlen(text);
    if (len + tlen < sizeof(toolbar->address_bar_text)-1) {
        memmove(&toolbar->address_bar_text[toolbar->address_bar_cursor_pos + tlen], &toolbar->address_bar_text[toolbar->address_bar_cursor_pos], len - toolbar->address_bar_cursor_pos + 1);
        memcpy(&toolbar->address_bar_text[toolbar->address_bar_cursor_pos], text, tlen);
        toolbar->address_bar_cursor_pos += (int)tlen;
    }
}
void toolbar_address_bar_handle_key(Toolbar *toolbar, SDL_Scancode sc) {
    if (!toolbar || !toolbar->address_bar_active) return;
    size_t len = strlen(toolbar->address_bar_text);
    switch (sc) {
        case SDL_SCANCODE_BACKSPACE:
            if (toolbar->address_bar_cursor_pos > 0 && len > 0) {
                memmove(&toolbar->address_bar_text[toolbar->address_bar_cursor_pos-1], &toolbar->address_bar_text[toolbar->address_bar_cursor_pos], len - toolbar->address_bar_cursor_pos + 1);
                toolbar->address_bar_cursor_pos--;
            } break;
        case SDL_SCANCODE_RETURN:
            if (toolbar->address_bar_text[0] != '\0') toolbar_address_bar_navigate(toolbar, toolbar->address_bar_text);
            toolbar_address_bar_stop(toolbar); break;
        case SDL_SCANCODE_ESCAPE: toolbar_address_bar_stop(toolbar); break;
        case SDL_SCANCODE_LEFT: if (toolbar->address_bar_cursor_pos > 0) toolbar->address_bar_cursor_pos--; break;
        case SDL_SCANCODE_RIGHT: if (toolbar->address_bar_cursor_pos < (int)len) toolbar->address_bar_cursor_pos++; break;
        default: break;
    }
}
```

---

## 自检清单与故障定位
- 必备入口：`toolbar_new/toolbar_free/toolbar_handle_event/toolbar_draw/toolbar_notify_directory_changed`
- 搜索/地址栏互斥：任一激活时，另一个需停止；对应 `SDL_StartTextInput/SDL_StopTextInput` 匹配。
- 历史可用性：后退可用 `history_index>0`；前进可用 `history_index<history_count-1`。
- 常见症状 → 排查点
  - “看得到点不到”：按钮 `rect` 是否为 0；`find_button_at_point` 与 `pressed/hovered` 状态是否更新。
  - “后退/前进灰置”：外部导航后未调用 `toolbar_notify_directory_changed`。
  - “搜索无输入”：激活时是否调用 `SDL_StartTextInput(window)`；文本插入是否考虑光标位置与 memmove。
  - “布局错位”：搜索按钮位置是否在 `toolbar_draw` 与搜索框同步。

以上内容确保：
- 未来新增功能、调整布局与交互都有标准流程可循；
- 对现有实现有逐段、逐行的定位与理解；
- 误删实现中的关键段落后，可直接复制“粘贴修复片段”恢复到可用状态。

