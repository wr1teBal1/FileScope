// main.c
#include "main.h"
#include "window.h"
#include "main_window.h"
#include "renderer.h"
#include "event.h"
#include "file_system.h"
#include "path_resolver.h"
#include "app.h" // 添加app.h头文件
#include "toolbar.h"
#include "sidebar.h"

#include <SDL3/SDL_main.h>

// 检查各功能是否正常加载的函数
void check_features_status(MainWindow *main_window) {
    printf("\n[SYSTEM STATUS] FileScope Feature Loading Check\n");
    printf("----------------------------------------\n");
    
    // 检查文件列表视图
    printf("[FEATURE CHECK] File List View: %s\n", 
           main_window && main_window->file_list_view ? "Loaded Successfully" : "Loading Failed");
    
    // 检查工具栏
    printf("[FEATURE CHECK] Toolbar: %s\n", 
           main_window && main_window->toolbar ? "Loaded Successfully" : "Loading Failed");
    
    // 检查侧边栏
    printf("[FEATURE CHECK] Sidebar: %s\n", 
           main_window && main_window->sidebar ? "Loaded Successfully" : "Loading Failed");
    
    // 检查右键菜单
    printf("[FEATURE CHECK] Context Menu: %s\n", 
           main_window && main_window->context_menu ? "Loaded Successfully" : "Loading Failed");
    
    // 检查路径解析器
    printf("[FEATURE CHECK] Path Resolver: Loaded Successfully\n");
    
    printf("----------------------------------------\n");
    printf("[USER GUIDE] Operation Instructions:\n");
    printf("  - Left-click on file/folder: Select item\n");
    printf("  - Double-click on file/folder: Open file or enter folder\n");
    printf("  - Right-click: Show context menu\n");
    printf("  - Toolbar buttons: Navigation/Search/View switching\n");
    printf("  - Sidebar: Quick access to common locations\n");
    printf("  - F2: Rename selected item\n");
    printf("  - Delete: Delete selected item\n");
    printf("  - Ctrl+C/X/V: Copy/Cut/Paste\n");
    printf("  - Ctrl+F: Search\n");
    printf("  - F5: Refresh current directory\n");
    printf("  - ESC: Exit program\n");
    printf("----------------------------------------\n\n");
}

int main(int argc, char* argv[]) {
    // 设置控制台字符编码为UTF-8，解决中文显示乱码问题
#ifdef _WIN32
    // Windows平台设置控制台代码页为UTF-8
    system("chcp 65001 > nul");
#endif
    
    printf("[DEBUG] Program started\n");
    
    bool exit_status = EXIT_FAILURE;
    struct Window *window = NULL;
    MainWindow *main_window = NULL;
    char *start_dir = NULL;
    
    // 初始化路径解析器
    printf("[DEBUG] Initializing path resolver...\n");
    if (!path_resolver_init(NULL)) {
        printf("[ERROR] Failed to initialize path resolver\n");
        return EXIT_FAILURE;
    }
    printf("[DEBUG] Path resolver initialized successfully\n");

    printf("[DEBUG] Initializing window...\n");
    // 初始化窗口
    if (window_new(&window)) {
        printf("[DEBUG] Window initialized successfully\n");
        // 创建主窗口
        printf("[DEBUG] Creating main window...\n");
        main_window = main_window_new(window);
        if (main_window) {
            printf("[DEBUG] Main window created successfully\n");
            // 获取起始目录
            start_dir = (argc > 1) ? argv[1] : fs_get_current_directory();// 从命令行参数获取起始目录
            printf("[DEBUG] Start directory: %s\n", start_dir ? start_dir : "NULL");
            if (start_dir) {
                // 加载目录内容
                printf("[DEBUG] Loading directory content...\n");
                if (main_window->file_list_view && 
                    file_list_view_load_directory(main_window->file_list_view, start_dir)) {
                    printf("[DEBUG] Directory loaded successfully, starting main loop\n");
                    
                    // 检查各功能是否正常加载并输出操作指引
                    check_features_status(main_window);
                    
                    // 运行主循环
                    app_run(window, main_window); // 使用新的app_run函数
                    exit_status = EXIT_SUCCESS;
                } else {
                    printf("[ERROR] Failed to load directory or file_list_view is NULL\n");
                }
                if (argc <= 1) {
                    free(start_dir);
                }
            } else {
                printf("[ERROR] Failed to get start directory\n");
            }
            printf("[DEBUG] Freeing main window\n");
            main_window_free(main_window);
        } else {
            printf("[ERROR] Failed to create main window\n");
        }
        printf("[DEBUG] Freeing window\n");
        window_free(&window);
    } else {
        printf("[ERROR] Failed to initialize window\n");
    }
    
    // 清理路径解析器
    printf("[DEBUG] Cleaning up path resolver\n");
    path_resolver_cleanup();

    return exit_status;
}
