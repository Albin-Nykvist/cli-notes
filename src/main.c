
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <direct.h>
#include <io.h>
#include <ctype.h>

#define FILE_PATH_LENGTH 60

const char* NEW_NOTES_FOLDER = "new_notes";
const char* COLLECTIONS_FOLDER = "collections";


// TUI input
#define MAX_INPUT_LENGTH 100

void getInput(char* buffer, int size) {
    if(fgets(buffer, size-1, stdin) == NULL) {
        printf("Error reading input\n");
        return;
    }

    if (buffer[strlen(buffer) - 1] != '\n') {
        char c;
        while ((c = getchar()) != '\n' && c != EOF);
    }

    buffer[strlen(buffer) - 1] = '\0';
}

// Assumes the max buffer size is FILE_PATH_LENGTH
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

const char* getFileNameFromPath(const char *path) {
    const char *filename = strrchr(path, '/');
    if(filename) {
        return filename + 1;
    }
    return path; 
}

bool hasFlag(char* flag, char ** argv, int argc) {
    if(argc < 2) return false;

    for(int i=1; i<argc; i++) {
        if(strcmp(argv[i], flag) == 0) {
            return true;
        }
    }
    return false;
}

bool isNoteFile(char* filename) {
    return (strcmp(filename, ".") != 0 && strcmp(filename, "..") != 0);
}

bool isValidCollectionName(const char* collectionName) {
    int size = sizeof(collectionName) / sizeof(char);
    if(size <= 0) return false;
    if(!isalpha(collectionName[0])) return false;
    for(int i=1; i<size-1; i++) { // last char should always be '\0'
        if(!isalnum(collectionName[i]) && collectionName[i] != '_' && collectionName[i] != '-') return false;
    }
    return true;
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

int getNumCollections() {
    ensureDirectoryExists(COLLECTIONS_FOLDER);
    struct _finddata_t fileinfo; 
    intptr_t handle;             
    char path[FILE_PATH_LENGTH] = ""; 
    snprintf(path, sizeof(path), "%s/*", COLLECTIONS_FOLDER);

    handle = _findfirst(path, &fileinfo);
    if(handle == -1) {
        printf("No files found in the directory.\n");
    } 

    int numCollections = 0;
    do{
        if(isDirectory(fileinfo)) {
            numCollections++;
        }
    }while(_findnext(handle, &fileinfo) == 0);

    return numCollections;
}

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
void getCollections(struct _finddata_t ** collections, int numCollections) {

    if(numCollections <= 0) return;

    ensureDirectoryExists(COLLECTIONS_FOLDER);

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
            *collections[i] = fileinfo;
            i++;
        }
    }while(_findnext(handle, &fileinfo) == 0);
}

void saveNoteToCollection(char* filePath) {
    int numCollections = getNumCollections();
    struct _finddata_t * collections = (struct _finddata_t *)malloc(numCollections * sizeof(struct _finddata_t)); 
    getCollections(&collections, numCollections);

    for(int i=0; i<numCollections; i++) {
        printf("[%d] %s\n", i+1, collections[i].name);
    }

    if(numCollections <= 0) {
        printf("no collections found\n");
        printf("[create <new collection name>]>");
    }else {
        printf("[select: <number> | create <new collection name>]>");
    }

    // get input
    char input[MAX_INPUT_LENGTH];
    while(true) {
        getInput(input, MAX_INPUT_LENGTH);
        int size = sizeof(input) / sizeof(char);
        if(size < 2) {
            printf("invalid input\n>");
            continue;
        }

        if(isdigit(input[0])) { // User attempts to choose a listed collection
            char *endptr;
            int inputNumber = strtol(input, &endptr, 10); // (base 10)

            if(*endptr == '\0' && inputNumber <= numCollections) {
                char newPath[FILE_PATH_LENGTH] = "";
                snprintf(newPath, sizeof(newPath), "%s/%s/%s", COLLECTIONS_FOLDER, collections[inputNumber - 1].name, getFileNameFromPath(filePath));

                if(rename(filePath, newPath) != 0) {
                    perror("Error moving file");
                    printf("from: %s, to: %s\n", filePath, newPath);
                }
                break;
            
            }else {
                printf("invalid input\n>");
                continue;
            }
        }else { // User attempts to create and save the note to a new collection

            if(!createNewCollection(input)) {
                printf("failed to create new collection\n>");
                continue;
            }

            if(!moveNoteToCollection(filePath, input)) {
                printf("failed to move note to new collection\n>");
                continue;
            }

            break;
        }
    }

    free(collections);
}

void reviewNewNotes() {
    struct _finddata_t fileinfo; 
    intptr_t handle;             
    char directoryPath[FILE_PATH_LENGTH] = ""; 
    snprintf(directoryPath, sizeof(directoryPath), "%s/*", NEW_NOTES_FOLDER);

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

// Print the collections list and ask what the user wants to do with a given collection.
// Note Collections:
// [1] Chemistry, 32 notes
// [2] Ethics, 12 notes
// [3] Computer security, 4 notes
// [print: p + <number> | delete: d + <number> | edit: e + <number> | save as textfile: s + <number>]
void viewCollections() {

}

// Print the whole collection like a text file, numbering all notes.
// Name: Chemistry
// Notes: 32
// 
// Note 1: 
// The exam requires you to write at least 40 answers to the questions.
//
// Note 2: 
// The exam requires you to write at least 40 answers to the questions.
//
// [rename: r + <new name> | delete note/s: d + <number>]
void editCollection() {

}

int main(int argc, char** argv) {

    if(argc < 2) {
        printf("Usage: %s <text>\n", argv[0]);
        return 1;
    }

    if(hasFlag("-r", argv, argc) || hasFlag("--review", argv, argc)) { // review
        reviewNewNotes();

    }else { // default: create note in the "new_notes" folder

        // remove first argument
        int numWords = argc - 1;
        char** words = argv + 1;
        createNewNote(words, numWords);
    }

    return 0;
}