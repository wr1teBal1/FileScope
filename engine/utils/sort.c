/*
 * 排序工具模块
 * 职责：
 * 1. 文件排序算法
 * 2. 自定义排序规则
 * 3. 排序性能优化
 * 4. 多字段排序支持
 */

#include "sort.h"
#include "../../include/path_resolver.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <SDL3/SDL.h>

// 全局排序统计信息
static SortStats g_sort_stats = {0};

// 内部函数声明
static void quick_sort_recursive(FileItem **items, int low, int high, SortConfig *config);
static int quick_sort_partition(FileItem **items, int low, int high, SortConfig *config);
static void merge_sort_recursive(FileItem **items, FileItem **temp, int left, int right, SortConfig *config);
static void merge_sort_merge(FileItem **items, FileItem **temp, int left, int mid, int right, SortConfig *config);
static void heap_sort_build_heap(FileItem **items, int n, SortConfig *config);
static void heap_sort_heapify(FileItem **items, int n, int i, SortConfig *config);
static void insertion_sort(FileItem **items, int n, SortConfig *config);
static void selection_sort(FileItem **items, int n, SortConfig *config);
static int compare_by_field(const FileItem *item1, const FileItem *item2, SortField field, SortDirection direction, bool case_sensitive);

// 创建排序配置
SortConfig* sort_config_new(void) {
    SortConfig *config = (SortConfig*)malloc(sizeof(SortConfig));
    if (!config) return NULL;
    
    config->max_rules = 10;
    config->rules = (SortRule*)malloc(sizeof(SortRule) * config->max_rules);
    if (!config->rules) {
        free(config);
        return NULL;
    }
    
    config->rule_count = 0;
    config->folder_first = true;
    config->case_sensitive = false;
    config->natural_sort = true;
    
    return config;
}

// 释放排序配置
void sort_config_free(SortConfig *config) {
    if (config) {
        if (config->rules) {
            free(config->rules);
        }
        free(config);
    }
}

// 添加排序规则
bool sort_config_add_rule(SortConfig *config, SortField field, SortDirection direction, int priority) {
    if (!config || config->rule_count >= config->max_rules) {
        return false;
    }
    
    // 检查优先级是否已存在
    for (int i = 0; i < config->rule_count; i++) {
        if (config->rules[i].priority == priority) {
            return false;
        }
    }
    
    config->rules[config->rule_count].field = field;
    config->rules[config->rule_count].direction = direction;
    config->rules[config->rule_count].priority = priority;
    config->rule_count++;
    
    return true;
}

// 移除排序规则
bool sort_config_remove_rule(SortConfig *config, int index) {
    if (!config || index < 0 || index >= config->rule_count) {
        return false;
    }
    
    // 移动后面的规则
    for (int i = index; i < config->rule_count - 1; i++) {
        config->rules[i] = config->rules[i + 1];
    }
    config->rule_count--;
    
    return true;
}

// 清空所有排序规则
void sort_config_clear_rules(SortConfig *config) {
    if (config) {
        config->rule_count = 0;
    }
}

// 设置文件夹优先
void sort_config_set_folder_first(SortConfig *config, bool folder_first) {
    if (config) {
        config->folder_first = folder_first;
    }
}

// 设置大小写敏感
void sort_config_set_case_sensitive(SortConfig *config, bool case_sensitive) {
    if (config) {
        config->case_sensitive = case_sensitive;
    }
}

// 设置自然排序
void sort_config_set_natural_sort(SortConfig *config, bool natural_sort) {
    if (config) {
        config->natural_sort = natural_sort;
    }
}

// 对文件列表进行排序
bool sort_file_list(FileList *list, SortConfig *config, SortAlgorithm algorithm) {
    if (!list || !config || list->count <= 1) {
        return true;
    }
    
    // 重置统计信息
    sort_reset_stats();
    g_sort_stats.total_items = list->count;
    g_sort_stats.algorithm = algorithm;
    
    // 创建文件项数组
    FileItem **items = (FileItem**)malloc(sizeof(FileItem*) * list->count);
    if (!items) return false;
    
    // 填充数组
    FileItem *current = list->head;
    int index = 0;
    while (current && index < list->count) {
        items[index++] = current;
        current = current->next;
    }
    
    // 记录开始时间
    Uint64 start_time = SDL_GetPerformanceCounter();
    
    // 执行排序
    bool success = sort_file_items(items, list->count, config, algorithm);
    
    // 记录结束时间
    Uint64 end_time = SDL_GetPerformanceCounter();
    g_sort_stats.sort_time_ms = (double)(end_time - start_time) * 1000.0 / SDL_GetPerformanceFrequency();
    
    if (success) {
        // 重新构建链表
        list->head = items[0];
        list->tail = items[list->count - 1];
        
        for (int i = 0; i < list->count; i++) {
            if (i == 0) {
                items[i]->next = (i + 1 < list->count) ? items[i + 1] : NULL;
            } else if (i == list->count - 1) {
                items[i]->next = NULL;
            } else {
                items[i]->next = items[i + 1];
            }
        }
        
        g_sort_stats.sorted_items = list->count;
    }
    
    free(items);
    return success;
}

// 对文件项数组进行排序
bool sort_file_items(FileItem **items, int count, SortConfig *config, SortAlgorithm algorithm) {
    if (!items || !config || count <= 1) {
        return true;
    }
    
    switch (algorithm) {
        case SORT_ALGORITHM_QUICK:
            quick_sort_recursive(items, 0, count - 1, config);
            break;
        case SORT_ALGORITHM_MERGE:
            {
                FileItem **temp = (FileItem**)malloc(sizeof(FileItem*) * count);
                if (!temp) return false;
                merge_sort_recursive(items, temp, 0, count - 1, config);
                free(temp);
            }
            break;
        case SORT_ALGORITHM_HEAP:
            heap_sort_build_heap(items, count, config);
            for (int i = count - 1; i > 0; i--) {
                // 交换根节点和最后一个节点
                FileItem *temp = items[0];
                items[0] = items[i];
                items[i] = temp;
                g_sort_stats.swaps++;
                
                // 重新构建堆
                heap_sort_heapify(items, i, 0, config);
            }
            break;
        case SORT_ALGORITHM_INSERTION:
            insertion_sort(items, count, config);
            break;
        case SORT_ALGORITHM_SELECTION:
            selection_sort(items, count, config);
            break;
        default:
            return false;
    }
    
    return true;
}

// 快速排序递归实现
static void quick_sort_recursive(FileItem **items, int low, int high, SortConfig *config) {
    if (low < high) {
        int pi = quick_sort_partition(items, low, high, config);
        quick_sort_recursive(items, low, pi - 1, config);
        quick_sort_recursive(items, pi + 1, high, config);
    }
}

// 快速排序分区
static int quick_sort_partition(FileItem **items, int low, int high, SortConfig *config) {
    FileItem *pivot = items[high];
    int i = low - 1;
    
    for (int j = low; j < high; j++) {
        g_sort_stats.comparisons++;
        if (sort_compare_items(items[j], pivot, config) <= 0) {
            i++;
            if (i != j) {
                FileItem *temp = items[i];
                items[i] = items[j];
                items[j] = temp;
                g_sort_stats.swaps++;
            }
        }
    }
    
    if (i + 1 != high) {
        FileItem *temp = items[i + 1];
        items[i + 1] = items[high];
        items[high] = temp;
        g_sort_stats.swaps++;
    }
    
    return i + 1;
}

// 归并排序递归实现
static void merge_sort_recursive(FileItem **items, FileItem **temp, int left, int right, SortConfig *config) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        merge_sort_recursive(items, temp, left, mid, config);
        merge_sort_recursive(items, temp, mid + 1, right, config);
        merge_sort_merge(items, temp, left, mid, right, config);
    }
}

// 归并排序合并
static void merge_sort_merge(FileItem **items, FileItem **temp, int left, int mid, int right, SortConfig *config) {
    int i = left;
    int j = mid + 1;
    int k = left;
    
    while (i <= mid && j <= right) {
        g_sort_stats.comparisons++;
        if (sort_compare_items(items[i], items[j], config) <= 0) {
            temp[k++] = items[i++];
        } else {
            temp[k++] = items[j++];
        }
    }
    
    while (i <= mid) {
        temp[k++] = items[i++];
    }
    
    while (j <= right) {
        temp[k++] = items[j++];
    }
    
    for (i = left; i <= right; i++) {
        items[i] = temp[i];
    }
}

// 堆排序构建堆
static void heap_sort_build_heap(FileItem **items, int n, SortConfig *config) {
    for (int i = n / 2 - 1; i >= 0; i--) {
        heap_sort_heapify(items, n, i, config);
    }
}

// 堆排序堆化
static void heap_sort_heapify(FileItem **items, int n, int i, SortConfig *config) {
    int largest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;
    
    if (left < n) {
        g_sort_stats.comparisons++;
        if (sort_compare_items(items[left], items[largest], config) > 0) {
            largest = left;
        }
    }
    
    if (right < n) {
        g_sort_stats.comparisons++;
        if (sort_compare_items(items[right], items[largest], config) > 0) {
            largest = right;
        }
    }
    
    if (largest != i) {
        FileItem *temp = items[i];
        items[i] = items[largest];
        items[largest] = temp;
        g_sort_stats.swaps++;
        heap_sort_heapify(items, n, largest, config);
    }
}

// 插入排序
static void insertion_sort(FileItem **items, int n, SortConfig *config) {
    for (int i = 1; i < n; i++) {
        FileItem *key = items[i];
        int j = i - 1;
        
        while (j >= 0) {
            g_sort_stats.comparisons++;
            if (sort_compare_items(items[j], key, config) > 0) {
                items[j + 1] = items[j];
                g_sort_stats.swaps++;
                j--;
            } else {
                break;
            }
        }
        items[j + 1] = key;
    }
}

// 选择排序
static void selection_sort(FileItem **items, int n, SortConfig *config) {
    for (int i = 0; i < n - 1; i++) {
        int min_idx = i;
        for (int j = i + 1; j < n; j++) {
            g_sort_stats.comparisons++;
            if (sort_compare_items(items[j], items[min_idx], config) < 0) {
                min_idx = j;
            }
        }
        
        if (min_idx != i) {
            FileItem *temp = items[i];
            items[i] = items[min_idx];
            items[min_idx] = temp;
            g_sort_stats.swaps++;
        }
    }
}

// 比较两个文件项（用于排序）
int sort_compare_items(const FileItem *item1, const FileItem *item2, const SortConfig *config) {
    if (!item1 || !item2 || !config) {
        return 0;
    }
    
    // 文件夹优先处理
    if (config->folder_first) {
        bool is_dir1 = (item1->type == FILE_TYPE_DIRECTORY);
        bool is_dir2 = (item2->type == FILE_TYPE_DIRECTORY);
        
        if (is_dir1 && !is_dir2) return -1;
        if (!is_dir1 && is_dir2) return 1;
    }
    
    // 如果没有排序规则，按名称排序
    if (config->rule_count == 0) {
        return compare_by_field(item1, item2, SORT_FIELD_NAME, SORT_ASCENDING, config->case_sensitive);
    }
    
    // 多字段排序
    for (int i = 0; i < config->rule_count; i++) {
        SortRule *rule = &config->rules[i];
        int result = compare_by_field(item1, item2, rule->field, rule->direction, config->case_sensitive);
        if (result != 0) {
            return result;
        }
    }
    
    return 0;
}

// 按字段比较
static int compare_by_field(const FileItem *item1, const FileItem *item2, SortField field, SortDirection direction, bool case_sensitive) {
    int result = 0;
    
    switch (field) {
        case SORT_FIELD_NAME:
            if (item1->name && item2->name) {
                if (case_sensitive) {
                    result = strcmp(item1->name, item2->name);
                } else {
                    result = strcasecmp(item1->name, item2->name);
                }
            }
            break;
            
        case SORT_FIELD_SIZE:
            if (item1->size < item2->size) result = -1;
            else if (item1->size > item2->size) result = 1;
            break;
            
        case SORT_FIELD_TYPE:
            if (item1->type < item2->type) result = -1;
            else if (item1->type > item2->type) result = 1;
            break;
            
        case SORT_FIELD_DATE_MODIFIED:
            if (item1->modified_time < item2->modified_time) result = -1;
            else if (item1->modified_time > item2->modified_time) result = 1;
            break;
            
        case SORT_FIELD_DATE_CREATED:
            if (item1->created_time < item2->created_time) result = -1;
            else if (item1->created_time > item2->created_time) result = 1;
            break;
            
        case SORT_FIELD_DATE_ACCESSED:
            if (item1->accessed_time < item2->accessed_time) result = -1;
            else if (item1->accessed_time > item2->accessed_time) result = 1;
            break;
            
        case SORT_FIELD_EXTENSION:
            {
                char *ext1 = path_get_extension(item1->name);
                char *ext2 = path_get_extension(item2->name);
                if (ext1 && ext2) {
                    if (case_sensitive) {
                        result = strcmp(ext1, ext2);
                    } else {
                        result = strcasecmp(ext1, ext2);
                    }
                } else if (ext1) {
                    result = 1;
                } else if (ext2) {
                    result = -1;
                } else {
                    result = 0;
                }
                // 释放内存
                free(ext1);
                free(ext2);
            }
            break;
            
        case SORT_FIELD_ATTRIBUTES:
            // 比较隐藏属性
            if (item1->is_hidden != item2->is_hidden) {
                result = item1->is_hidden ? 1 : -1;
            }
            break;
            
        default:
            result = 0;
            break;
    }
    
    // 根据排序方向调整结果
    return (direction == SORT_DESCENDING) ? -result : result;
}

// 获取文件扩展名
// get_file_extension函数已移除，使用path_get_extension替代

// 自然排序比较函数
int natural_string_compare(const char *str1, const char *str2, bool case_sensitive) {
    if (!str1 || !str2) return 0;
    
    while (*str1 && *str2) {
        // 跳过前导空格
        while (*str1 == ' ') str1++;
        while (*str2 == ' ') str2++;
        
        if (!*str1 || !*str2) break;
        
        // 如果两个字符都是数字，进行数字比较
        if (isdigit(*str1) && isdigit(*str2)) {
            int num1 = 0, num2 = 0;
            
            // 提取数字
            while (isdigit(*str1)) {
                num1 = num1 * 10 + (*str1 - '0');
                str1++;
            }
            while (isdigit(*str2)) {
                num2 = num2 * 10 + (*str2 - '0');
                str2++;
            }
            
            if (num1 != num2) {
                return num1 - num2;
            }
        } else {
            // 普通字符比较
            char c1 = case_sensitive ? *str1 : tolower(*str1);
            char c2 = case_sensitive ? *str2 : tolower(*str2);
            
            if (c1 != c2) {
                return c1 - c2;
            }
            str1++;
            str2++;
        }
    }
    
    // 处理长度差异
    while (*str1 == ' ') str1++;
    while (*str2 == ' ') str2++;
    
    if (*str1 && !*str2) return 1;
    if (!*str1 && *str2) return -1;
    
    return 0;
}

// 智能选择排序算法
SortAlgorithm sort_select_algorithm(int item_count) {
    if (item_count <= 10) {
        return SORT_ALGORITHM_INSERTION;
    } else if (item_count <= 50) {
        return SORT_ALGORITHM_SELECTION;
    } else if (item_count <= 1000) {
        return SORT_ALGORITHM_QUICK;
    } else {
        return SORT_ALGORITHM_MERGE;
    }
}

// 获取排序统计信息
SortStats* sort_get_stats(void) {
    return &g_sort_stats;
}

// 重置排序统计信息
void sort_reset_stats(void) {
    memset(&g_sort_stats, 0, sizeof(SortStats));
}

// 验证排序结果
bool sort_validate_result(FileItem **items, int count, const SortConfig *config) {
    if (!items || !config || count <= 1) {
        return true;
    }
    
    for (int i = 1; i < count; i++) {
        if (sort_compare_items(items[i-1], items[i], config) > 0) {
            return false;
        }
    }
    
    return true;
}


