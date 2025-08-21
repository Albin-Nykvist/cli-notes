
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <direct.h>
#include <io.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>

#define FILE_PATH_LENGTH 60

#define BUFFER_SIZE 1024 // used for exporting to textfile

const char* NEW_NOTES_FOLDER = "../note_data/new_notes";
const char* COLLECTIONS_FOLDER = "../note_data/collections";


// TUI input
#define MAX_INPUT_LENGTH 100

// IO
void getInput(char* buffer, int size) {
    strcpy(buffer, "");
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

// FILE
const char* getFileNameFromPath(const char *path) {
    const char *filename = strrchr(path, '/');
    if(filename) {
        return filename + 1;
    }
    return path; 
}

// ARG
bool hasFlag(char* flag, char ** argv, int argc) {
    if(argc < 2) return false;

    for(int i=1; i<argc; i++) {
        if(strcmp(argv[i], flag) == 0) {
            return true;
        }
    }
    return false;
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

// FILE
bool isDirectory(struct _finddata_t fileinfo) {
    return fileinfo.attrib & _A_SUBDIR && strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0;
}

// FILE
bool ensureDirectoryExists(const char * directoryPath) {
    if(_access(directoryPath, 0) == -1) {
        if(_mkdir(directoryPath) == -1) {
            perror("failed to create folder\n");
            return false;
        }
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

// FILE
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

// FILE
char * readFileToString(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
        return NULL;
    }

    // Move to the end of the file to get its size
    if (fseek(file, 0, SEEK_END) != 0) {
        perror("Failed to seek to end of file");
        fclose(file);
        return NULL;
    }

    // Get the size of the file
    long fileSize = ftell(file);
    if (fileSize == -1L) {
        perror("Failed to get file size");
        fclose(file);
        return NULL;
    }

    // Reset the file position to the beginning
    rewind(file);

    // Allocate memory for the file contents + null terminator
    char *buffer = (char *)malloc(fileSize + 1);
    if (!buffer) {
        perror("Failed to allocate memory");
        fclose(file);
        return NULL;
    }

    // Read the file contents into the buffer
    size_t bytesRead = fread(buffer, 1, fileSize, file);
    if (bytesRead < (size_t)fileSize) {
        if (ferror(file)) {
            perror("Error reading file");
            free(buffer);
            fclose(file);
            return NULL;
        }
    }

    // Null-terminate the buffer
    buffer[bytesRead] = '\0';

    // Close the file and return the buffer
    fclose(file);
    return buffer;
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

// UTIL/string
int stringToInteger(const char * string) {
    char *endptr;
    int integer = strtol(string, &endptr, 10); // (base 10)

    if(*endptr != '\0' && *endptr != '\n') {
        return -1;
    }
    return integer;
}

// UTIL/string
int extractFirstInteger(const char * string) {
    while (*string && !isdigit(*string)) {
        string++;
    }

    if (*string == '\0') {
        return -1;
    }

    char *endptr;
    int integer = strtol(string, &endptr, 10);

    return integer; 
}

// CORE
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

// CORE
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

int deleteCollection(const char * collectionName) {
    char path[FILE_PATH_LENGTH] = ""; 
    snprintf(path, sizeof(path), "%s/%s", COLLECTIONS_FOLDER, collectionName);
    return removeDirectory(path);
}

// used by editcollection and viewcollections
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

void printViewCollectionsPrompt(struct _finddata_t * collections, int numCollections) {
    if(numCollections <= 0) {
        printf("no collections found\n");
        printf("[create: c + <collection name>]>");
    }else {
        for(int i=0; i<numCollections; i++) {
            printf("[%d] %s (#%d)\n", i+1, collections[i].name, getNumNotes(collections[i].name));
        }
        printf("[print: p + <number> | delete: d + <number> | export to textfile: e + <number> | create: c + <collection name>]>");
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

    // print prompt to user
    printViewCollectionsPrompt(collections, numCollections);

    // get input from user
    char input[MAX_INPUT_LENGTH];
    while(true) {
        getInput(input, MAX_INPUT_LENGTH);
        int size = sizeof(input) / sizeof(char);
        if(size < 2) {
            printf("invalid input\n>");
            continue;
        }

        int inputNumber = extractFirstInteger(input);   // this does not work for creating new collections
        if(input[0] != 'c') {
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
        default: // print
            printCollection(collections[inputNumber - 1].name);
            break;
        }
        updateCollections(&collections, &numCollections);
        printViewCollectionsPrompt(collections, numCollections);
    }

    free(collections);
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

/*
    for(int i=0; i<numCollections; i++) {
        int numNotes = getNumNotes(collections[i].name);
        struct _finddata_t * notes = (struct _finddata_t *)malloc(numNotes * sizeof(struct _finddata_t)); 
        getNotes(notes, numNotes, collections[i].name);
        for(int i=0; i<numNotes; i++) {
            printf("Note #1\n");
            printFile(notes[i].name);
            printf("\n");
        }
    }
*/
}

int main(int argc, char** argv) {

    if(argc < 2) {
        printf("Usage: %s <text>\n", argv[0]);
        return 1;
    }

    if(hasFlag("-r", argv, argc) || hasFlag("--review", argv, argc)) { // review
        reviewNewNotes();
    }else if(hasFlag("-v", argv, argc) || hasFlag("--view", argv, argc)) { // view collections
        viewCollections();
    }else { // default: create note in the "new_notes" folder

        // remove first argument
        int numWords = argc - 1;
        char** words = argv + 1;
        createNewNote(words, numWords);
    }

    return 0;
}