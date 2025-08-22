
#ifndef COLLECTION_H
#define COLLECTION_H

#include <dirent.h>

bool isNoteFile(char* filename);
void createNewNote(char** words, int size);
void getFileNameFromNote(char* buffer, char** words, int size);
bool isValidCollectionName(const char* collectionName);
bool createNewCollection(const char * collectionName);
int getNumCollections();
bool getCollections(struct _finddata_t ** collections, int numCollections);
int getNumNotes(const char * collectionName);
bool getNotes(struct _finddata_t ** notes, int numNotes, const char * collectionName);
int deleteCollection(const char * collectionName);
void printCollection(const char * collectionName);
bool moveNoteToCollection(const char * notePath, const char * collectionName);

#endif