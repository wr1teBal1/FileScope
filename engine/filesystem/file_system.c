/*
 * 文件系统操作模块
 * 职责：
 * 1. 封装底层文件系统操作
 * 2. 提供文件和目录操作接口
 * 3. 处理文件系统错误
 * 4. 管理文件系统权限
 *  1. 处理文件操作（复制、剪切、粘贴、删除、重命名）
 */

#include "file_system.h"
#include "sidebar.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
// #include <pwd.h>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#endif

// 当前错误码
static FSError last_error = FS_ERROR_NONE;

// 获取最后一次错误
FSError fs_get_last_error(void) {
    return last_error;
}

// 获取错误描述
const char* fs_get_error_string(FSError error) {
    switch (error) {
        case FS_ERROR_NONE:
            return "No error";
        case FS_ERROR_ACCESS_DENIED:
            return "Access denied";
        case FS_ERROR_NOT_FOUND:
            return "File or directory not found";
        case FS_ERROR_ALREADY_EXISTS:
            return "File or directory already exists";
        case FS_ERROR_DISK_FULL:
            return "Disk is full";
        case FS_ERROR_INVALID_NAME:
            return "Invalid file or directory name";
        case FS_ERROR_UNKNOWN:
        default:
            return "Unknown error";
    }
}

// 设置错误码
static void fs_set_error(FSError error) {
    last_error = error;
}

// 根据系统错误码设置错误
static void fs_set_error_from_errno(void) {
    switch (errno) {
        case EACCES:
        case EPERM:
            fs_set_error(FS_ERROR_ACCESS_DENIED);
            break;
        case ENOENT:
            fs_set_error(FS_ERROR_NOT_FOUND);
            break;
        case EEXIST:
            fs_set_error(FS_ERROR_ALREADY_EXISTS);
            break;
        case ENOSPC:
            fs_set_error(FS_ERROR_DISK_FULL);
            break;
        case EINVAL:
            fs_set_error(FS_ERROR_INVALID_NAME);
            break;
        default:
            fs_set_error(FS_ERROR_UNKNOWN);
            break;
    }
}

// 获取当前工作目录
char* fs_get_current_directory(void) {
    char *buffer = NULL;
    size_t size = 256;
    char *result = NULL;

    do {
        buffer = realloc(buffer, size);
        if (!buffer) {
            fs_set_error(FS_ERROR_UNKNOWN);
            return NULL;
        }

        result = getcwd(buffer, size);
        if (!result) {
            if (errno == ERANGE) {
                // 缓冲区太小，增加大小
                size *= 2;
            } else {
                // 其他错误
                fs_set_error_from_errno();
                free(buffer);
                return NULL;
            }
        }
    } while (!result);

    return buffer;
}

// 设置当前工作目录
bool fs_set_current_directory(const char *path) {
    if (!path) {
        fs_set_error(FS_ERROR_INVALID_NAME);
        return false;
    }

    if (chdir(path) != 0) {// 逻辑
        fs_set_error_from_errno();
        return false;
    }

    fs_set_error(FS_ERROR_NONE);
    return true;
}

// 获取用户主目录
char* fs_get_home_directory(void) {
    const char *home = getenv("HOME");
    if (home) {
        return strdup(home);
    }

    // // 如果环境变量不可用，尝试从密码数据库获取
    // struct passwd *pw = getpwuid(getuid());
    // if (pw && pw->pw_dir) {
    //     return strdup(pw->pw_dir);
    // }

    fs_set_error(FS_ERROR_UNKNOWN);
    return NULL;
}

// 检查路径是否存在
bool fs_path_exists(const char *path) {
    if (!path) {
        fs_set_error(FS_ERROR_INVALID_NAME);
        return false;
    }
    //
    struct stat st;
    if (stat(path, &st) == 0) {
        fs_set_error(FS_ERROR_NONE);
        return true;
    }

    fs_set_error_from_errno();
    return false;
}

// 检查是否为目录
bool fs_is_directory(const char *path) {
    if (!path) {
        fs_set_error(FS_ERROR_INVALID_NAME);
        return false;
    }

    struct stat st;
    if (stat(path, &st) != 0) {
        fs_set_error_from_errno();
        return false;
    }

    fs_set_error(FS_ERROR_NONE);
    return S_ISDIR(st.st_mode);
}

// 检查是否为文件
bool fs_is_file(const char *path) {
    if (!path) {
        fs_set_error(FS_ERROR_INVALID_NAME);
        return false;
    }

    struct stat st;
    if (stat(path, &st) != 0) {
        fs_set_error_from_errno();
        return false;
    }

    fs_set_error(FS_ERROR_NONE);
    return S_ISREG(st.st_mode);
}

// 检查是否为隐藏文件
bool fs_is_hidden(const char *path) {
    if (!path) {
        fs_set_error(FS_ERROR_INVALID_NAME);
        return false;
    }

    // 获取文件名（不含路径）
    const char *filename = fs_get_filename(path);
    if (!filename) {
        return false;
    }

    // 在Unix/Linux/macOS系统中，以.开头的文件被视为隐藏文件
    fs_set_error(FS_ERROR_NONE);
    return filename[0] == '.';
}

// 获取文件大小
size_t fs_get_file_size(const char *path) {
    if (!path) {
        fs_set_error(FS_ERROR_INVALID_NAME);
        return 0;
    }

    struct stat st;
    if (stat(path, &st) != 0) {
        fs_set_error_from_errno();
        return 0;
    }

    fs_set_error(FS_ERROR_NONE);
    return (size_t)st.st_size;
}

// 获取文件修改时间
time_t fs_get_modified_time(const char *path) {
    if (!path) {
        fs_set_error(FS_ERROR_INVALID_NAME);
        return 0;
    }

    struct stat st;
    if (stat(path, &st) != 0) {
        fs_set_error_from_errno();
        return 0;
    }

    fs_set_error(FS_ERROR_NONE);
    return st.st_mtime;
}

// 获取文件创建时间
time_t fs_get_created_time(const char *path) {
    if (!path) {
        fs_set_error(FS_ERROR_INVALID_NAME);
        return 0;
    }

    struct stat st;
    if (stat(path, &st) != 0) {
        fs_set_error_from_errno();
        return 0;
    }

    fs_set_error(FS_ERROR_NONE);
    return st.st_ctime;
}

// 获取文件访问时间
time_t fs_get_accessed_time(const char *path) {
    if (!path) {
        fs_set_error(FS_ERROR_INVALID_NAME);
        return 0;
    }

    struct stat st;
    if (stat(path, &st) != 0) {
        fs_set_error_from_errno();
        return 0;
    }

    fs_set_error(FS_ERROR_NONE);
    return st.st_atime;
}

// 读取目录内容
DIR* fs_open_directory(const char *path) {
    if (!path) {
        fs_set_error(FS_ERROR_INVALID_NAME);
        return NULL;
    }

    DIR *dir = opendir(path);
    if (!dir) {
        fs_set_error_from_errno();
        return NULL;
    }

    fs_set_error(FS_ERROR_NONE);
    return dir;
}

// 关闭目录
void fs_close_directory(DIR *dir) {
    if (dir) {
        closedir(dir);
    }
    fs_set_error(FS_ERROR_NONE);
}

// 读取下一个目录项
struct dirent* fs_read_directory(DIR *dir) {
    if (!dir) {
        fs_set_error(FS_ERROR_INVALID_NAME);
        return NULL;
    }

    errno = 0;
    struct dirent *entry = readdir(dir);
    if (!entry && errno != 0) {
        fs_set_error_from_errno();
    } else {
        fs_set_error(FS_ERROR_NONE);
    }

    return entry;
}

// 创建目录
bool fs_create_directory(const char *path) {
    if (!path) {
        fs_set_error(FS_ERROR_INVALID_NAME);
        return false;
    }

    // 创建目录
    if (mkdir(path) != 0) {
        fs_set_error_from_errno();
        return false;
    }

    fs_set_error(FS_ERROR_NONE);
    return true;
}

// 删除文件
bool fs_delete_file(const char *path) {
    if (!path) {
        fs_set_error(FS_ERROR_INVALID_NAME);
        return false;
    }

    if (unlink(path) != 0) {
        fs_set_error_from_errno();
        return false;
    }

    fs_set_error(FS_ERROR_NONE);
    return true;
}

// 删除目录
bool fs_delete_directory(const char *path) {
    if (!path) {
        fs_set_error(FS_ERROR_INVALID_NAME);
        return false;
    }

    if (rmdir(path) != 0) {
        fs_set_error_from_errno();
        return false;
    }

    fs_set_error(FS_ERROR_NONE);
    return true;
}

// 重命名文件或目录
bool fs_rename(const char *old_path, const char *new_path) {
    if (!old_path || !new_path) {
        fs_set_error(FS_ERROR_INVALID_NAME);
        return false;
    }

    if (rename(old_path, new_path) != 0) {
        fs_set_error_from_errno();
        return false;
    }

    fs_set_error(FS_ERROR_NONE);
    return true;
}

// 复制文件
bool fs_copy_file(const char *src_path, const char *dst_path) {
    if (!src_path || !dst_path) {
        fs_set_error(FS_ERROR_INVALID_NAME);
        return false;
    }

    FILE *src = fopen(src_path, "rb");
    if (!src) {
        fs_set_error_from_errno();
        return false;
    }

    FILE *dst = fopen(dst_path, "wb");
    if (!dst) {
        fs_set_error_from_errno();
        fclose(src);
        return false;
    }

    // 复制文件内容
    char buffer[4096];
    size_t bytes_read;
    bool success = true;

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        if (fwrite(buffer, 1, bytes_read, dst) != bytes_read) {
            fs_set_error_from_errno();
            success = false;
            break;
        }
    }

    if (ferror(src)) {
        fs_set_error_from_errno();
        success = false;
    }

    fclose(src);
    fclose(dst);

    if (success) {
        fs_set_error(FS_ERROR_NONE);
    }

    return success;
}

// 移动文件
bool fs_move_file(const char *src_path, const char *dst_path) {
    // 尝试直接重命名（在同一文件系统上效率更高）
    if (fs_rename(src_path, dst_path)) {
        return true;
    }

    // 如果重命名失败，尝试复制后删除
    if (fs_copy_file(src_path, dst_path)) {
        return fs_delete_file(src_path);
    }

    return false;
}

// 获取文件扩展名
const char* fs_get_extension(const char *path) {
    if (!path) {
        fs_set_error(FS_ERROR_INVALID_NAME);
        return NULL;
    }

    const char *filename = fs_get_filename(path);
    if (!filename) {
        return NULL;
    }

    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename) {
        // 没有扩展名或者文件名以.开头（隐藏文件）
        fs_set_error(FS_ERROR_NONE);
        return "";
    }

    fs_set_error(FS_ERROR_NONE);
    return dot + 1;
}

// 获取文件名（不含路径）
const char* fs_get_filename(const char *path) {
    if (!path) {
        fs_set_error(FS_ERROR_INVALID_NAME);
        return NULL;
    }

    // 查找最后一个路径分隔符（支持Unix和Windows）
    const char *slash = strrchr(path, '/');
    const char *backslash = strrchr(path, '\\');
    
    // 选择最后出现的分隔符
    const char *last_sep = NULL;
    if (slash && backslash) {
        last_sep = (slash > backslash) ? slash : backslash;
    } else if (slash) {
        last_sep = slash;
    } else if (backslash) {
        last_sep = backslash;
    }
    
    if (!last_sep) {
        // 没有路径分隔符，整个字符串就是文件名
        fs_set_error(FS_ERROR_NONE);
        return path;
    }

    fs_set_error(FS_ERROR_NONE);
    return last_sep + 1;
}

// 获取文件名（不含扩展名）
const char* fs_get_basename(const char *path) {
    static char basename[256];
    const char *filename = fs_get_filename(path);
    if (!filename) {
        return NULL;
    }

    strncpy(basename, filename, sizeof(basename) - 1);
    basename[sizeof(basename) - 1] = '\0';

    // 查找最后一个点
    char *dot = strrchr(basename, '.');
    if (dot && dot != basename) {
        // 如果找到点并且不是文件名的开头（隐藏文件），则截断
        *dot = '\0';
    }

    fs_set_error(FS_ERROR_NONE);
    return basename;
}

// 获取目录路径（不含文件名）
const char* fs_get_directory(const char *path) {
    static char dirname[1024];
    if (!path) {
        fs_set_error(FS_ERROR_INVALID_NAME);
        return NULL;
    }

    strncpy(dirname, path, sizeof(dirname) - 1);
    dirname[sizeof(dirname) - 1] = '\0';

    // 查找最后一个路径分隔符
    char *slash = strrchr(dirname, '/');
#ifdef _WIN32
    char *backslash = strrchr(dirname, '\\');
    // 在Windows下，使用最后出现的分隔符（/ 或 \）
    if (backslash && (!slash || backslash > slash)) {
        slash = backslash;
    }
#endif
    
    if (!slash) {
        // 没有路径分隔符，返回当前目录
        fs_set_error(FS_ERROR_NONE);
        return ".";
    }

    if (slash == dirname) {
        // 路径是根目录
        *(slash + 1) = '\0';
    } else {
        // 截断最后一个路径分隔符
        *slash = '\0';
    }

    fs_set_error(FS_ERROR_NONE);
    return dirname;
}

// 组合路径
char* fs_combine_path(const char *path1, const char *path2) {
    if (!path1 || !path2) {
        fs_set_error(FS_ERROR_INVALID_NAME);
        return NULL;
    }

    size_t len1 = strlen(path1);
    size_t len2 = strlen(path2);
    size_t len = len1 + len2 + 2; // +2 for separator and '\0'

    char *result = (char*)malloc(len);
    if (!result) {
        fs_set_error(FS_ERROR_UNKNOWN);
        return NULL;
    }

    strcpy(result, path1);

#ifdef _WIN32
    // Windows: 使用反斜杠作为路径分隔符
    if (len1 > 0 && path1[len1 - 1] != '\\' && path1[len1 - 1] != '/') {
        strcat(result, "\\");
    }
    
    // 确保路径2的开头没有分隔符
    if (path2[0] == '\\' || path2[0] == '/') {
        strcat(result, path2 + 1);
    } else {
        strcat(result, path2);
    }
#else
    // Unix/Linux: 使用正斜杠作为路径分隔符
    if (len1 > 0 && path1[len1 - 1] != '/') {
        strcat(result, "/");
    }
    
    // 确保路径2的开头没有斜杠
    if (path2[0] == '/') {
        strcat(result, path2 + 1);
    } else {
        strcat(result, path2);
    }
#endif

    fs_set_error(FS_ERROR_NONE);
    return result;
}

// 获取绝对路径
char* fs_get_absolute_path(const char *path) {
    // if (!path) {
    //     fs_set_error(FS_ERROR_INVALID_NAME);
    //     return NULL;
    // }

    // char *abs_path = realpath(path, NULL);
    // if (!abs_path) {
    //     fs_set_error_from_errno();
        return NULL;
    // }

    // fs_set_error(FS_ERROR_NONE);
    // return abs_path;
}

// 获取相对路径
char* fs_get_relative_path(const char *path, const char *base) {
    if (!path || !base) {
        fs_set_error(FS_ERROR_INVALID_NAME);
        return NULL;
    }

    // 获取绝对路径
    char *abs_path = fs_get_absolute_path(path);
    if (!abs_path) {
        return NULL;
    }

    char *abs_base = fs_get_absolute_path(base);
    if (!abs_base) {
        free(abs_path);
        return NULL;
    }

    char *rel_path = NULL;
    size_t base_len = strlen(abs_base);
    size_t path_len = strlen(abs_path);

    // 检查路径是否以基础路径开头
    if (strncmp(abs_path, abs_base, base_len) == 0 && 
        (abs_path[base_len] == '/' || abs_path[base_len] == '\0')) {
        // 路径是基础路径的子目录
        if (abs_path[base_len] == '\0') {
            // 路径与基础路径相同
            rel_path = strdup(".");
        } else {
            // 跳过基础路径和斜杠
            rel_path = strdup(abs_path + base_len + 1);
        }
    } else {
        // 路径不是基础路径的子目录，返回原始路径
        rel_path = strdup(path);
    }

    free(abs_path);
    free(abs_base);

    if (!rel_path) {
        fs_set_error(FS_ERROR_UNKNOWN);
    } else {
        fs_set_error(FS_ERROR_NONE);
    }

    return rel_path;
}

// 获取特殊文件夹路径
bool get_special_folder_path(SpecialFolder folder, char *path, size_t path_size) {
    if (!path || path_size == 0) {
        return false;
    }
    
    // 获取用户主目录
    const char *home = getenv("HOME");
    if (!home) {
        home = getenv("USERPROFILE"); // Windows环境
    }
    
    if (!home) {
        return false;
    }
    
    // 根据特殊文件夹类型拼接路径
    switch (folder) {
        case FOLDER_DESKTOP:
            snprintf(path, path_size, "%s\\Desktop", home);
            break;
        case FOLDER_DOCUMENTS:
            snprintf(path, path_size, "%s\\Documents", home);
            break;
        case FOLDER_DOWNLOADS:
            snprintf(path, path_size, "%s\\Downloads", home);
            break;
        case FOLDER_MUSIC:
            snprintf(path, path_size, "%s\\Music", home);
            break;
        case FOLDER_PICTURES:
            snprintf(path, path_size, "%s\\Pictures", home);
            break;
        case FOLDER_VIDEOS:
            snprintf(path, path_size, "%s\\Videos", home);
            break;
        default:
            return false;
    }
    
    // 检查路径是否存在
    if (!fs_path_exists(path)) {
        // 如果路径不存在，尝试使用Linux风格路径
        switch (folder) {
            case FOLDER_DESKTOP:
                snprintf(path, path_size, "%s/Desktop", home);
                break;
            case FOLDER_DOCUMENTS:
                snprintf(path, path_size, "%s/Documents", home);
                break;
            case FOLDER_DOWNLOADS:
                snprintf(path, path_size, "%s/Downloads", home);
                break;
            case FOLDER_MUSIC:
                snprintf(path, path_size, "%s/Music", home);
                break;
            case FOLDER_PICTURES:
                snprintf(path, path_size, "%s/Pictures", home);
                break;
            case FOLDER_VIDEOS:
                snprintf(path, path_size, "%s/Videos", home);
                break;
            default:
                return false;
        }
        
        // 再次检查路径是否存在
        if (!fs_path_exists(path)) {
            return false;
        }
    }
    
    return true;
}

// 获取驱动器列表（Windows平台）
int get_drives(DriveInfo *drives, int max_count) {
    if (!drives || max_count <= 0) {
        return 0;
    }
    
#ifdef _WIN32
    // Windows平台使用GetLogicalDriveStrings获取驱动器列表
    char buffer[1024];
    DWORD result = GetLogicalDriveStringsA(sizeof(buffer), buffer);
    
    if (result == 0 || result > sizeof(buffer)) {
        return 0;
    }
    
    int count = 0;
    char *drive_ptr = buffer;
    
    while (*drive_ptr && count < max_count) {
        // 获取驱动器盘符
        drives[count].letter = drive_ptr[0];
        
        // 获取驱动器标签
        char volume_name[256] = {0};
        char fs_name[32] = {0};
        DWORD serial_number = 0;
        DWORD max_component_length = 0;
        DWORD fs_flags = 0;
        
        char root_path[4] = {drive_ptr[0], ':', '\\', 0};
        
        if (GetVolumeInformationA(
                root_path,
                volume_name,
                sizeof(volume_name),
                &serial_number,
                &max_component_length,
                &fs_flags,
                fs_name,
                sizeof(fs_name))) {
            strncpy(drives[count].label, volume_name, sizeof(drives[count].label) - 1);
            strncpy(drives[count].fs_type, fs_name, sizeof(drives[count].fs_type) - 1);
        }
        
        // 获取驱动器大小信息
        ULARGE_INTEGER total_bytes, free_bytes;
        if (GetDiskFreeSpaceExA(
                root_path,
                NULL,
                &total_bytes,
                &free_bytes)) {
            drives[count].total_size = total_bytes.QuadPart;
            drives[count].free_size = free_bytes.QuadPart;
        }
        
        // 判断是否为可移动设备
        drives[count].is_removable = (GetDriveTypeA(root_path) == DRIVE_REMOVABLE);
        
        count++;
        
        // 移动到下一个驱动器
        while (*drive_ptr) drive_ptr++;
        drive_ptr++;
    }
    
    return count;
#else
    // 非Windows平台，仅返回根目录
    if (max_count > 0) {
        drives[0].letter = '/';
        strncpy(drives[0].label, "Root", sizeof(drives[0].label) - 1);
        strncpy(drives[0].fs_type, "Unknown", sizeof(drives[0].fs_type) - 1);
        drives[0].total_size = 0;
        drives[0].free_size = 0;
        drives[0].is_removable = false;
        return 1;
    }
    return 0;
#endif
}
