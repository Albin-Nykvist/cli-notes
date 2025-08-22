

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <direct.h>
#include <io.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>

#include "../include/util.h"
#include "../include/file.h"
#include "../include/io.h"
#include "../include/config.h"
#include "../include/collection.h"
#include "../include/core.h"

void saveNoteToCollection(char* filePath) {
    int numCollections = getNumCollections();
    struct _finddata_t * collections = (struct _finddata_t *)malloc(numCollections * sizeof(struct _finddata_t)); 
    if(!getCollections(&collections, numCollections)) {
        printf("failed to get collections\n");
    }

    // print prompt to user
    if(numCollections <= 0) {
        printf("no collections found\n");
        printf("[create <new collection name>]>");
    }else {
        for(int i=0; i<numCollections; i++) {
            printf("[%d] %s (#%d)\n", i+1, collections[i].name, getNumNotes(collections[i].name));
        }
        printf("[select: <number> | create <new collection name>]>");
    }

    // get input from user
    char input[MAX_INPUT_LENGTH];
    while(true) {
        getInput(input, MAX_INPUT_LENGTH);
        int size = sizeof(input) / sizeof(char);
        if(size < 2) {
            printf("invalid input\n>");
            continue;
        }

        if(isdigit(input[0])) { // User attempts to choose a listed collection

            int inputNumber = stringToInteger(input);
            if(inputNumber == -1 || inputNumber > numCollections) {
                printf("invalid input\n>");
                continue;
            }

            if(!moveNoteToCollection(filePath, collections[inputNumber - 1].name)) {
                printf("failed to save note to collection\n>");
                continue;
            }

            break;

        }else { // User attempts to create and save the note to a new collection

            if(!createNewCollection(input)) {
                printf("failed to create new collection\n>");
                continue;
            }

            if(!moveNoteToCollection(filePath, input)) {
                printf("failed to save note to new collection\n>");
                continue;
            }

            break;
        }
    }

    free(collections);
}

void reviewNewNotes() {
    char directoryPath[FILE_PATH_LENGTH] = ""; 
    snprintf(directoryPath, sizeof(directoryPath), "%s/*", NEW_NOTES_FOLDER);

    if(!ensureDirectoryExists(NEW_NOTES_FOLDER)) {
        printf("failed to create new notes folder\n");
        return;
    }

    struct _finddata_t fileinfo; 
    intptr_t handle;             
    handle = _findfirst(directoryPath, &fileinfo);
    if(handle == -1) {
        printf("No files found in the directory.\n");
        return;
    }

    int numNotes = 0;
    do{
        if(isNoteFile(fileinfo.name) && !isDirectory(fileinfo)) {

            char path[FILE_PATH_LENGTH] = "";
            snprintf(path, sizeof(path), "%s/%s", NEW_NOTES_FOLDER, fileinfo.name);

            // print note and prompt the user
            printFile(path);
            printf("\n[save: s | delete: d | ignore: enter]>");

            // get input
            char buffer[MAX_INPUT_LENGTH];
            getInput(buffer, MAX_INPUT_LENGTH);

            // save, delete or ignore based on input
            switch (buffer[0]) {
            case 's':
                saveNoteToCollection(path);
                printf("saved\n");
                break;
            case 'd':
                if(remove(path) == -1) {
                    perror("Error: could not delete file\n");
                }else {
                    printf("deleted\n");
                }
                break;
            default:
                printf("ignored\n");
                break;
            }

            printf("\n");

            numNotes++;
        }
    } while (_findnext(handle, &fileinfo) == 0);

    _findclose(handle);

    if(numNotes <= 0) {
        printf("No new notes to review.\n");
    }
}

void printViewCollectionsPrompt(struct _finddata_t * collections, int numCollections) {
    if(numCollections <= 0) {
        printf("no collections found\n");
        printf("[create: c + <collection name>]>");
    }else {
        for(int i=0; i<numCollections; i++) {
            printf("[%d] %s (#%d)\n", i+1, collections[i].name, getNumNotes(collections[i].name));
        }
        printf("[print: p + <number> | delete: d + <number> | export to text file: e + <number> | create: c + <collection name> | quit: q]>");
    }
}

void updateCollections(struct _finddata_t ** collections, int * numCollections) {
    *numCollections = getNumCollections();

    if (*collections) {
        free(*collections);
        *collections = NULL;
    }

    *collections = malloc(*numCollections * sizeof(struct _finddata_t));
    if (!*collections) {
        perror("malloc failed");
        *numCollections = 0;
        return;
    }

    if (!getCollections(collections, *numCollections)) {
        printf("failed to get collections\n");
    }
}

int exportCollection(const char *collectionName, const char *exportFile) {
    FILE *out = fopen(exportFile, "w");
    if (!out) {
        perror("fopen exportFile");
        return -1;
    }

    // Build search pattern for files in collection
    char collectionPath[FILE_PATH_LENGTH] = "";
    snprintf(collectionPath, sizeof(collectionPath), "%s/%s", COLLECTIONS_FOLDER, collectionName);
    char searchPath[260];
    snprintf(searchPath, sizeof(searchPath), "%s\\*.*", collectionPath);

    struct _finddata_t fileinfo;
    intptr_t hFile = _findfirst(searchPath, &fileinfo);
    if (hFile == -1L) {
        perror("findfirst");
        fclose(out);
        return -1;
    }

    fprintf(out, "==== %s ====\n\n", collectionName);

    do {
        if (fileinfo.attrib & _A_SUBDIR) continue;

        char filepath[260];
        snprintf(filepath, sizeof(filepath), "%s\\%s", collectionPath, fileinfo.name);

        FILE *in = fopen(filepath, "r");
        if (!in) {
            fprintf(out, "Failed to open note: %s\n", fileinfo.name);
            continue;
        }

        char buffer[BUFFER_SIZE];
        size_t n;
        while ((n = fread(buffer, 1, sizeof(buffer), in)) > 0) {
            fwrite(buffer, 1, n, out);
        }
        fprintf(out, "\n\n");

        fclose(in);

    } while (_findnext(hFile, &fileinfo) == 0);

    _findclose(hFile);
    fclose(out);

    return 0;
}

// Print the collections list and ask what the user wants to do with a given collection.
// Note Collections:
// [1] Chemistry, 32 notes
// [2] Ethics, 12 notes
// [3] Computer security, 4 notes
// [print: p + <number> | delete: d + <number> | edit: e + <number> | save as textfile: s + <number> | create new collection: <new collection name>]
void viewCollections() {
    int numCollections = getNumCollections();
    struct _finddata_t * collections = (struct _finddata_t *)malloc(numCollections * sizeof(struct _finddata_t)); 
    if(!getCollections(&collections, numCollections)) {
        printf("failed to get collections\n");
    }

    printViewCollectionsPrompt(collections, numCollections);

    char input[MAX_INPUT_LENGTH];
    while(true) {
        getInput(input, MAX_INPUT_LENGTH);
        int size = sizeof(input) / sizeof(char);
        if(size <= 0) {
            printf("invalid input\n>");
            continue;
        }
        if(input[0] != 'q' && size < 2) {
            printf("invalid input\n>");
            continue;
        }

        int inputNumber = extractFirstInteger(input);   
        if(input[0] != 'c' && input[0] != 'q') {
            if(inputNumber <= 0 || inputNumber > numCollections) {
                printf("invalid input\n>");
                continue;
            }
        }

        switch(input[0])
        {
        case 'p':
            printf("printing...\n");
            printCollection(collections[inputNumber - 1].name);
            break;
        case 'd':
            printf("delting...\n");
            if (deleteCollection(collections[inputNumber - 1].name) == 0) {
                printf("Deleted collection.\n");
            } else {
                printf("Failed to delete collection.\n");
            }
            break;
        case 'e':
            printf("exporting...\n");
            if(exportCollection(collections[inputNumber - 1].name, "export.txt") == 0) {
                 printf("Export successful!\n");
            } else {
                printf("Export failed.\n");
            }
            break;
        case 'c':
            printf("creating...\n");
            char *ptr = input + 1;
            while (*ptr == ' ' || *ptr == '\t')
                ptr++;

            char newName[FILE_PATH_LENGTH];
            strncpy(newName, ptr, FILE_PATH_LENGTH - 1);
            newName[FILE_PATH_LENGTH - 1] = '\0'; 
            if(!createNewCollection(newName)) {
                printf("failed to create new collection\n>");
                continue;
            }
            break;
        case 'q':
            printf("quitting...\n");
            exit(0);  
            break;
        default: 
            printCollection(collections[inputNumber - 1].name);
            break;
        }
        updateCollections(&collections, &numCollections);
        printViewCollectionsPrompt(collections, numCollections);
    }

    free(collections);
}
