/*
 * 路径解析模块使用示例
 * 演示如何使用路径解析模块的各种功能
 */

#include "../include/path_resolver.h"
#include <stdio.h>
#include <stdlib.h>

void test_path_normalization(void) {
    printf("=== 路径规范化测试 ===\n");
    
    const char *test_paths[] = {
        "C:\\Dir\\Sub\\\\File",
        "C:/Dir/Sub//File",
        ".\\Downloads\\file.txt",
        "..\\..\\Documents\\file.txt",
        "C:\\Users\\Name\\Desktop\\"
    };
    
    for (int i = 0; i < 5; i++) {
        char *normalized = path_normalize(test_paths[i]);
        if (normalized) {
            printf("原始: %s\n", test_paths[i]);
            printf("规范化: %s\n\n", normalized);
            free(normalized);
        }
    }
}

void test_path_validation(void) {
    printf("=== 路径验证测试 ===\n");
    
    const char *test_paths[] = {
        "C:\\Valid\\Path\\file.txt",
        "C:\\Invalid<Path>\\file.txt",
        "C:\\Very\\Long\\Path\\That\\Exceeds\\The\\Maximum\\Length\\Limit\\And\\Should\\Be\\Rejected\\Because\\It\\Is\\Too\\Long\\For\\The\\System\\To\\Handle\\Properly\\Without\\Causing\\Issues\\With\\File\\Operations\\And\\Path\\Resolution\\file.txt",
        "C:\\Path\\With\\Invalid\\Characters\\*\\file.txt"
    };
    
    for (int i = 0; i < 4; i++) {
        PathError error;
        bool is_valid = path_validate(test_paths[i], &error);
        printf("路径: %s\n", test_paths[i]);
        printf("有效: %s\n", is_valid ? "是" : "否");
        if (!is_valid) {
            printf("错误: %s\n", path_get_error_string(error));
        }
        printf("\n");
    }
}

void test_path_resolution(void) {
    printf("=== 路径解析测试 ===\n");
    
    const char *test_paths[] = {
        "桌面\\文档.txt",
        ".\\Downloads\\file.txt",
        "%USERPROFILE%\\Documents\\file.txt",
        "C:\\Users\\Name\\Desktop\\快捷方式.lnk"
    };
    
    for (int i = 0; i < 4; i++) {
        printf("解析路径: %s\n", test_paths[i]);
        
        PathInfo *info = path_resolve(test_paths[i], "C:\\Users\\Name");
        if (info && info->is_valid) {
            printf("  类型: %d\n", info->type);
            printf("  规范化: %s\n", info->normalized_path);
            printf("  绝对路径: %s\n", info->absolute_path);
            printf("  相对路径: %s\n", info->relative_path);
        } else {
            printf("  解析失败: %s\n", path_get_error_string(info->error));
        }
        printf("\n");
        
        path_info_free(info);
    }
}

void test_path_operations(void) {
    printf("=== 路径操作测试 ===\n");
    
    const char *test_path = "C:\\Users\\Name\\Documents\\file.txt";
    
    // 获取目录
    char *dir = path_get_directory(test_path);
    printf("路径: %s\n", test_path);
    printf("目录: %s\n", dir);
    free(dir);
    
    // 获取文件名
    char *filename = path_get_filename(test_path);
    printf("文件名: %s\n", filename);
    free(filename);
    
    // 获取扩展名
    char *ext = path_get_extension(test_path);
    printf("扩展名: %s\n", ext);
    free(ext);
    
    // 获取基础名
    char *basename = path_get_basename(test_path);
    printf("基础名: %s\n", basename);
    free(basename);
    
    printf("\n");
}

void test_environment_expansion(void) {
    printf("=== 环境变量展开测试 ===\n");
    
    const char *test_paths[] = {
        "%USERPROFILE%\\Downloads",
        "%TEMP%\\file.txt",
        "C:\\Users\\Name\\%USERNAME%\\file.txt"
    };
    
    for (int i = 0; i < 3; i++) {
        char *expanded = path_expand_environment(test_paths[i]);
        if (expanded) {
            printf("原始: %s\n", test_paths[i]);
            printf("展开: %s\n\n", expanded);
            free(expanded);
        }
    }
}

void test_path_combination(void) {
    printf("=== 路径组合测试 ===\n");
    
    const char *base_paths[] = {
        "C:\\Users\\Name",
        "C:\\Users\\Name\\",
        "/home/user",
        "/home/user/"
    };
    
    const char *relative_paths[] = {
        "Documents\\file.txt",
        "Documents\\file.txt",
        "documents/file.txt",
        "documents/file.txt"
    };
    
    for (int i = 0; i < 4; i++) {
        char *combined = path_combine(base_paths[i], relative_paths[i]);
        if (combined) {
            printf("基础: %s\n", base_paths[i]);
            printf("相对: %s\n", relative_paths[i]);
            printf("组合: %s\n\n", combined);
            free(combined);
        }
    }
}

int main(void) {
    printf("路径解析模块使用示例\n");
    printf("==================\n\n");
    
    // 初始化路径解析器
    if (!path_resolver_init(NULL)) {
        printf("路径解析器初始化失败\n");
        return -1;
    }
    
    printf("路径解析器初始化成功\n\n");
    
    // 运行各种测试
    test_path_normalization();
    test_path_validation();
    test_path_resolution();
    test_path_operations();
    test_environment_expansion();
    test_path_combination();
    
    // 清理资源
    path_resolver_cleanup();
    
    printf("示例运行完成\n");
    return 0;
} 