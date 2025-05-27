/*
 * 文件项数据模型
 * 职责：
 * 1. 定义文件项的数据结构
 * 2. 文件属性管理（名称、大小、类型、日期等）
 * 3. 文件元数据缓存
 * 4. 文件状态跟踪（选中、重命名等）
 */

#include "file_item.h"
#include "file_system.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


// 创建文件项
FileItem* file_item_new(const char *path) {
    if (!path || !fs_path_exists(path)) {
        return NULL;
    }

    FileItem *item = (FileItem*)calloc(1, sizeof(FileItem));
    if (!item) {
        return NULL;
    }

    // 设置路径
    item->path = strdup(path);
    if (!item->path) {
        free(item);
        item = NULL;
        return NULL;
    }

    // 设置文件名
    const char *filename = fs_get_filename(path);
    if (filename) {
        item->name = strdup(filename);
        item->display_name = strdup(filename);
    } else {
        item->name = strdup(path);
        item->display_name = strdup(path);
    }

    // 获取文件信息
    struct stat st;
    if (stat(path, &st) == 0) {
        item->type = get_file_type(&st);
        item->size = st.st_size;
        item->modified_time = st.st_mtime;
        item->created_time = st.st_ctime;
        item->accessed_time = st.st_atime;
    } else {
        item->type = FILE_TYPE_UNKNOWN;
    }

    // 检查是否为隐藏文件
    item->is_hidden = fs_is_hidden(path);
    item->is_selected = false;
    item->icon = NULL;
    item->next = NULL;

    return item;
}

// 释放文件项
void file_item_free(FileItem *item) {
    if (!item) {
        return;
    }

    if (item->path) {
        free(item->path);
    }
    if (item->name) {
        free(item->name);
    }
    if (item->display_name) {
        free(item->display_name);
    }
    if (item->icon) {
        SDL_DestroyTexture(item->icon);
    }

    free(item);
    item = NULL;
}

// 创建文件列表
FileList* file_list_new(void) {
    FileList *list = (FileList*)calloc(1, sizeof(FileList));
    if (!list) {
        return NULL;
    }

    list->head = NULL;
    list->tail = NULL;
    list->count = 0;
    list->current_dir = NULL;

    return list;
}

// 释放文件列表
void file_list_free(FileList *list) {
    if (!list) {
        return;
    }

    // 释放所有文件项
    file_list_clear(list);

    // 释放当前目录
    if (list->current_dir) {
        free(list->current_dir);
        list->current_dir = NULL;
    }

    free(list);
    list = NULL;
}

// 加载目录内容
bool file_list_load_directory(FileList *list, const char *dir_path) {
    if (!list || !dir_path) {
        return false;
    }

    // 清空当前列表
    file_list_clear(list);

    // 设置当前目录
    if (list->current_dir) {
        free(list->current_dir);
        list->current_dir = NULL;
    }
    list->current_dir = strdup(dir_path);

    // 打开目录
    DIR *dir = fs_open_directory(dir_path);
    if (!dir) {
        return false;
    }

    // 读取目录内容
    struct dirent *entry;
    while ((entry = fs_read_directory(dir)) != NULL) {
        // 跳过 . 和 ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // 构建完整路径
        char *full_path = fs_combine_path(dir_path, entry->d_name);
        if (!full_path) {
            continue;
        }

        // 创建文件项
        FileItem *item = file_item_new(full_path);
        free(full_path); // 释放临时路径

        if (item) {
            // 添加到列表
            file_list_add_item(list, item);
        }
    }

    // 关闭目录
    fs_close_directory(dir);

    return true;
}

// 添加文件项到列表
void file_list_add_item(FileList *list, FileItem *item) {
    if (!list || !item) {
        return;
    }

    // 添加到链表尾部
    if (list->tail) {
        list->tail->next = item;
    } else {
        list->head = item;
    }

    list->tail = item;
    list->count++;
}

// 清空文件列表
void file_list_clear(FileList *list) {
    if (!list) {
        return;
    }

    FileItem *current = list->head;
    while (current) {
        FileItem *next = current->next;
        file_item_free(current);
        current = next;
    }

    list->head = NULL;
    list->tail = NULL;
    list->count = 0;
}

// 获取文件类型
FileType get_file_type(const struct stat *st) {
    if (!st) {
        return FILE_TYPE_UNKNOWN;
    }

    // if (S_ISREG(st->st_mode)) {
    //     return FILE_TYPE_REGULAR;
    // } else if (S_ISDIR(st->st_mode)) {
    //     return FILE_TYPE_DIRECTORY;
    // } else if (S_ISLNK(st->st_mode)) {
    //     return FILE_TYPE_SYMLINK;
    // } else if (S_ISCHR(st->st_mode) || S_ISBLK(st->st_mode)) {
    //     return FILE_TYPE_DEVICE;
    // } else if (S_ISFIFO(st->st_mode)) {
    //     return FILE_TYPE_PIPE;
    // } else if (S_ISSOCK(st->st_mode)) {
    //     return FILE_TYPE_SOCKET;
    // }

    return FILE_TYPE_UNKNOWN;
}

// 获取文件大小的可读字符串
const char* get_size_string(size_t size, char *buffer, size_t buffer_size) {
    const char *units[] = {"B", "KB", "MB", "GB", "TB"};
    int unit_index = 0;
    double size_value = (double)size;

    // 转换为合适的单位
    while (size_value >= 1024.0 && unit_index < 4) {
        size_value /= 1024.0;
        unit_index++;
    }

    // 格式化字符串
    if (unit_index == 0) {
        snprintf(buffer, buffer_size, "%d %s", (int)size_value, units[unit_index]);
    } else {
        snprintf(buffer, buffer_size, "%.2f %s", size_value, units[unit_index]);
    }

    return buffer;
}

// 获取时间的可读字符串
const char* get_time_string(time_t time_value, char *buffer, size_t buffer_size) {
    struct tm *tm_info = localtime(&time_value);
    strftime(buffer, buffer_size, "%Y-%m-%d %H:%M:%S", tm_info);
    return buffer;
}
