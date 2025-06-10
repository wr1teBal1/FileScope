#ifndef FILE_OPS_H
#define FILE_OPS_H

#include <stdbool.h>

// 文件操作函数声明

// 复制文件到剪贴板
bool file_ops_copy(const char *file_path);

// 剪切文件到剪贴板
bool file_ops_cut(const char *file_path);

// 粘贴文件从剪贴板
bool file_ops_paste(const char *target_dir);

// 删除文件
bool file_ops_delete(const char *file_path);

// 重命名文件
bool file_ops_rename(const char *old_path, const char *new_name);

// 检查剪贴板是否有数据
bool file_ops_has_clipboard_data(void);

// 获取剪贴板操作类型
int file_ops_get_clipboard_operation(void);

#endif // FILE_OPS_H