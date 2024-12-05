
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

void getFileName(char* buffer, char** words, int size) {
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
            strcat(buffer, ending);
            return;
        }
        strcat(buffer, words[i]);

        // If this is not the last word, add a space.
        if(i < num_words_in_filename) {
            strcat(buffer, " ");
        }
    }

    strcat(buffer, ending);
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

bool isValidCollectionName(char* collectionName) {
    int size = sizeof(collectionName) / sizeof(char);
    if(size <= 0) return false;
    if(!isalpha(collectionName[0])) return false;
    for(int i=1; i<size-1; i++) { // last char should always be '\0'
        if(!isalnum(collectionName[i]) && collectionName[i] != '_' && collectionName[i] != '-') return false;
    }
    return true;
}

bool isDirectory(struct _finddata_t fileinfo) {
    return fileinfo.attrib & _A_SUBDIR;
}

void createNewNote(char** words, int size) {
    // if there is none, create a directory for new notes
    if(_access(NEW_NOTES_FOLDER, 0) == -1) {
        if(_mkdir(NEW_NOTES_FOLDER) == -1) {
            perror("failed to create new notes folder\n");
        }
    }

    // create notefile and save it
    char filename[FILE_PATH_LENGTH];
    getFileName(filename, words, size);

    FILE *file;

    file = fopen(filename, "w");

    for (int i = 0; i < size; i++) {
        fprintf(file, words[i]);
        fprintf(file, " ");
    }

    fclose(file);
}

void createCollection() {

}

void noCollectionsFound() {
    printf("no collections found\n");
    printf("[create <new collection name>]>");

    // create new collection
    char buffer[MAX_INPUT_LENGTH] = "";
    while(!isValidCollectionName(buffer)) {
        getInput(buffer, MAX_INPUT_LENGTH);
        if(!isValidCollectionName(buffer)) {
            printf("invalid collection name\n>");
        }else {
            char path[FILE_PATH_LENGTH] = "";
            strcat(path, COLLECTIONS_FOLDER);
            strcat(path, "/");
            strcat(path, buffer);
            if(_access(path, 0) == -1) {
                if(_mkdir(path) == -1) {
                    perror("failed to create collections folder\n");
                }
            }else {
                printf("collection already exists\n");
                strcpy(buffer, ""); // empty the buffer to make it invalid
            }
        }
    }

}

bool printCollections() {
    if(_access(COLLECTIONS_FOLDER, 0) == -1) {
        // collection folder not found, create one and create new collection
        if(_mkdir(COLLECTIONS_FOLDER) == -1) {
            perror("failed to create collections folder\n");
        }else {
            return false;
        }
    }else {
        // print collections
        struct _finddata_t fileinfo; // Struct to store file information
        intptr_t handle;             // Handle for directory searching
        char path[FILE_PATH_LENGTH] = ""; 
        strcat(path, COLLECTIONS_FOLDER);
        strcat(path, "/*");

        // Start searching for files
        handle = _findfirst(path, &fileinfo);
        if(handle == -1) {
            //printf("No files found in the directory.\n");
            return false;
        } 

        int i = 1;
        do{
            if(isDirectory(fileinfo) && strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0) {
                printf("[%d] %s\n", i, fileinfo.name);
                i++;
            }
        }while(_findnext(handle, &fileinfo) == 0);

        // No collections found
        if(i == 1) {
            return false;
        }
        return true;

    }
}

const char *get_filename(const char *path) {
    const char *filename = strrchr(path, '/'); // Find the last '/' in the path
    if (filename) {
        return filename + 1; // Move past the '/'
    }
    return path; // If no '/' found, return the original path
}

void saveNoteToCollection(char* filePath) {

    // create collection folder if there is none
    if(_access(COLLECTIONS_FOLDER, 0) == -1) {
        if(_mkdir(COLLECTIONS_FOLDER) == -1) {
            perror("failed to create collections folder\n");
        }
    }

    struct _finddata_t fileinfo; 
    intptr_t handle;             
    char path[FILE_PATH_LENGTH] = ""; 
    strcat(path, COLLECTIONS_FOLDER);
    strcat(path, "/*");

    handle = _findfirst(path, &fileinfo);
    if(handle == -1) {
        printf("No files found in the directory.\n");
    } 

    int i = 1;
    do{
        if(isDirectory(fileinfo) && strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0) {
            printf("[%d] %s\n", i, fileinfo.name);
            i++;
        }
    }while(_findnext(handle, &fileinfo) == 0);

    int numCollections = i - 1;
    struct _finddata_t* collections = (struct _finddata_t*)malloc((i - 1) * sizeof(struct _finddata_t)); 
    handle = _findfirst(path, &fileinfo);

    handle = _findfirst(path, &fileinfo);
    if(handle == -1) {
        printf("No files found in the directory.\n");
    } 

    i = 0;
    do{
        if(isDirectory(fileinfo) && strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0) {
            collections[i] = fileinfo;
            i++;
        }
    }while(_findnext(handle, &fileinfo) == 0);

    if(i == 0) {
        printf("no collections found\n");
        printf("[create <new collection name>]>");
    }else {
        printf("[select: <number> | create <new collection name>]>");
    }


    // prompt user:
    char input[MAX_INPUT_LENGTH];
    while(true) {
        getInput(input, MAX_INPUT_LENGTH);
        int size = sizeof(input) / sizeof(char);
        if(size < 2) {
            printf("invalid input\n>");
            continue;
        }

        if(isdigit(input[0])) {
            char *endptr;
            int inputNumber = strtol(input, &endptr, 10); // (base 10)

            if(*endptr == '\0' && inputNumber <= numCollections) {
                // save to collections[inputNumber - 1]
                char newPath[FILE_PATH_LENGTH] = "";
                strcat(newPath, COLLECTIONS_FOLDER); // path to collections directory
                strcat(newPath, "/");
                strcat(newPath, collections[inputNumber - 1].name);
                strcat(newPath, "/");
                strcat(newPath, get_filename(filePath));
                if(rename(filePath, newPath) != 0) {
                    perror("Error moving file");
                    printf("from: %s, to: %s\n", filePath, newPath);
                }
                return;
            
            }else {
                printf("invalid input\n>");
                continue;
            }
        }else {
            // treat it as a new collection
            if(isValidCollectionName(input)) {
                char newPath[FILE_PATH_LENGTH] = "";
                strcat(newPath, COLLECTIONS_FOLDER);
                strcat(newPath, "/");
                strcat(newPath, input);
                if(_access(newPath, 0) == -1) {
                    if(_mkdir(newPath) == -1) {
                        perror("failed to create collections folder\n");
                        continue;
                    }
                }else {
                    printf("collection already exists\n");
                    continue;
                }

                strcat(newPath, "/");
                strcat(newPath, get_filename(filePath));
                if(rename(filePath, newPath) != 0) {
                    perror("Error moving file");
                    printf("from: %s, to: %s\n", filePath, newPath);
                }
                return;


            }else {
                printf("invalid input\n>");
                continue;
            }

        }


    }
}

void reviewNewNotes() {
    struct _finddata_t fileinfo; // Struct to store file information
    intptr_t handle;             // Handle for directory searching
    char directoryPath[FILE_PATH_LENGTH] = ""; 
    strcat(directoryPath, NEW_NOTES_FOLDER);
    strcat(directoryPath, "/*");

    // Start searching for files
    handle = _findfirst(directoryPath, &fileinfo);
    if(handle == -1) {
        printf("No files found in the directory.\n");
        return;
    }

    // Iterate through all files in the directory
    int numNotes = 0;
    do{
        if(isNoteFile(fileinfo.name) && !isDirectory(fileinfo)) {

            // open the file and print its contents
            FILE *file;               
            char ch;                  
            char path[FILE_PATH_LENGTH] = "";
            strcat(path, NEW_NOTES_FOLDER);
            strcat(path, "/");
            strcat(path, fileinfo.name);

            file = fopen(path, "r");
            if(file == NULL) {
                perror("Error opening file");
                return;
            }

            while((ch = fgetc(file)) != EOF) {
                putchar(ch);
            }
            fclose(file);

            // prompt the user
            printf("\n[save: s | delete: d | ignore: enter]>");

            // get input
            char buffer[MAX_INPUT_LENGTH];
            getInput(buffer, MAX_INPUT_LENGTH);

            // save, delete or ignore based on input
            switch (buffer[0]) {
            case 's':
                saveNoteToCollection(path);
                printf("saved to ...\n");
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

    // Close the search handle
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