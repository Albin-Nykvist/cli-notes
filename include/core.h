
#ifndef CORE_H
#define CORE_H

#include <dirent.h>

void saveNoteToCollection(char* filePath);
void reviewNewNotes();
void printViewCollectionsPrompt(struct _finddata_t * collections, int numCollections);
void updateCollections(struct _finddata_t ** collections, int * numCollections);
int exportCollection(const char *collectionName, const char *exportFile);
void viewCollections();

#endif