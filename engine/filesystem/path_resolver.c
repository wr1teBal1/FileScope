/*
 * 路径解析模块
 * 职责：
 * 1. 路径规范化
 * 2. 相对路径转绝对路径
 * 3. 路径合法性验证
 * 4. 特殊路径处理（环境变量、快捷方式等）
 */

#include "path_resolver.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#else
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>
#endif

// 全局配置
static PathResolverConfig g_config = {0};
static bool g_initialized = false;

// 非法字符（Windows）
static const char* INVALID_CHARS = "<>:\"|?*";

// 虚拟路径映射
typedef struct {
    const char *virtual_name;
    const char *real_path;
} VirtualPathMapping;

static VirtualPathMapping virtual_paths[] = {
    {"桌面", NULL},
    {"我的电脑", NULL},
    {"我的文档", NULL},
    {"下载", NULL},
    {"图片", NULL},
    {"音乐", NULL},
    {"视频", NULL},
    {NULL, NULL}
};

// 获取默认配置
PathResolverConfig path_get_default_config(void) {
    PathResolverConfig config = {
        .case_sensitive = false,      // Windows通常不区分大小写
        .max_path_length = 260,       // Windows默认最大路径长度
        .resolve_shortcuts = true,    // 默认解析快捷方式
        .expand_environment = true,   // 默认展开环境变量
        .resolve_virtual = true       // 默认解析虚拟路径
    };
    return config;
}

// 初始化路径解析器
bool path_resolver_init(const PathResolverConfig *config) {
    if (g_initialized) {
        return true;
    }

    if (config) {
        g_config = *config;
    } else {
        g_config = path_get_default_config();
    }

    // 初始化虚拟路径映射
#ifdef _WIN32
    char buffer[MAX_PATH];
    
    // 桌面
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_DESKTOP, NULL, 0, buffer))) {
        virtual_paths[0].real_path = strdup(buffer);
    }
    
    // 我的文档
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, 0, buffer))) {
        virtual_paths[2].real_path = strdup(buffer);
    }
    
    // 下载 - 使用环境变量获取下载文件夹路径
    char *userprofile = getenv("USERPROFILE");
    if (userprofile) {
        char *downloads_path = malloc(strlen(userprofile) + 20);
        if (downloads_path) {
            sprintf(downloads_path, "%s\\Downloads", userprofile);
            virtual_paths[3].real_path = downloads_path;
        }
    }
    
    // 图片 - 使用环境变量
    if (userprofile) {
        char *pictures_path = malloc(strlen(userprofile) + 20);
        if (pictures_path) {
            sprintf(pictures_path, "%s\\Pictures", userprofile);
            virtual_paths[4].real_path = pictures_path;
        }
    }
    
    // 音乐 - 使用环境变量
    if (userprofile) {
        char *music_path = malloc(strlen(userprofile) + 20);
        if (music_path) {
            sprintf(music_path, "%s\\Music", userprofile);
            virtual_paths[5].real_path = music_path;
        }
    }
    
    // 视频 - 使用环境变量
    if (userprofile) {
        char *videos_path = malloc(strlen(userprofile) + 20);
        if (videos_path) {
            sprintf(videos_path, "%s\\Videos", userprofile);
            virtual_paths[6].real_path = videos_path;
        }
    }
#endif

    g_initialized = true;
    return true;
}

// 清理路径解析器
void path_resolver_cleanup(void) {
    if (!g_initialized) {
        return;
    }

    // 释放虚拟路径映射
    for (int i = 0; virtual_paths[i].virtual_name != NULL; i++) {
        if (virtual_paths[i].real_path) {
            free((void*)virtual_paths[i].real_path);
            virtual_paths[i].real_path = NULL;
        }
    }

    g_initialized = false;
}

// 获取路径分隔符
char path_get_separator(void) {
#ifdef _WIN32
    return '\\';
#else
    return '/';
#endif
}

// 检查路径是否包含非法字符
bool path_has_invalid_characters(const char *path) {
    if (!path) return true;
    
    for (const char *p = path; *p; p++) {
        if (strchr(INVALID_CHARS, *p)) {
            return true;
        }
    }
    return false;
}

// 检查路径长度是否超出限制
bool path_is_too_long(const char *path) {
    if (!path) return true;
    return strlen(path) > g_config.max_path_length;
}

// 获取路径类型
PathType path_get_type(const char *path) {
    if (!path) return PATH_TYPE_RELATIVE;
    
    // 检查是否包含环境变量
    if (strchr(path, '%')) {
        return PATH_TYPE_ENVIRONMENT;
    }
    
    // 检查是否为快捷方式
    if (strstr(path, ".lnk")) {
        return PATH_TYPE_SHORTCUT;
    }
    
    // 检查是否为虚拟路径
    for (int i = 0; virtual_paths[i].virtual_name != NULL; i++) {
        if (strstr(path, virtual_paths[i].virtual_name)) {
            return PATH_TYPE_VIRTUAL;
        }
    }
    
    // 检查是否为绝对路径
#ifdef _WIN32
    if ((path[0] && path[1] == ':' && (path[2] == '\\' || path[2] == '/')) ||
        (path[0] == '\\' && path[1] == '\\')) {
        return PATH_TYPE_ABSOLUTE;
    }
#else
    if (path[0] == '/') {
        return PATH_TYPE_ABSOLUTE;
    }
#endif
    
    return PATH_TYPE_RELATIVE;
}

// 规范化路径
char* path_normalize(const char *path) {
    if (!path) return NULL;
    
    char *normalized = malloc(strlen(path) + 1);
    if (!normalized) return NULL;
    
    char *dst = normalized;
    const char *src = path;
    char separator = path_get_separator();
    
    // 处理开头的分隔符
    while (*src == separator || *src == '/') {
        *dst++ = separator;
        src++;
    }
    
    // 处理路径主体
    while (*src) {
        // 跳过连续的分隔符
        if (*src == separator || *src == '/') {
            *dst++ = separator;
            while (*src == separator || *src == '/') src++;
            continue;
        }
        
        // 处理当前目录
        if (*src == '.' && (*(src + 1) == separator || *(src + 1) == '/' || *(src + 1) == '\0')) {
            src++;
            if (*src == separator || *src == '/') src++;
            continue;
        }
        
        // 处理上级目录
        if (*src == '.' && *(src + 1) == '.' && 
            (*(src + 2) == separator || *(src + 2) == '/' || *(src + 2) == '\0')) {
            // 回退到上一个分隔符
            if (dst > normalized) {
                dst--;
                while (dst > normalized && *dst != separator) {
                    dst--;
                }
                if (dst > normalized) {
                    dst++;
                }
            }
            src += 2;
            if (*src == separator || *src == '/') src++;
            continue;
        }
        
        // 复制普通字符
        *dst++ = *src++;
    }
    
    // 移除末尾的分隔符（除非是根路径）
    if (dst > normalized && *(dst - 1) == separator && dst - normalized > 1) {
        dst--;
    }
    
    *dst = '\0';
    
    // 大小写转换（如果需要）
    if (!g_config.case_sensitive) {
        for (char *p = normalized; *p; p++) {
            *p = tolower(*p);
        }
    }
    
    return normalized;
}

// 展开环境变量
char* path_expand_environment(const char *path) {
    if (!path || !g_config.expand_environment) {
        return strdup(path);
    }
    
    char *expanded = malloc(strlen(path) * 2); // 预留空间
    if (!expanded) return NULL;
    
    char *dst = expanded;
    const char *src = path;
    
    while (*src) {
        if (*src == '%') {
            src++; // 跳过 %
            const char *var_start = src;
            
            // 找到变量名结束位置
            while (*src && *src != '%') src++;
            
            if (*src == '%') {
                // 提取变量名
                int var_len = src - var_start;
                char *var_name = malloc(var_len + 1);
                if (var_name) {
                    strncpy(var_name, var_start, var_len);
                    var_name[var_len] = '\0';
                    
                    // 获取环境变量值
#ifdef _WIN32
                    char *value = getenv(var_name);
#else
                    char *value = getenv(var_name);
#endif
                    
                    if (value) {
                        strcpy(dst, value);
                        dst += strlen(value);
                    } else {
                        // 变量不存在，保留原格式
                        *dst++ = '%';
                        strncpy(dst, var_name, var_len);
                        dst += var_len;
                        *dst++ = '%';
                    }
                    
                    free(var_name);
                }
                src++; // 跳过结束的 %
            } else {
                // 没有找到结束的 %，保留原字符
                *dst++ = '%';
                strncpy(dst, var_start, src - var_start);
                dst += src - var_start;
            }
        } else {
            *dst++ = *src++;
        }
    }
    
    *dst = '\0';
    
    // 重新分配内存到实际大小
    char *result = realloc(expanded, strlen(expanded) + 1);
    return result ? result : expanded;
}

// 解析快捷方式
char* path_resolve_shortcut(const char *shortcut_path) {
    if (!shortcut_path || !g_config.resolve_shortcuts) {
        return strdup(shortcut_path);
    }
    
#ifdef _WIN32
    if (!strstr(shortcut_path, ".lnk")) {
        return strdup(shortcut_path);
    }
    
    // 获取绝对路径
    char *abs_path = path_to_absolute(shortcut_path, NULL);
    if (!abs_path) return strdup(shortcut_path);
    
    // 使用Windows API解析快捷方式
    HRESULT hr;
    IShellLinkA *psl = NULL;
    char target_path[MAX_PATH];
    
    hr = CoCreateInstance(&CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, 
                         &IID_IShellLink, (void**)&psl);
    
    if (SUCCEEDED(hr)) {
        IPersistFile *ppf = NULL;
        hr = psl->lpVtbl->QueryInterface(psl, &IID_IPersistFile, (void**)&ppf);
        
        if (SUCCEEDED(hr)) {
            WCHAR wpath[MAX_PATH];
            MultiByteToWideChar(CP_ACP, 0, abs_path, -1, wpath, MAX_PATH);
            
            hr = ppf->lpVtbl->Load(ppf, wpath, STGM_READ);
            if (SUCCEEDED(hr)) {
                hr = psl->lpVtbl->GetPath(psl, target_path, MAX_PATH, NULL, 0);
                if (SUCCEEDED(hr)) {
                    free(abs_path);
                    ppf->lpVtbl->Release(ppf);
                    psl->lpVtbl->Release(psl);
                    return strdup(target_path);
                }
            }
            ppf->lpVtbl->Release(ppf);
        }
        psl->lpVtbl->Release(psl);
    }
    
    free(abs_path);
#endif
    
    return strdup(shortcut_path);
}

// 解析虚拟路径
char* path_resolve_virtual(const char *virtual_path) {
    if (!virtual_path || !g_config.resolve_virtual) {
        return strdup(virtual_path);
    }
    
    for (int i = 0; virtual_paths[i].virtual_name != NULL; i++) {
        if (virtual_paths[i].real_path && 
            strstr(virtual_path, virtual_paths[i].virtual_name)) {
            // 替换虚拟路径为真实路径
            char *result = malloc(strlen(virtual_path) + strlen(virtual_paths[i].real_path) + 1);
            if (result) {
                char *pos = strstr(virtual_path, virtual_paths[i].virtual_name);
                int prefix_len = pos - virtual_path;
                
                strncpy(result, virtual_path, prefix_len);
                result[prefix_len] = '\0';
                strcat(result, virtual_paths[i].real_path);
                strcat(result, pos + strlen(virtual_paths[i].virtual_name));
                
                return result;
            }
        }
    }
    
    return strdup(virtual_path);
}

// 将相对路径转换为绝对路径
char* path_to_absolute(const char *path, const char *base_path) {
    if (!path) return NULL;
    
    // 如果已经是绝对路径，直接返回
    if (path_get_type(path) == PATH_TYPE_ABSOLUTE) {
        return strdup(path);
    }
    
    // 获取基础路径
    char *base = NULL;
    if (base_path) {
        base = strdup(base_path);
    } else {
        base = malloc(MAX_PATH);
#ifdef _WIN32
        GetCurrentDirectoryA(MAX_PATH, base);
#else
        getcwd(base, MAX_PATH);
#endif
    }
    
    if (!base) return NULL;
    
    // 组合路径
    char *combined = path_combine(base, path);
    free(base);
    
    if (!combined) return NULL;
    
    // 规范化路径
    char *normalized = path_normalize(combined);
    free(combined);
    
    return normalized;
}

// 组合路径
char* path_combine(const char *path1, const char *path2) {
    if (!path1 || !path2) return NULL;
    
    char separator = path_get_separator();
    size_t len1 = strlen(path1);
    size_t len2 = strlen(path2);
    
    char *combined = malloc(len1 + len2 + 2);
    if (!combined) return NULL;
    
    strcpy(combined, path1);
    
    // 确保路径1以分隔符结尾
    if (len1 > 0 && combined[len1 - 1] != separator) {
        combined[len1] = separator;
        combined[len1 + 1] = '\0';
    }
    
    // 跳过路径2开头的分隔符
    while (*path2 == separator || *path2 == '/') path2++;
    
    strcat(combined, path2);
    
    return combined;
}

// 验证路径合法性
bool path_validate(const char *path, PathError *error) {
    if (!path) {
        if (error) *error = PATH_ERROR_INVALID_FORMAT;
        return false;
    }
    
    // 检查非法字符
    if (path_has_invalid_characters(path)) {
        if (error) *error = PATH_ERROR_INVALID_CHARACTERS;
        return false;
    }
    
    // 检查路径长度
    if (path_is_too_long(path)) {
        if (error) *error = PATH_ERROR_TOO_LONG;
        return false;
    }
    
    if (error) *error = PATH_ERROR_NONE;
    return true;
}

// 解析路径（综合处理）
PathInfo* path_resolve(const char *path, const char *base_path) {
    PathInfo *info = malloc(sizeof(PathInfo));
    if (!info) return NULL;
    
    memset(info, 0, sizeof(PathInfo));
    
    if (!path) {
        info->error = PATH_ERROR_INVALID_FORMAT;
        info->is_valid = false;
        return info;
    }
    
    // 验证路径
    if (!path_validate(path, &info->error)) {
        info->is_valid = false;
        return info;
    }
    
    // 获取路径类型
    info->type = path_get_type(path);
    
    // 处理不同类型的路径
    char *processed_path = strdup(path);
    
    // 展开环境变量
    if (info->type == PATH_TYPE_ENVIRONMENT) {
        char *expanded = path_expand_environment(processed_path);
        free(processed_path);
        processed_path = expanded;
    }
    
    // 解析虚拟路径
    if (info->type == PATH_TYPE_VIRTUAL) {
        char *resolved = path_resolve_virtual(processed_path);
        free(processed_path);
        processed_path = resolved;
    }
    
    // 解析快捷方式
    if (info->type == PATH_TYPE_SHORTCUT) {
        char *resolved = path_resolve_shortcut(processed_path);
        free(processed_path);
        processed_path = resolved;
    }
    
    // 规范化路径
    info->normalized_path = path_normalize(processed_path);
    
    // 转换为绝对路径
    info->absolute_path = path_to_absolute(info->normalized_path, base_path);
    
    // 计算相对路径
    if (base_path && info->absolute_path) {
        // 这里可以实现相对路径计算逻辑
        info->relative_path = strdup(info->normalized_path);
    } else {
        info->relative_path = strdup(info->normalized_path);
    }
    
    free(processed_path);
    
    info->is_valid = true;
    info->error = PATH_ERROR_NONE;
    
    return info;
}

// 释放路径信息
void path_info_free(PathInfo *info) {
    if (!info) return;
    
    if (info->normalized_path) free(info->normalized_path);
    if (info->absolute_path) free(info->absolute_path);
    if (info->relative_path) free(info->relative_path);
    
    free(info);
}

// 获取路径的目录部分
char* path_get_directory(const char *path) {
    if (!path) return NULL;
    
    char separator = path_get_separator();
    const char *last_sep = strrchr(path, separator);
    
    if (!last_sep) {
        // 没有分隔符，返回当前目录
        return strdup(".");
    }
    
    if (last_sep == path) {
        // 根目录
        char *result = malloc(2);
        if (result) {
            result[0] = separator;
            result[1] = '\0';
        }
        return result;
    }
    
    // 复制目录部分
    size_t len = last_sep - path;
    char *result = malloc(len + 1);
    if (result) {
        strncpy(result, path, len);
        result[len] = '\0';
    }
    
    return result;
}

// 获取路径的文件名部分
char* path_get_filename(const char *path) {
    if (!path) return NULL;
    
    char separator = path_get_separator();
    const char *last_sep = strrchr(path, separator);
    
    if (!last_sep) {
        return strdup(path);
    }
    
    return strdup(last_sep + 1);
}

// 获取路径的扩展名
char* path_get_extension(const char *path) {
    if (!path) return NULL;
    
    const char *filename = path_get_filename(path);
    if (!filename) return NULL;
    
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename) {
        free((void*)filename);
        return strdup("");
    }
    
    char *extension = strdup(dot);
    free((void*)filename);
    return extension;
}

// 获取路径的基础名（不含扩展名）
char* path_get_basename(const char *path) {
    if (!path) return NULL;
    
    const char *filename = path_get_filename(path);
    if (!filename) return NULL;
    
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename) {
        return (char*)filename;
    }
    
    size_t len = dot - filename;
    char *basename = malloc(len + 1);
    if (basename) {
        strncpy(basename, filename, len);
        basename[len] = '\0';
    }
    
    free((void*)filename);
    return basename;
}

// 获取错误描述
const char* path_get_error_string(PathError error) {
    switch (error) {
        case PATH_ERROR_NONE:
            return "No error";
        case PATH_ERROR_INVALID_CHARACTERS:
            return "Path contains invalid characters";
        case PATH_ERROR_TOO_LONG:
            return "Path is too long";
        case PATH_ERROR_INVALID_FORMAT:
            return "Invalid path format";
        case PATH_ERROR_ENVIRONMENT_VARIABLE_NOT_FOUND:
            return "Environment variable not found";
        case PATH_ERROR_SHORTCUT_RESOLUTION_FAILED:
            return "Failed to resolve shortcut";
        case PATH_ERROR_UNKNOWN:
        default:
            return "Unknown error";
    }
}
