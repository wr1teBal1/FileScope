cmake_minimum_required(VERSION 3.10.0)
project(FileScope VERSION 0.1.0 LANGUAGES C CXX)

add_executable(FileScope
    main.c
    app/controllers/file_ops.c
    app/controllers/view_ctrl.c
    app/models/file_item.c
    app/ui/file_list.c
    app/ui/main_window.c
    app/ui/sidebar.c
    app/ui/toolbar.c
    app/app.c
    engine/cache/thumbnail.c
    engine/filesystem/file_system.c
    engine/filesystem/file_watcher.c
    engine/filesystem/path_resolver.c
    engine/render/icon_cache.c
    engine/render/ui_renderer.c
    engine/utils/sort.c
    engine/utils/string_utils.c
    platform/sdl/events.c
    platform/sdl/init_sdl.c
    platform/sdl/renderer.c
    platform/sdl/window.c
    platform/system/clipboard.c
    platform/system/fs_api.c
    platform/system/theme.c
)

# 项目自身的include目录
include_directories(${CMAKE_SOURCE_DIR}/include)

# SDL3相关配置
include_directories(${CMAKE_SOURCE_DIR}/third_party/sdl3/include)
include_directories(${CMAKE_SOURCE_DIR}/third_party/SDL3_image/include)
include_directories(${CMAKE_SOURCE_DIR}/third_party/SDL3_ttf/include)

# 指定库文件路径
link_directories(${CMAKE_SOURCE_DIR}/third_party/sdl3/lib)
link_directories(${CMAKE_SOURCE_DIR}/third_party/SDL3_image/lib)
link_directories(${CMAKE_SOURCE_DIR}/third_party/SDL3_ttf/lib)

# 链接SDL3相关库文件
target_link_libraries(FileScope SDL3 SDL3_image SDL3_ttf)