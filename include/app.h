#ifndef APP_H
#define APP_H

#include "main.h"
#include "window.h"
#include "main_window.h"

/**
 * 应用程序主循环
 * 负责处理事件、更新界面和控制帧率
 * 
 * @param window 应用程序窗口
 * @param main_window 主窗口界面
 */
void app_run(struct Window *window, MainWindow *main_window);

#endif // APP_H