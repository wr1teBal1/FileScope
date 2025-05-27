// main.c
#include "main.h"
#include "window.h"
#include "main_window.h"
#include "renderer.h"
#include "event.h"
#include "file_system.h"

#include <SDL3/SDL_main.h>

int main(int argc, char* argv[]) {
    bool exit_status = EXIT_FAILURE;
    struct Window *window = NULL;
    MainWindow *main_window = NULL;
    char *start_dir = NULL;

    // 初始化窗口
    if (window_new(&window)) {
        // 创建主窗口
        main_window = main_window_new(window);
        if (main_window) {
            // 获取起始目录
            start_dir = (argc > 1) ? argv[1] : fs_get_current_directory();
            if (start_dir) {
                // 加载目录内容
                if (main_window->file_list_view && 
                    file_list_view_load_directory(main_window->file_list_view, start_dir)) {
                    // 运行主循环
                    window_run(window);
                    exit_status = EXIT_SUCCESS;
                }
                if (argc <= 1) {
                    free(start_dir);
                }
            }
            main_window_free(main_window);
        }
        window_free(&window);
    }

    return exit_status;
}
