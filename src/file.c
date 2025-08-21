

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <direct.h>
#include <dirent.h>
#include <sys/stat.h>

#include "../include/file.h"

const char* getFileNameFromPath(const char *path) {
    const char *filename = strrchr(path, '/');
    if(filename) {
        return filename + 1;
    }
    return path; 
}

bool isDirectory(struct _finddata_t fileinfo) {
    return fileinfo.attrib & _A_SUBDIR && strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0;
}

bool ensureDirectoryExists(const char * directoryPath) {
    if(_access(directoryPath, 0) == -1) {
        if(_mkdir(directoryPath) == -1) {
            perror("failed to create folder\n");
            return false;
        }
    }
    return true;
}

bool printFile(const char * path) {
    FILE *file;               
    char ch;                  

    file = fopen(path, "r");
    if(file == NULL) {
        perror("Error opening file");
        return false;
    }

    while((ch = fgetc(file)) != EOF) {
        putchar(ch);
    }
    fclose(file);
    return true;
}

char * readFileToString(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
        return NULL;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        perror("Failed to seek to end of file");
        fclose(file);
        return NULL;
    }

    long fileSize = ftell(file);
    if (fileSize == -1L) {
        perror("Failed to get file size");
        fclose(file);
        return NULL;
    }

    rewind(file);

    char *buffer = (char *)malloc(fileSize + 1);
    if (!buffer) {
        perror("Failed to allocate memory");
        fclose(file);
        return NULL;
    }

    size_t bytesRead = fread(buffer, 1, fileSize, file);
    if (bytesRead < (size_t)fileSize) {
        if (ferror(file)) {
            perror("Error reading file");
            free(buffer);
            fclose(file);
            return NULL;
        }
    }

    buffer[bytesRead] = '\0';

    fclose(file);
    return buffer;
}

int removeDirectory(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir");
        return -1;
    }

    struct dirent *entry;
    char filepath[1024];

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        snprintf(filepath, sizeof(filepath), "%s/%s", path, entry->d_name);

        struct stat statbuf;
        if (stat(filepath, &statbuf) == 0) {
            if (S_ISDIR(statbuf.st_mode)) {
                removeDirectory(filepath);
            } else {
                if (remove(filepath) != 0) {
                    perror("remove file");
                }
            }
        }
    }
    closedir(dir);

    if (rmdir(path) != 0) {
        perror("rmdir");
        return -1;
    }

    return 0;
}
