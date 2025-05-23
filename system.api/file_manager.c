#include "file_manager.h"

// 创建文件,filname均为文件名称
int create_file(const char* filename) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        return -1; // 创建失败
    }
    fclose(file);
    return 0; // 创建成功
}
//打开文件
FILE* open_file(const char*filename,const char*mode){
    return fopen(filename,mode);
}

//关闭文件
int close_file(FILE*fp){
    fclose(fp);
    if(fp==NULL){
        return -1;

    }
}
// 写入文件
int write_to_file(const char* filename, const char* content) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        return -1; // 打开文件失败
    }
    
    if (fputs(content, file) == EOF) {
        fclose(file);
        return -2; // 写入失败
    }
    
    fclose(file);
    return 0; // 写入成功
}



// 删除文件
int delete_file(const char* filename) {
    if (remove(filename) != 0) {
        return -1; // 删除失败
    }
    return 0; // 删除成功
}

// 复制文件
int copy_file(const char* source, const char* destination) {
    FILE* src = fopen(source, "rb");
    if (src == NULL) {
        return -1; // 源文件打开失败
    }

    FILE* dest = fopen(destination, "wb");
    if (dest == NULL) {
        fclose(src);
        return -2; // 目标文件创建失败
    }

    char buffer[4096];
    size_t bytes_read;

    // 循环读取和写入数据
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        if (fwrite(buffer, 1, bytes_read, dest) != bytes_read) {
            fclose(src);
            fclose(dest);
            return -3; // 写入失败
        }
    }

    fclose(src);
    fclose(dest);
    return 0; // 复制成功
}

// 读取文件内容
char* read_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        return NULL; // 文件打开失败
    }

    // 获取文件大小
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    // 分配内存
    char* content = (char*)malloc(file_size + 1);
    if (content == NULL) {
        fclose(file);
        return NULL; // 内存分配失败
    }

    // 读取文件内容
    size_t read_size = fread(content, 1, file_size, file);
    content[read_size] = '\0'; // 添加字符串结束符

    fclose(file);
    return content;
} 