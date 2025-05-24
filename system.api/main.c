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

    // 测试打开文件并写入
    FILE* file = open_file("test.txt", "w");
    if (file == NULL) {
        printf("文件打开失败\n");
        return 1;
    }//直接返回值即可，不要搞什么SB判断

    const char* content = "Hello, 这是一个测试文件内容！\n";
    if (fputs(content, file) == EOF) {
        printf("文件写入失败\n");
        close_file(file);
        return 1;
    }

    // 测试关闭文件
    if (close_file(file) != 0) {
        printf("文件关闭失败\n");
        return 1;
    }
    printf("文件写入并关闭成功\n");

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