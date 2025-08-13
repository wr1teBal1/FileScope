# 文件排序模块使用指南

## 概述

文件排序模块是FileScope资源管理器的核心功能模块之一，专门用于处理文件列表的排序功能。该模块仿照Windows文件资源管理器的排序行为，提供了灵活、高效的排序解决方案。

## 核心特性

### 1. 多种排序算法
- **快速排序**: 适用于中等规模数据（50-1000项）
- **归并排序**: 适用于大规模数据（1000+项），稳定排序
- **堆排序**: 适用于中等规模数据，原地排序
- **插入排序**: 适用于小规模数据（≤10项）
- **选择排序**: 适用于小规模数据（≤50项）

### 2. 灵活的排序规则
- 支持按名称、大小、类型、日期等字段排序
- 支持升序和降序排序
- 支持多字段组合排序
- 支持文件夹优先排序

### 3. 智能算法选择
- 根据数据量自动选择最优排序算法
- 提供性能统计和优化建议

## 数据结构

### SortConfig - 排序配置
```c
typedef struct {
    SortRule *rules;           // 排序规则数组
    int rule_count;            // 规则数量
    int max_rules;             // 最大规则数量
    bool folder_first;         // 文件夹是否优先
    bool case_sensitive;       // 是否区分大小写
    bool natural_sort;         // 是否使用自然排序
} SortConfig;
```

### SortRule - 排序规则
```c
typedef struct {
    SortField field;           // 排序字段
    SortDirection direction;    // 排序方向
    int priority;              // 优先级（用于多字段排序）
} SortRule;
```

### SortField - 排序字段枚举
```c
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
```

## 基本使用方法

### 1. 创建排序配置
```c
SortConfig *config = sort_config_new();
if (!config) {
    // 处理错误
    return;
}
```

### 2. 设置排序规则
```c
// 按名称升序排序
sort_config_add_rule(config, SORT_FIELD_NAME, SORT_ASCENDING, 1);

// 按大小降序排序
sort_config_add_rule(config, SORT_FIELD_SIZE, SORT_DESCENDING, 1);

// 多字段排序：先按类型，再按名称
sort_config_add_rule(config, SORT_FIELD_TYPE, SORT_ASCENDING, 1);
sort_config_add_rule(config, SORT_FIELD_NAME, SORT_ASCENDING, 2);
```

### 3. 配置排序选项
```c
// 设置文件夹优先
sort_config_set_folder_first(config, true);

// 设置大小写敏感
sort_config_set_case_sensitive(config, false);

// 设置自然排序
sort_config_set_natural_sort(config, true);
```

### 4. 执行排序
```c
// 对文件列表进行排序
bool success = sort_file_list(file_list, config, SORT_ALGORITHM_QUICK);

// 对文件项数组进行排序
bool success = sort_file_items(items, count, config, SORT_ALGORITHM_QUICK);
```

### 5. 清理资源
```c
sort_config_free(config);
```

## 高级功能

### 1. 智能算法选择
```c
// 根据数据量自动选择最优算法
SortAlgorithm algorithm = sort_select_algorithm(item_count);
sort_file_items(items, count, config, algorithm);
```

### 2. 性能统计
```c
// 获取排序统计信息
SortStats *stats = sort_get_stats();
printf("排序耗时: %.2f ms\n", stats->sort_time_ms);
printf("比较次数: %d\n", stats->comparisons);
printf("交换次数: %d\n", stats->swaps);

// 重置统计信息
sort_reset_stats();
```

### 3. 结果验证
```c
// 验证排序结果是否正确
bool valid = sort_validate_result(items, count, config);
if (valid) {
    printf("排序结果正确\n");
} else {
    printf("排序结果错误\n");
}
```

### 4. 性能测试
```c
// 测试不同排序算法的性能
sort_performance_test(items, count, config);
```

## 典型应用场景

### 1. 文件列表排序
```c
// 用户点击表头排序
void on_header_click(FileListView *view, SortField field) {
    SortConfig *config = sort_config_new();
    sort_config_add_rule(config, field, SORT_ASCENDING, 1);
    
    // 智能选择算法
    SortAlgorithm algorithm = sort_select_algorithm(view->files->count);
    sort_file_list(view->files, config, algorithm);
    
    // 刷新显示
    file_list_view_refresh(view);
    
    sort_config_free(config);
}
```

### 2. 多字段排序
```c
// 先按类型，再按名称排序
void sort_by_type_and_name(FileList *list) {
    SortConfig *config = sort_config_new();
    
    // 类型优先级1，名称优先级2
    sort_config_add_rule(config, SORT_FIELD_TYPE, SORT_ASCENDING, 1);
    sort_config_add_rule(config, SORT_FIELD_NAME, SORT_ASCENDING, 2);
    
    sort_file_list(list, config, SORT_ALGORITHM_QUICK);
    sort_config_free(config);
}
```

### 3. 文件夹优先排序
```c
// 文件夹始终排在前面
void sort_with_folders_first(FileList *list) {
    SortConfig *config = sort_config_new();
    
    // 启用文件夹优先
    sort_config_set_folder_first(config, true);
    
    // 按名称排序
    sort_config_add_rule(config, SORT_FIELD_NAME, SORT_ASCENDING, 1);
    
    sort_file_list(list, config, SORT_ALGORITHM_QUICK);
    sort_config_free(config);
}
```

## 性能优化建议

### 1. 算法选择
- 小数据集（≤10项）: 使用插入排序
- 中等数据集（10-1000项）: 使用快速排序
- 大数据集（1000+项）: 使用归并排序

### 2. 内存管理
- 及时释放排序配置对象
- 避免频繁创建和销毁配置对象
- 重用配置对象进行多次排序

### 3. 排序规则优化
- 优先使用高优先级规则
- 避免过多的排序规则
- 合理设置文件夹优先选项

## 错误处理

### 1. 内存分配失败
```c
SortConfig *config = sort_config_new();
if (!config) {
    printf("内存分配失败\n");
    return;
}
```

### 2. 排序失败
```c
bool success = sort_file_list(list, config, algorithm);
if (!success) {
    printf("排序操作失败\n");
    // 处理错误
}
```

### 3. 规则添加失败
```c
bool added = sort_config_add_rule(config, field, direction, priority);
if (!added) {
    printf("添加排序规则失败\n");
    // 处理错误
}
```

## 注意事项

1. **内存管理**: 使用完SortConfig后必须调用`sort_config_free()`释放内存
2. **线程安全**: 当前版本不支持多线程并发排序
3. **性能考虑**: 对于大量文件，建议使用智能算法选择
4. **错误检查**: 始终检查函数返回值，确保操作成功

## 示例代码

完整的使用示例请参考 `examples/sort_example.c` 文件，其中包含了：
- 基本排序功能演示
- 多字段排序演示
- 不同算法性能对比
- 智能算法选择演示

## 扩展开发

如需扩展排序模块功能，可以：
1. 添加新的排序字段类型
2. 实现新的排序算法
3. 优化现有算法的性能
4. 添加更多的排序选项

## 技术支持

如有问题或建议，请参考项目文档或提交issue。 