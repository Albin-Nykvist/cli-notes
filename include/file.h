
#ifndef FILE_H
#define FILE_H

const char* getFileNameFromPath(const char *path);
bool printFile(const char * path);
char * readFileToString(const char *filename);
bool isDirectory(struct _finddata_t fileinfo);
bool ensureDirectoryExists(const char * directoryPath);
int removeDirectory(const char *path);

#endif