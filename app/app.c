/*
 * 应用程序核心功能模块
 * 职责：
 * 1. 应用程序生命周期管理
 * 2. 协调各个模块的工作
 * 3. 处理核心业务逻辑
 */

#include "main.h"
#include "window.h"
#include "main_window.h"
#include "event.h"
#include "renderer.h"
#include "toolbar.h"
#include "sidebar.h"

// 应用程序主循环
void app_run(struct Window *window, MainWindow *main_window) {
    printf("[DEBUG] app_run called\n");
    
    if (!window) {
        printf("[ERROR] app_run: Window is NULL\n");
        return;
    }
    
    if (!main_window) {
        printf("[ERROR] app_run: MainWindow is NULL\n");
        return;
    }
    
    printf("[DEBUG] Starting main loop, window->is_running = %s\n", window->is_running ? "true" : "false");
    
    // 主循环
    while (window->is_running) {
        
        // 处理SDL事件
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            // 处理退出事件
            if (event.type == SDL_EVENT_QUIT) {
                printf("[DEBUG] Quit event received\n");
                window->is_running = false;
                break;
            }
            
            // 处理ESC键退出
            if (event.type == SDL_EVENT_KEY_DOWN && 
                event.key.scancode == SDL_SCANCODE_ESCAPE) {
                printf("[DEBUG] ESC key pressed\n");
                window->is_running = false;
                break;
            }
            
            // 实时监测鼠标点击事件
            if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                const char* button_type = "Unknown";
                switch(event.button.button) {
                    case SDL_BUTTON_LEFT: button_type = "Left"; break;
                    case SDL_BUTTON_RIGHT: button_type = "Right"; break;
                    case SDL_BUTTON_MIDDLE: button_type = "Middle"; break;
                }
                
                printf("[USER ACTION] Mouse %s click - Position: (%d,%d) - ", 
                       button_type, event.button.x, event.button.y);
                
                // 根据点击位置判断可能的功能区域
                if (event.button.y < TOOLBAR_HEIGHT) {
                    printf("Area: Toolbar - ");
                    
                    // 尝试识别工具栏按钮
                    int x = event.button.x;
                    int button_x = BUTTON_PADDING;
                    
                    const char* button_names[] = {
                        "Back", "Forward", "Up", "Home", 
                        "Refresh", "Search", "View", "Copy"
                    };
                    
                    bool button_found = false;
                    for (int i = 0; i < BUTTON_COUNT; i++) {
                        if (x >= button_x && x < button_x + BUTTON_SIZE) {
                            printf("Button: %s\n", button_names[i]);
                            button_found = true;
                            break;
                        }
                        button_x += BUTTON_SIZE + BUTTON_SPACING;
                    }
                    
                    if (!button_found) {
                        printf("Empty area\n");
                    }
                } else if (event.button.x < SIDEBAR_WIDTH) {
                    printf("Area: Sidebar\n");
                } else {
                    printf("Area: File List\n");
                }
            }
            
            // 实时监测键盘按键事件
            if (event.type == SDL_EVENT_KEY_DOWN) {
                // 获取修饰键状态
                SDL_Keymod mod = SDL_GetModState();
                char modifiers[128] = "";
                
                if (mod & SDL_KMOD_SHIFT) strcat(modifiers, "Shift+");
                if (mod & SDL_KMOD_CTRL) strcat(modifiers, "Ctrl+");
                if (mod & SDL_KMOD_ALT) strcat(modifiers, "Alt+");
                if (mod & SDL_KMOD_GUI) strcat(modifiers, "Super+");
                
                printf("[USER ACTION] Keyboard key - %s%s (Keycode: %d)", 
                       modifiers,
                       SDL_GetScancodeName(event.key.scancode),
                       event.key.scancode);
                
                // 识别常用快捷键功能
                if ((mod & SDL_KMOD_CTRL) && event.key.scancode == SDL_SCANCODE_C) {
                    printf(" - Function: Copy");
                } else if ((mod & SDL_KMOD_CTRL) && event.key.scancode == SDL_SCANCODE_V) {
                    printf(" - Function: Paste");
                } else if ((mod & SDL_KMOD_CTRL) && event.key.scancode == SDL_SCANCODE_X) {
                    printf(" - Function: Cut");
                } else if ((mod & SDL_KMOD_CTRL) && event.key.scancode == SDL_SCANCODE_F) {
                    printf(" - Function: Search");
                } else if ((mod & SDL_KMOD_CTRL) && event.key.scancode == SDL_SCANCODE_A) {
                    printf(" - Function: Select All");
                } else if (event.key.scancode == SDL_SCANCODE_DELETE) {
                    printf(" - Function: Delete");
                } else if (event.key.scancode == SDL_SCANCODE_F2) {
                    printf(" - Function: Rename");
                } else if (event.key.scancode == SDL_SCANCODE_F5) {
                    printf(" - Function: Refresh");
                } else if (event.key.scancode == SDL_SCANCODE_RETURN) {
                    printf(" - Function: Enter/Open");
                }
                
                printf("\n");
            }
            
            // 处理窗口事件
            if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                printf("[DEBUG] Window resized to %dx%d\n", 
                       event.window.data1, event.window.data2);
                window->config.window_width = event.window.data1;
                window->config.window_height = event.window.data2;
                
                // 调用主窗口大小调整函数
                main_window_resize(main_window, event.window.data1, event.window.data2);
            } else if (event.type == SDL_EVENT_WINDOW_MOVED) {
                printf("[DEBUG] Window moved to (%d,%d)\n", 
                       event.window.data1, event.window.data2);
                window->config.window_x = event.window.data1;
                window->config.window_y = event.window.data2;
            } else if (event.type == SDL_EVENT_WINDOW_MAXIMIZED) {
                printf("[DEBUG] Window maximized\n");
                window->config.window_maximized = true;
            } else if (event.type == SDL_EVENT_WINDOW_RESTORED) {
                printf("[DEBUG] Window restored\n");
                window->config.window_maximized = false;
            }
            
            // 将事件传递给主窗口处理
            bool handled = main_window_handle_event(main_window, &event);
            
            // 输出事件是否被处理
            if (handled && (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN || 
                           event.type == SDL_EVENT_KEY_DOWN)) {
                printf("[USER ACTION] Event processed - Function executed\n");
            }
        }
        
        // 绘制界面
        window_clear(window);           // 清除渲染器
        window_draw(window);            // 绘制窗口背景内容
        main_window_draw(main_window);  // 绘制主窗口内容
        window_present(window);         // 呈现渲染结果
        
        // 限制帧率
        SDL_Delay(16); // 约60FPS
    }
}

