// main.c
#include "main.h"
#include "window.h"
#include "main_window.h"
#include "renderer.h"
#include "event.h"

int main(int argc, char* argv[]) {
    bool exit_status = EXIT_FAILURE;
    // 初始化SDL
    struct Application *app = NULL;

    if (app_new(&app))
    {
        app_run(app);

        exit_status = EXIT_SUCCESS;
    }
    
    app_free(&app);

    return exit_status;
}
