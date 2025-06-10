/*
 * 文件操作控制器
 * 职责：
 * 1. 处理文件操作（复制、剪切、粘贴、删除、重命名）
 * 2. 文件操作进度显示
 * 3. 错误处理和用户提示
 * 4. 撤销/重做支持
 */

#include "file_system.h"
#include "file_item.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 剪贴板操作类型
typedef enum {
    CLIPBOARD_NONE,
    CLIPBOARD_COPY,
    CLIPBOARD_CUT
} ClipboardOperation;

// 剪贴板数据结构
typedef struct {
    char *file_path;           // 文件路径
    ClipboardOperation op;     // 操作类型
    bool is_valid;            // 是否有效
} ClipboardData;

// 全局剪贴板数据
static ClipboardData g_clipboard = {NULL, CLIPBOARD_NONE, false};

// 清空剪贴板
static void clipboard_clear(void) {
    if (g_clipboard.file_path) {
        free(g_clipboard.file_path);
        g_clipboard.file_path = NULL;
    }
    g_clipboard.op = CLIPBOARD_NONE;
    g_clipboard.is_valid = false;
}

// 复制文件到剪贴板
bool file_ops_copy(const char *file_path) {
    if (!file_path || !fs_path_exists(file_path)) {
        printf("[ERROR] Invalid file path for copy: %s\n", file_path ? file_path : "NULL");
        return false;
    }

    clipboard_clear();
    
    g_clipboard.file_path = strdup(file_path);
    if (!g_clipboard.file_path) {
        printf("[ERROR] Failed to allocate memory for clipboard\n");
        return false;
    }
    
    g_clipboard.op = CLIPBOARD_COPY;
    g_clipboard.is_valid = true;
    
    printf("[INFO] File copied to clipboard: %s\n", file_path);
    return true;
}

// 剪切文件到剪贴板
bool file_ops_cut(const char *file_path) {
    if (!file_path || !fs_path_exists(file_path)) {
        printf("[ERROR] Invalid file path for cut: %s\n", file_path ? file_path : "NULL");
        return false;
    }

    clipboard_clear();
    
    g_clipboard.file_path = strdup(file_path);
    if (!g_clipboard.file_path) {
        printf("[ERROR] Failed to allocate memory for clipboard\n");
        return false;
    }
    
    g_clipboard.op = CLIPBOARD_CUT;
    g_clipboard.is_valid = true;
    
    printf("[INFO] File cut to clipboard: %s\n", file_path);
    return true;
}

// 粘贴文件从剪贴板
bool file_ops_paste(const char *target_dir) {
    if (!g_clipboard.is_valid || !g_clipboard.file_path) {
        printf("[ERROR] No file in clipboard to paste\n");
        return false;
    }
    
    if (!target_dir || !fs_path_exists(target_dir)) {
        printf("[ERROR] Invalid target directory: %s\n", target_dir ? target_dir : "NULL");
        return false;
    }
    
    // 获取源文件名
    const char *filename = fs_get_filename(g_clipboard.file_path);
    if (!filename) {
        printf("[ERROR] Failed to get filename from path: %s\n", g_clipboard.file_path);
        return false;
    }
    
    // 构建目标路径
    char *target_path = fs_combine_path(target_dir, filename);
    if (!target_path) {
        printf("[ERROR] Failed to combine target path\n");
        return false;
    }
    
    bool success = false;
    
    if (g_clipboard.op == CLIPBOARD_COPY) {
        // 复制操作
        success = fs_copy_file(g_clipboard.file_path, target_path);
        if (success) {
            printf("[INFO] File copied successfully: %s -> %s\n", g_clipboard.file_path, target_path);
        } else {
            printf("[ERROR] Failed to copy file: %s -> %s\n", g_clipboard.file_path, target_path);
        }
    } else if (g_clipboard.op == CLIPBOARD_CUT) {
        // 剪切操作（移动文件）
        success = fs_move_file(g_clipboard.file_path, target_path);
        if (success) {
            printf("[INFO] File moved successfully: %s -> %s\n", g_clipboard.file_path, target_path);
            // 剪切操作完成后清空剪贴板
            clipboard_clear();
        } else {
            printf("[ERROR] Failed to move file: %s -> %s\n", g_clipboard.file_path, target_path);
        }
    }
    
    free(target_path);
    return success;
}

// 删除文件
bool file_ops_delete(const char *file_path) {
    if (!file_path || !fs_path_exists(file_path)) {
        printf("[ERROR] Invalid file path for delete: %s\n", file_path ? file_path : "NULL");
        return false;
    }
    
    bool success = fs_delete_file(file_path);
    if (success) {
        printf("[INFO] File deleted successfully: %s\n", file_path);
    } else {
        printf("[ERROR] Failed to delete file: %s\n", file_path);
    }
    
    return success;
}

// 重命名文件
bool file_ops_rename(const char *old_path, const char *new_name) {
    if (!old_path || !new_name || !fs_path_exists(old_path)) {
        printf("[ERROR] Invalid parameters for rename\n");
        return false;
    }
    
    // 获取父目录
    const char *parent_dir_const = fs_get_directory(old_path);
    if (!parent_dir_const) {
        printf("[ERROR] Failed to get parent directory\n");
        return false;
    }
    
    // 复制父目录路径
    char *parent_dir = strdup(parent_dir_const);
    if (!parent_dir) {
        printf("[ERROR] Failed to allocate memory for parent directory\n");
        return false;
    }
    
    // 构建新路径
    char *new_path = fs_combine_path(parent_dir, new_name);
    free(parent_dir);
    
    if (!new_path) {
        printf("[ERROR] Failed to combine new path\n");
        return false;
    }
    
    bool success = fs_rename(old_path, new_path);
    if (success) {
        printf("[INFO] File renamed successfully: %s -> %s\n", old_path, new_path);
    } else {
        printf("[ERROR] Failed to rename file: %s -> %s\n", old_path, new_path);
    }
    
    free(new_path);
    return success;
}

// 检查剪贴板是否有数据
bool file_ops_has_clipboard_data(void) {
    return g_clipboard.is_valid && g_clipboard.file_path != NULL;
}

// 获取剪贴板操作类型
int file_ops_get_clipboard_operation(void) {
    if (!g_clipboard.is_valid) {
        return CLIPBOARD_NONE;
    }
    return g_clipboard.op;
}
