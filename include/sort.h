#ifndef SORT_H
#define SORT_H

#include "file_item.h"
#include <stdbool.h>

// 排序方向枚举
typedef enum {
    SORT_ASCENDING,    // 升序
    SORT_DESCENDING    // 降序
} SortDirection;

// 排序字段枚举
typedef enum {
    SORT_FIELD_NAME,           // 按名称排序
    SORT_FIELD_SIZE,           // 按大小排序
    SORT_FIELD_TYPE,           // 按类型排序
    SORT_FIELD_DATE_MODIFIED,  // 按修改日期排序
    SORT_FIELD_DATE_CREATED,   // 按创建日期排序
    SORT_FIELD_DATE_ACCESSED,  // 按访问日期排序
    SORT_FIELD_EXTENSION,      // 按扩展名排序
    SORT_FIELD_ATTRIBUTES      // 按属性排序
} SortField;

// 排序规则结构
typedef struct {
    SortField field;           // 排序字段
    SortDirection direction;    // 排序方向
    int priority;              // 优先级（用于多字段排序）
} SortRule;

// 排序配置结构
typedef struct {
    SortRule *rules;           // 排序规则数组
    int rule_count;            // 规则数量
    int max_rules;             // 最大规则数量
    bool folder_first;         // 文件夹是否优先
    bool case_sensitive;       // 是否区分大小写
    bool natural_sort;         // 是否使用自然排序
} SortConfig;

// 排序算法枚举
typedef enum {
    SORT_ALGORITHM_QUICK,      // 快速排序
    SORT_ALGORITHM_MERGE,      // 归并排序
    SORT_ALGORITHM_HEAP,       // 堆排序
    SORT_ALGORITHM_INSERTION,  // 插入排序（小数据集）
    SORT_ALGORITHM_SELECTION   // 选择排序（小数据集）
} SortAlgorithm;

// 排序统计信息
typedef struct {
    int total_items;           // 总项目数
    int sorted_items;          // 已排序项目数
    double sort_time_ms;       // 排序耗时（毫秒）
    int comparisons;           // 比较次数
    int swaps;                 // 交换次数
    SortAlgorithm algorithm;   // 使用的算法
} SortStats;

// 创建排序配置
SortConfig* sort_config_new(void);

// 释放排序配置
void sort_config_free(SortConfig *config);

// 添加排序规则
bool sort_config_add_rule(SortConfig *config, SortField field, SortDirection direction, int priority);

// 移除排序规则
bool sort_config_remove_rule(SortConfig *config, int index);

// 清空所有排序规则
void sort_config_clear_rules(SortConfig *config);

// 设置文件夹优先
void sort_config_set_folder_first(SortConfig *config, bool folder_first);

// 设置大小写敏感
void sort_config_set_case_sensitive(SortConfig *config, bool case_sensitive);

// 设置自然排序
void sort_config_set_natural_sort(SortConfig *config, bool natural_sort);

// 对文件列表进行排序
bool sort_file_list(FileList *list, SortConfig *config, SortAlgorithm algorithm);

// 对文件项数组进行排序
bool sort_file_items(FileItem **items, int count, SortConfig *config, SortAlgorithm algorithm);

// 获取排序统计信息
SortStats* sort_get_stats(void);

// 重置排序统计信息
void sort_reset_stats(void);

// 比较两个文件项（用于排序）
int sort_compare_items(const FileItem *item1, const FileItem *item2, const SortConfig *config);

// 获取文件扩展名
const char* get_file_extension(const char *filename);

// 自然排序比较函数
int natural_string_compare(const char *str1, const char *str2, bool case_sensitive);

// 智能选择排序算法（根据数据量自动选择）
SortAlgorithm sort_select_algorithm(int item_count);

// 验证排序结果
bool sort_validate_result(FileItem **items, int count, const SortConfig *config);



#endif // SORT_H 