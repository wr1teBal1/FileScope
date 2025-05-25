#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include "main.h"
#include "file_item.h"
#include <stdbool.h>
#include <dirent.h>

// 文件系统错误码
typedef enum {
    FS_ERROR_NONE,
    FS_ERROR_ACCESS_DENIED,
    FS_ERROR_NOT_FOUND,
    FS_ERROR_ALREADY_EXISTS,
    FS_ERROR_DISK_FULL,
    FS_ERROR_INVALID_NAME,
    FS_ERROR_UNKNOWN
} FSError;

// 获取最后一次错误
FSError fs_get_last_error(void);

// 获取错误描述
const char* fs_get_error_string(FSError error);

// 获取当前工作目录
char* fs_get_current_directory(void);

// 设置当前工作目录
bool fs_set_current_directory(const char *path);

// 获取用户主目录
char* fs_get_home_directory(void);

// 检查路径是否存在
bool fs_path_exists(const char *path);

// 检查是否为目录
bool fs_is_directory(const char *path);

// 检查是否为文件
bool fs_is_file(const char *path);

// 检查是否为隐藏文件
bool fs_is_hidden(const char *path);

// 获取文件大小
size_t fs_get_file_size(const char *path);

// 获取文件修改时间
time_t fs_get_modified_time(const char *path);

// 获取文件创建时间
time_t fs_get_created_time(const char *path);

// 获取文件访问时间
time_t fs_get_accessed_time(const char *path);

// 读取目录内容
DIR* fs_open_directory(const char *path);

// 关闭目录
void fs_close_directory(DIR *dir);

// 读取下一个目录项
struct dirent* fs_read_directory(DIR *dir);

// 创建目录
bool fs_create_directory(const char *path);

// 删除文件
bool fs_delete_file(const char *path);

// 删除目录
bool fs_delete_directory(const char *path);

// 重命名文件或目录
bool fs_rename(const char *old_path, const char *new_path);

// 复制文件
bool fs_copy_file(const char *src_path, const char *dst_path);

// 移动文件
bool fs_move_file(const char *src_path, const char *dst_path);

// 获取文件扩展名
const char* fs_get_extension(const char *path);

// 获取文件名（不含路径）
const char* fs_get_filename(const char *path);

// 获取文件名（不含扩展名）
const char* fs_get_basename(const char *path);

// 获取目录路径（不含文件名）
const char* fs_get_directory(const char *path);

// 组合路径
char* fs_combine_path(const char *path1, const char *path2);

// 获取绝对路径
char* fs_get_absolute_path(const char *path);

// 获取相对路径
char* fs_get_relative_path(const char *path, const char *base);

#endif // FILE_SYSTEM_H