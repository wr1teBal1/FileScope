#include <stdio.h>
#include "file_manager.h"

int main() {
    // 测试创建文件
    if (create_file("test.txt") == 0) {
        printf("文件创建成功\n");
    } else {
        printf("文件创建失败\n");
        return 1;
    }

    // 测试写入文件
    const char* content = "Hello, 这是一个测试文件内容！\n";
    if (write_to_file("test.txt", content) == 0) {
        printf("文件写入成功\n");
    } else {
        printf("文件写入失败\n");
        return 1;
    }

    // 测试读取文件
    char* read_content = read_file("test.txt");
    if (read_content != NULL) {
        printf("文件内容: %s", read_content);
        free(read_content);
    } else {
        printf("文件读取失败\n");
        return 1;
    }

    // 测试复制文件
    if (copy_file("test.txt", "test_copy.txt") == 0) {
        printf("文件复制成功\n");
    } else {
        printf("文件复制失败\n");
        return 1;
    }

    // 测试删除原文件
    if (delete_file("test.txt") == 0) {
        printf("原文件删除成功\n");
    } else {
        printf("原文件删除失败\n");
        return 1;
    }

    return 0;
} 