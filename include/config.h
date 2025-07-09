#ifndef CONFIG_H
#define CONFIG_H

#include "main.h"

// 应用程序配置结构
typedef struct {
    int window_width;
    int window_height;
    int window_x;
    int window_y;
    bool window_maximized;
} AppConfig;

// 配置管理函数
bool config_load(AppConfig *config);
bool config_save(const AppConfig *config);
void config_update_from_window(AppConfig *config, SDL_Window *window);

#endif // CONFIG_H