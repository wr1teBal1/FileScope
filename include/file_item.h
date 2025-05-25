#ifndef FILE_ITEM_H
#define FILE_ITEM_H

#include "main.h"
#include <stdbool.h>
#include <time.h>
#include <sys/stat.h>

// 文件类型枚举
typedef enum {
    FILE_TYPE_UNKNOWN,
    FILE_TYPE_REGULAR,
    FILE_TYPE_DIRECTORY,
    FILE_TYPE_SYMLINK,
    FILE_TYPE_DEVICE,
    FILE_TYPE_PIPE,
    FILE_TYPE_SOCKET
} FileType;

// 文件项数据结构
typedef struct FileItem {
    char *name;              // 文件名
    char *path;              // 完整路径
    char *display_name;      // 显示名称
    FileType type;           // 文件类型
    size_t size;             // 文件大小
    time_t modified_time;    // 修改时间
    time_t created_time;     // 创建时间
    time_t accessed_time;    // 访问时间
    bool is_hidden;          // 是否隐藏
    bool is_selected;        // 是否选中
    SDL_Texture *icon;       // 文件图标
    struct FileItem *next;   // 链表下一项
} FileItem;

// 文件列表数据结构
typedef struct {
    FileItem *head;          // 链表头
    FileItem *tail;          // 链表尾
    int count;               // 文件数量
    char *current_dir;       // 当前目录
} FileList;

// 创建文件项
FileItem* file_item_new(const char *path);

// 释放文件项
void file_item_free(FileItem *item);

// 创建文件列表
FileList* file_list_new(void);

// 释放文件列表
void file_list_free(FileList *list);

// 加载目录内容
bool file_list_load_directory(FileList *list, const char *dir_path);

// 添加文件项到列表
void file_list_add_item(FileList *list, FileItem *item);

// 清空文件列表
void file_list_clear(FileList *list);

// 获取文件类型
FileType get_file_type(const struct stat *st);

// 获取文件大小的可读字符串
const char* get_size_string(size_t size, char *buffer, size_t buffer_size);

// 获取时间的可读字符串
const char* get_time_string(time_t time, char *buffer, size_t buffer_size);

#endif // FILE_ITEM_H