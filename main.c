// main.c
#include "main.h"
#include "window.h"
#include "main_window.h"
#include "renderer.h"
#include "event.h"
#include "file_system.h"
#include "app.h" // 添加app.h头文件

#include <SDL3/SDL_main.h>

int main(int argc, char* argv[]) {
    printf("[DEBUG] Program started\n");
    
    bool exit_status = EXIT_FAILURE;
    struct Window *window = NULL;
    MainWindow *main_window = NULL;
    char *start_dir = NULL;

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

    return exit_status;
}
