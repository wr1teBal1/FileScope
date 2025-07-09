/*
 * 配置管理模块
 * 职责：
 * 1. 保存和加载应用程序配置
 * 2. 窗口大小记忆功能
 * 3. 配置文件管理
 */

#include "../../include/config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CONFIG_FILE "config.ini"
#define MAX_LINE_LENGTH 256

// 默认配置
static AppConfig default_config = {
    .window_width = 800,
    .window_height = 600,
    .window_x = SDL_WINDOWPOS_CENTERED,
    .window_y = SDL_WINDOWPOS_CENTERED,
    .window_maximized = false
};

// 加载配置文件
bool config_load(AppConfig *config) {
    if (!config) {
        return false;
    }
    
    // 先设置默认值
    *config = default_config;
    
    FILE *file = fopen(CONFIG_FILE, "r");
    if (!file) {
        printf("[INFO] Config file not found, using defaults\n");
        return true; // 使用默认配置
    }
    
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        // 移除换行符
        line[strcspn(line, "\n")] = 0;
        
        // 跳过空行和注释
        if (line[0] == '\0' || line[0] == '#' || line[0] == ';') {
            continue;
        }
        
        // 解析键值对
        char *key = strtok(line, "=");
        char *value = strtok(NULL, "=");
        
        if (!key || !value) {
            continue;
        }
        
        // 去除前后空格
        while (*key == ' ') key++;
        while (*value == ' ') value++;
        
        // 解析配置项
        if (strcmp(key, "window_width") == 0) {
            config->window_width = atoi(value);
        } else if (strcmp(key, "window_height") == 0) {
            config->window_height = atoi(value);
        } else if (strcmp(key, "window_x") == 0) {
            config->window_x = atoi(value);
        } else if (strcmp(key, "window_y") == 0) {
            config->window_y = atoi(value);
        } else if (strcmp(key, "window_maximized") == 0) {
            config->window_maximized = (strcmp(value, "true") == 0 || strcmp(value, "1") == 0);
        }
    }
    
    fclose(file);
    printf("[INFO] Config loaded: %dx%d at (%d,%d)\n", 
           config->window_width, config->window_height, 
           config->window_x, config->window_y);
    return true;
}

// 保存配置文件
bool config_save(const AppConfig *config) {
    if (!config) {
        return false;
    }
    
    FILE *file = fopen(CONFIG_FILE, "w");
    if (!file) {
        printf("[ERROR] Failed to save config file\n");
        return false;
    }
    
    fprintf(file, "# FileScope Configuration File\n");
    fprintf(file, "# Window settings\n");
    fprintf(file, "window_width=%d\n", config->window_width);
    fprintf(file, "window_height=%d\n", config->window_height);
    fprintf(file, "window_x=%d\n", config->window_x);
    fprintf(file, "window_y=%d\n", config->window_y);
    fprintf(file, "window_maximized=%s\n", config->window_maximized ? "true" : "false");
    
    fclose(file);
    printf("[INFO] Config saved\n");
    return true;
}

// 从窗口更新配置
void config_update_from_window(AppConfig *config, SDL_Window *window) {
    if (!config || !window) {
        return;
    }
    
    // 获取窗口大小
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    config->window_width = w;
    config->window_height = h;
    
    // 获取窗口位置
    int x, y;
    SDL_GetWindowPosition(window, &x, &y);
    config->window_x = x;
    config->window_y = y;
    
    // 检查是否最大化
    Uint32 flags = SDL_GetWindowFlags(window);
    config->window_maximized = (flags & SDL_WINDOW_MAXIMIZED) != 0;
}