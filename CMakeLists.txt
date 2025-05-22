cmake_minimum_required(VERSION 3.10.0)
project(FileScope VERSION 0.1.0 LANGUAGES C CXX)

add_executable(FileScope
    main.c
    src/log.c
    src/resources.c
    src/ui.c
)

#指定SDL3的头文件路径
include_directories("C:\\SDL3-devel-3.2.14-VC\\SDL3-3.2.14\\include")

#指定SDL3的库文件路径
link_directories("C:\\SDL3-devel-3.2.14-VC\\SDL3-3.2.14\\lib\\x64")

#链接SDL3的库文件
target_link_libraries(FileScope SDL3)