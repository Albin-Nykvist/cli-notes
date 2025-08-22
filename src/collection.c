
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#include "../include/file.h"
#include "../include/config.h"
#include "../include/collection.h"

// Assumes the max buffer size is FILE_PATH_LENGTH
// Note
void getFileNameFromNote(char* buffer, char** words, int size) {
    strcpy(buffer, "");
    if(size <= 0) return;

    strcat(buffer, NEW_NOTES_FOLDER);
    strcat(buffer, "/");

    int num_words_in_filename = 3;
    if(size < num_words_in_filename) {
        num_words_in_filename = size;
    }

    char* ending = "-.txt\0";

    for(int i=0; i<num_words_in_filename; i++) {
        if(strlen(buffer) + strlen(words[i]) + strlen(ending) + 1 > FILE_PATH_LENGTH) {

            // Cut off the word and concatinate onto the buffer.
            int sliceLength = FILE_PATH_LENGTH - strlen(buffer) - strlen(ending) - 1;
            char slice[FILE_PATH_LENGTH] = "";
            strncpy(slice, words[i], sliceLength);
            strcat(buffer, slice);
            break;
        }
        strcat(buffer, words[i]);

        // If this is not the last word, add a space.
        if(i < num_words_in_filename) {
            strcat(buffer, " ");
        }
    }

    strcat(buffer, ending);
}

// NOTE
bool isNoteFile(char* filename) {
    // TODO: also check if it is a .txt file
    return (strcmp(filename, ".") != 0 && strcmp(filename, "..") != 0);
}

// COLLECTION
bool isValidCollectionName(const char* collectionName) {
    int size = sizeof(collectionName) / sizeof(char);
    if(size <= 0) return false;
    if(!isalpha(collectionName[0])) return false;
    for(int i=1; i<size-1; i++) { // last char should always be '\0'
        if(!isalnum(collectionName[i]) && collectionName[i] != '_' && collectionName[i] != '-') return false;
    }
    return true;
}

// COLLECTION
bool createNewCollection(const char * collectionName) {
    if(!ensureDirectoryExists(COLLECTIONS_FOLDER)) {
        printf("failed to create collections folder, attempted path: %s\n", COLLECTIONS_FOLDER);
        return false;
    }
    if(!isValidCollectionName(collectionName)) {
        printf("invalid collection name: %s\n", collectionName);
        return false;
    }
    char path[FILE_PATH_LENGTH] = "";
    snprintf(path, sizeof(path), "%s/%s", COLLECTIONS_FOLDER, collectionName);

    if(_access(path, 0) == 0) {
        printf("collection already exists\n");
        return false;
    }

    if(_mkdir(path) == -1) {
        printf("failed to create new collection folder\n");
        return false;
    }
    return true;
}

// NOTE
void createNewNote(char** words, int size) {
    ensureDirectoryExists(NEW_NOTES_FOLDER);

    // create notefile and save it
    char filename[FILE_PATH_LENGTH];
    getFileNameFromNote(filename, words, size);

    FILE *file;
    file = fopen(filename, "w");

    for (int i = 0; i < size; i++) {
        fprintf(file, words[i]);
        fprintf(file, " ");
    }

    fclose(file);
}

// COLLECTIONS
int getNumCollections() {
    if(!ensureDirectoryExists(COLLECTIONS_FOLDER)) {
        return -1;
    }

    struct _finddata_t fileinfo; 
    intptr_t handle;             
    char path[FILE_PATH_LENGTH] = ""; 
    snprintf(path, sizeof(path), "%s/*", COLLECTIONS_FOLDER);

    handle = _findfirst(path, &fileinfo);
    if(handle == -1) {
        printf("No files found in the directory.\n");
        return -1;
    } 

    int numCollections = 0;
    do{
        if(isDirectory(fileinfo)) {
            numCollections++;
        }
    }while(_findnext(handle, &fileinfo) == 0);

    return numCollections;
}

// COLLECTIONS
bool moveNoteToCollection(const char * notePath, const char * collectionName) {
    char destinationPath[FILE_PATH_LENGTH] = "";
    snprintf(destinationPath, sizeof(destinationPath), "%s/%s/%s", COLLECTIONS_FOLDER, collectionName, getFileNameFromPath(notePath));

    if(rename(notePath, destinationPath) != 0) {
        perror("Error moving file");
        printf("from: %s, to: %s\n", notePath, destinationPath);
        return false;
    }
    return true;
}

// Caller responsible for allocating collections and sending number of collections
// COLLECTIONS
bool getCollections(struct _finddata_t ** collections, int numCollections) {

    if(numCollections <= 0) return true;

    if(!ensureDirectoryExists(COLLECTIONS_FOLDER)) {
        return false;
    }

    struct _finddata_t fileinfo; 
    intptr_t handle;             
    char path[FILE_PATH_LENGTH] = ""; 
    snprintf(path, sizeof(path), "%s/*", COLLECTIONS_FOLDER);

    handle = _findfirst(path, &fileinfo);
    if(handle == -1) {
        printf("No files found in the directory.\n");
    } 

    int i = 0;
    do{
        if(isDirectory(fileinfo) && i < numCollections) {
            (*collections)[i] = fileinfo;
            i++;
        }
    }while(_findnext(handle, &fileinfo) == 0);

    if(i != numCollections) {
        return false;
    }

    return true;
}

// COLLECTIONS
int getNumNotes(const char * collectionName) {
    char path[FILE_PATH_LENGTH] = "";
    snprintf(path, sizeof(path), "%s/%s/*", COLLECTIONS_FOLDER, collectionName);

    if(!ensureDirectoryExists(COLLECTIONS_FOLDER)) {
        return -1;
    }

    struct _finddata_t fileinfo; 
    intptr_t handle;             
    handle = _findfirst(path, &fileinfo);
    if(handle == -1) {
        printf("No files found in the directory.\n");
        return -1;
    }

    int numNotes = 0;
    do {
        if(isNoteFile(fileinfo.name) && !isDirectory(fileinfo)) {
            numNotes++;
        }

    }while(_findnext(handle, &fileinfo) == 0);

    return numNotes;
}

// Caller responsible for allocating notes and sending number of notes: use getNumNotes before calling this function
// COLLECTIONS
bool getNotes(struct _finddata_t ** notes, int numNotes, const char * collectionName) {

    if(numNotes <= 0) return true;

    if(!ensureDirectoryExists(COLLECTIONS_FOLDER)) {
        return false;
    }

    struct _finddata_t fileinfo; 
    intptr_t handle;             
    char path[FILE_PATH_LENGTH] = ""; 
    snprintf(path, sizeof(path), "%s/%s/*", COLLECTIONS_FOLDER, collectionName);

    handle = _findfirst(path, &fileinfo);
    if(handle == -1) {
        printf("No files found in the directory.\n");
    } 

    int i = 0;
    do{
        if(isNoteFile(fileinfo.name) && !isDirectory(fileinfo) && i < numNotes) {
            (*notes)[i] = fileinfo;
            i++;
        }
    }while(_findnext(handle, &fileinfo) == 0);

    if(i != numNotes) {
        return false;
    }

    return true;
}

// COLLECTION
int deleteCollection(const char * collectionName) {
    char path[FILE_PATH_LENGTH] = ""; 
    snprintf(path, sizeof(path), "%s/%s", COLLECTIONS_FOLDER, collectionName);
    return removeDirectory(path);
}

// used by editcollection and viewcollections
// COLLECTION
void printCollection(const char * collectionName) {
    int numNotes = getNumNotes(collectionName);
    struct _finddata_t * notes = (struct _finddata_t *)malloc(numNotes * sizeof(struct _finddata_t)); 
    getNotes(&notes, numNotes, collectionName);
    printf("%s (#%d):\n\n", collectionName, numNotes);
    for(int i=0; i<numNotes; i++) {
        printf("Note #%d\n", i+1);
        char path[FILE_PATH_LENGTH];
        snprintf(path, sizeof(path), "%s/%s/%s", COLLECTIONS_FOLDER, collectionName, notes[i].name);
        printFile(path);
        printf("\n\n");
    }

    free(notes);
}

