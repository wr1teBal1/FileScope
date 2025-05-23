// main.c
#include <SDL3/SDL.h>


int main(int argc, char* argv[]) {
    bool exit_status = EXIT_FAILURE;
    // 初始化SDL
    struct Application *app = NULL;

    if (app_new(&app))
    {
        app_run(app);

        exit_status = EXIT_SUCCESS;
    }
    
    app_free(&app);n

    return exit_status;
}
