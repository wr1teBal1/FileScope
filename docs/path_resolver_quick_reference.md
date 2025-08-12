# 路径解析模块快速参考

## 核心函数

### 初始化和清理
```c
bool path_resolver_init(const PathResolverConfig *config);
void path_resolver_cleanup(void);
```

### 路径解析
```c
PathInfo* path_resolve(const char *path, const char *base_path);
void path_info_free(PathInfo *info);
```

### 路径规范化
```c
char* path_normalize(const char *path);
char* path_to_absolute(const char *path, const char *base_path);
```

### 路径验证
```c
bool path_validate(const char *path, PathError *error);
bool path_has_invalid_characters(const char *path);
bool path_is_too_long(const char *path);
```

### 特殊路径处理
```c
char* path_expand_environment(const char *path);
char* path_resolve_shortcut(const char *shortcut_path);
char* path_resolve_virtual(const char *virtual_path);
```

### 路径操作
```c
char* path_combine(const char *path1, const char *path2);
char* path_get_directory(const char *path);
char* path_get_filename(const char *path);
char* path_get_extension(const char *path);
char* path_get_basename(const char *path);
```

## 数据结构

### PathInfo
```c
typedef struct {
    char *normalized_path;   // 规范化后的路径
    char *absolute_path;     // 绝对路径
    char *relative_path;     // 相对路径
    PathType type;           // 路径类型
    bool is_valid;           // 是否有效
    PathError error;         // 错误码
} PathInfo;
```

### PathResolverConfig
```c
typedef struct {
    bool case_sensitive;     // 是否区分大小写
    size_t max_path_length;  // 最大路径长度
    bool resolve_shortcuts;  // 是否解析快捷方式
    bool expand_environment; // 是否展开环境变量
    bool resolve_virtual;    // 是否解析虚拟路径
} PathResolverConfig;
```

## 错误码

| 错误码 | 描述 |
|--------|------|
| `PATH_ERROR_NONE` | 无错误 |
| `PATH_ERROR_INVALID_CHARACTERS` | 包含非法字符 |
| `PATH_ERROR_TOO_LONG` | 路径过长 |
| `PATH_ERROR_INVALID_FORMAT` | 路径格式无效 |
| `PATH_ERROR_ENVIRONMENT_VARIABLE_NOT_FOUND` | 环境变量未找到 |
| `PATH_ERROR_SHORTCUT_RESOLUTION_FAILED` | 快捷方式解析失败 |
| `PATH_ERROR_UNKNOWN` | 未知错误 |

## 路径类型

| 类型 | 描述 | 示例 |
|------|------|------|
| `PATH_TYPE_ABSOLUTE` | 绝对路径 | `C:\Users\Name\file.txt` |
| `PATH_TYPE_RELATIVE` | 相对路径 | `.\Downloads\file.txt` |
| `PATH_TYPE_ENVIRONMENT` | 包含环境变量 | `%USERPROFILE%\Documents` |
| `PATH_TYPE_SHORTCUT` | 快捷方式 | `file.lnk` |
| `PATH_TYPE_VIRTUAL` | 虚拟路径 | `桌面\文件.txt` |

## 常用模式

### 基本使用
```c
// 初始化
path_resolver_init(NULL);

// 解析路径
PathInfo *info = path_resolve("桌面\\文件.txt", NULL);
if (info && info->is_valid) {
    // 使用 info->absolute_path 进行文件操作
    printf("绝对路径: %s\n", info->absolute_path);
}
path_info_free(info);

// 清理
path_resolver_cleanup();
```

### 路径验证
```c
PathError error;
if (!path_validate(path, &error)) {
    printf("路径无效: %s\n", path_get_error_string(error));
}
```

### 路径规范化
```c
char *normalized = path_normalize("C:\\Dir\\\\Sub\\File");
// 使用 normalized
free(normalized);
```

### 环境变量展开
```c
char *expanded = path_expand_environment("%USERPROFILE%\\Downloads");
// 使用 expanded
free(expanded);
```

## 注意事项

1. **内存管理**: 所有返回的字符串都需要 `free()`
2. **线程安全**: 当前实现不是线程安全的
3. **编码**: 确保中文字符编码一致性
4. **权限**: 某些路径可能需要管理员权限

## 性能提示

- 路径规范化: O(n)
- 环境变量展开: O(n*m)
- 快捷方式解析: 较慢（需要文件系统访问）
- 建议对频繁访问的路径进行缓存 