#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 创建文件
int create_file(const char* filename);

//打开文件
FILE* open_file(const char*filename,const char*mode);

//关闭文件
int close_file(FILE*fp);

// 写入文件
int write_to_file(const char* filename, const char* content);

// 删除文件
int delete_file(const char* filename);

// 复制文件
int copy_file(const char* source, const char* destination);

// 读取文件内容
char* read_file(const char* filename);

#endif // FILE_MANAGER_H 