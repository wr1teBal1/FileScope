/*
 * 排序模块使用示例
 * 演示如何使用排序模块的各种功能
 */

#include "../include/sort.h"
#include "../include/file_item.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// 创建测试文件项
FileItem* create_test_file_item(const char *name, FileType type, size_t size, time_t modified_time) {
    FileItem *item = (FileItem*)malloc(sizeof(FileItem));
    if (!item) return NULL;
    
    item->name = strdup(name);
    item->path = strdup(name);
    item->display_name = strdup(name);
    item->type = type;
    item->size = size;
    item->modified_time = modified_time;
    item->created_time = modified_time;
    item->accessed_time = modified_time;
    item->is_hidden = false;
    item->is_selected = false;
    item->icon = NULL;
    item->next = NULL;
    
    return item;
}

// 释放测试文件项
void free_test_file_item(FileItem *item) {
    if (item) {
        free(item->name);
        free(item->path);
        free(item->display_name);
        free(item);
    }
}

// 打印文件列表
void print_file_list(FileItem **items, int count) {
    printf("文件列表 (共 %d 项):\n", count);
    printf("%-20s %-10s %-15s %-20s\n", "名称", "类型", "大小", "修改时间");
    printf("------------------------------------------------------------\n");
    
    for (int i = 0; i < count; i++) {
        FileItem *item = items[i];
        const char *type_str = (item->type == FILE_TYPE_DIRECTORY) ? "目录" : "文件";
        char time_str[64];
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M", localtime(&item->modified_time));
        
        printf("%-20s %-10s %-15zu %-20s\n", 
               item->name, type_str, item->size, time_str);
    }
    printf("\n");
}

// 演示基本排序功能
void demo_basic_sorting(void) {
    printf("=== 基本排序功能演示 ===\n");
    
    // 创建排序配置
    SortConfig *config = sort_config_new();
    if (!config) {
        printf("创建排序配置失败\n");
        return;
    }
    
    // 创建测试数据
    FileItem **items = (FileItem**)malloc(sizeof(FileItem*) * 8);
    if (!items) {
        sort_config_free(config);
        return;
    }
    
    // 填充测试数据
    items[0] = create_test_file_item("document.txt", FILE_TYPE_REGULAR, 1024, time(NULL) - 3600);
    items[1] = create_test_file_item("folder1", FILE_TYPE_DIRECTORY, 0, time(NULL) - 7200);
    items[2] = create_test_file_item("image.jpg", FILE_TYPE_REGULAR, 2048, time(NULL) - 1800);
    items[3] = create_test_file_item("data.csv", FILE_TYPE_REGULAR, 512, time(NULL) - 5400);
    items[4] = create_test_file_item("backup", FILE_TYPE_DIRECTORY, 0, time(NULL) - 10800);
    items[5] = create_test_file_item("readme.md", FILE_TYPE_REGULAR, 256, time(NULL) - 9000);
    items[6] = create_test_file_item("temp", FILE_TYPE_DIRECTORY, 0, time(NULL) - 3600);
    items[7] = create_test_file_item("config.ini", FILE_TYPE_REGULAR, 128, time(NULL) - 7200);
    
    printf("原始文件列表:\n");
    print_file_list(items, 8);
    
    // 按名称排序（升序）
    printf("按名称排序（升序）:\n");
    sort_file_items(items, 8, config, SORT_ALGORITHM_QUICK);
    print_file_list(items, 8);
    
    // 按大小排序（降序）
    printf("按大小排序（降序）:\n");
    sort_config_clear_rules(config);
    sort_config_add_rule(config, SORT_FIELD_SIZE, SORT_DESCENDING, 1);
    sort_file_items(items, 8, config, SORT_ALGORITHM_QUICK);
    print_file_list(items, 8);
    
    // 按修改时间排序（降序）
    printf("按修改时间排序（降序）:\n");
    sort_config_clear_rules(config);
    sort_config_add_rule(config, SORT_FIELD_DATE_MODIFIED, SORT_DESCENDING, 1);
    sort_file_items(items, 8, config, SORT_ALGORITHM_QUICK);
    print_file_list(items, 8);
    
    // 清理
    for (int i = 0; i < 8; i++) {
        free_test_file_item(items[i]);
    }
    free(items);
    sort_config_free(config);
}

// 演示多字段排序
void demo_multi_field_sorting(void) {
    printf("=== 多字段排序演示 ===\n");
    
    SortConfig *config = sort_config_new();
    if (!config) return;
    
    // 创建更多测试数据
    FileItem **items = (FileItem**)malloc(sizeof(FileItem*) * 12);
    if (!items) {
        sort_config_free(config);
        return;
    }
    
    // 填充测试数据
    items[0] = create_test_file_item("doc1.txt", FILE_TYPE_REGULAR, 1024, time(NULL) - 3600);
    items[1] = create_test_file_item("doc2.txt", FILE_TYPE_REGULAR, 2048, time(NULL) - 7200);
    items[2] = create_test_file_item("doc3.txt", FILE_TYPE_REGULAR, 512, time(NULL) - 1800);
    items[3] = create_test_file_item("img1.jpg", FILE_TYPE_REGULAR, 4096, time(NULL) - 5400);
    items[4] = create_test_file_item("img2.jpg", FILE_TYPE_REGULAR, 2048, time(NULL) - 9000);
    items[5] = create_test_file_item("data1.csv", FILE_TYPE_REGULAR, 256, time(NULL) - 10800);
    items[6] = create_test_file_item("data2.csv", FILE_TYPE_REGULAR, 512, time(NULL) - 3600);
    items[7] = create_test_file_item("folder1", FILE_TYPE_DIRECTORY, 0, time(NULL) - 7200);
    items[8] = create_test_file_item("folder2", FILE_TYPE_DIRECTORY, 0, time(NULL) - 1800);
    items[9] = create_test_file_item("backup1", FILE_TYPE_DIRECTORY, 0, time(NULL) - 5400);
    items[10] = create_test_file_item("backup2", FILE_TYPE_DIRECTORY, 0, time(NULL) - 9000);
    items[11] = create_test_file_item("temp", FILE_TYPE_DIRECTORY, 0, time(NULL) - 10800);
    
    printf("原始文件列表:\n");
    print_file_list(items, 12);
    
    // 多字段排序：先按类型，再按名称
    printf("多字段排序：先按类型（目录优先），再按名称:\n");
    sort_config_clear_rules(config);
    sort_config_add_rule(config, SORT_FIELD_TYPE, SORT_ASCENDING, 1);  // 类型优先级1
    sort_config_add_rule(config, SORT_FIELD_NAME, SORT_ASCENDING, 2);  // 名称优先级2
    sort_file_items(items, 12, config, SORT_ALGORITHM_QUICK);
    print_file_list(items, 12);
    
    // 多字段排序：先按扩展名，再按大小
    printf("多字段排序：先按扩展名，再按大小（降序）:\n");
    sort_config_clear_rules(config);
    sort_config_add_rule(config, SORT_FIELD_EXTENSION, SORT_ASCENDING, 1);
    sort_config_add_rule(config, SORT_FIELD_SIZE, SORT_DESCENDING, 2);
    sort_file_items(items, 12, config, SORT_ALGORITHM_QUICK);
    print_file_list(items, 12);
    
    // 清理
    for (int i = 0; i < 12; i++) {
        free_test_file_item(items[i]);
    }
    free(items);
    sort_config_free(config);
}

// 演示不同排序算法
void demo_sorting_algorithms(void) {
    printf("=== 排序算法性能对比 ===\n");
    
    SortConfig *config = sort_config_new();
    if (!config) return;
    
    // 设置排序规则：按名称升序
    sort_config_add_rule(config, SORT_FIELD_NAME, SORT_ASCENDING, 1);
    
    // 创建大量测试数据
    const int test_count = 1000;
    FileItem **items = (FileItem**)malloc(sizeof(FileItem*) * test_count);
    if (!items) {
        sort_config_free(config);
        return;
    }
    
    // 生成随机测试数据
    srand((unsigned int)time(NULL));
    for (int i = 0; i < test_count; i++) {
        char name[64];
        snprintf(name, sizeof(name), "file_%04d.txt", rand() % 10000);
        items[i] = create_test_file_item(name, FILE_TYPE_REGULAR, rand() % 1000000, time(NULL) - rand() % 86400);
    }
    
    printf("测试数据: %d 个文件\n", test_count);
    
    // 测试不同排序算法
    SortAlgorithm algorithms[] = {
        SORT_ALGORITHM_QUICK,
        SORT_ALGORITHM_MERGE,
        SORT_ALGORITHM_HEAP,
        SORT_ALGORITHM_INSERTION,
        SORT_ALGORITHM_SELECTION
    };
    
    const char* algorithm_names[] = {
        "快速排序",
        "归并排序",
        "堆排序",
        "插入排序",
        "选择排序"
    };
    
    for (int i = 0; i < 5; i++) {
        // 创建测试数据副本
        FileItem **test_items = (FileItem**)malloc(sizeof(FileItem*) * test_count);
        if (!test_items) continue;
        
        memcpy(test_items, items, sizeof(FileItem*) * test_count);
        
        // 重置统计信息
        sort_reset_stats();
        
        // 记录开始时间
        clock_t start_time = clock();
        
        // 执行排序
        sort_file_items(test_items, test_count, config, algorithms[i]);
        
        // 记录结束时间
        clock_t end_time = clock();
        double sort_time_ms = (double)(end_time - start_time) * 1000.0 / CLOCKS_PER_SEC;
        
        // 验证结果
        bool valid = sort_validate_result(test_items, test_count, config);
        
        printf("%s: %.2f ms, 比较: %d, 交换: %d, 结果: %s\n",
               algorithm_names[i],
               sort_time_ms,
               sort_get_stats()->comparisons,
               sort_get_stats()->swaps,
               valid ? "正确" : "错误");
        
        free(test_items);
    }
    
    // 清理
    for (int i = 0; i < test_count; i++) {
        free_test_file_item(items[i]);
    }
    free(items);
    sort_config_free(config);
}

// 演示智能算法选择
void demo_smart_algorithm_selection(void) {
    printf("=== 智能算法选择演示 ===\n");
    
    SortConfig *config = sort_config_new();
    if (!config) return;
    
    sort_config_add_rule(config, SORT_FIELD_NAME, SORT_ASCENDING, 1);
    
    // 测试不同数据量
    int test_sizes[] = {5, 25, 100, 500, 2000};
    
    for (int i = 0; i < 5; i++) {
        int count = test_sizes[i];
        FileItem **items = (FileItem**)malloc(sizeof(FileItem*) * count);
        if (!items) continue;
        
        // 生成测试数据
        for (int j = 0; j < count; j++) {
            char name[64];
            snprintf(name, sizeof(name), "item_%04d", rand() % 10000);
            items[j] = create_test_file_item(name, FILE_TYPE_REGULAR, rand() % 1000, time(NULL));
        }
        
        // 自动选择算法
        SortAlgorithm selected = sort_select_algorithm(count);
        const char* algorithm_names[] = {"插入排序", "选择排序", "快速排序", "归并排序"};
        int algorithm_index = (selected == SORT_ALGORITHM_INSERTION) ? 0 :
                            (selected == SORT_ALGORITHM_SELECTION) ? 1 :
                            (selected == SORT_ALGORITHM_QUICK) ? 2 : 3;
        
        printf("数据量 %d: 自动选择 %s\n", count, algorithm_names[algorithm_index]);
        
        // 执行排序
        sort_file_items(items, count, config, selected);
        
        // 清理
        for (int j = 0; j < count; j++) {
            free_test_file_item(items[j]);
        }
        free(items);
    }
    
    sort_config_free(config);
}

// 主函数
int main(void) {
    printf("文件排序模块演示程序\n");
    printf("====================\n\n");
    
    // 演示各种功能
    demo_basic_sorting();
    demo_multi_field_sorting();
    demo_sorting_algorithms();
    demo_smart_algorithm_selection();
    
    printf("演示完成！\n");
    return 0;
} 