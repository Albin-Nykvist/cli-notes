
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <direct.h>
#include <io.h>

#define FILE_PATH_LENGTH 50

const char* NEW_NOTES_FOLDER = "new_notes";


// TUI input
#define MAX_INPUT_LENGTH 100


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
    printf("%d\n", strlen(ending));

    for(int i=1; i<num_words_in_filename + 1; i++) {
        if(strlen(buffer) + strlen(words[i]) + strlen(ending) + 1 > FILE_PATH_LENGTH) {

            // Cut off the word and concatinate onto the buffer.
            int sliceLength = FILE_PATH_LENGTH - strlen(buffer) - strlen(ending) - 1;
            printf("%d\n", sliceLength);
            char slice[FILE_PATH_LENGTH] = "";
            strncpy(slice, words[i], sliceLength);
            strcat(buffer, slice);
            printf("%s\n", buffer);
            strcat(buffer, ending);
            printf("%s\n", buffer);
            return;
        }
        strcat(buffer, words[i]);
        printf("%s\n", buffer);

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

    for (int i = 1; i < size; i++) {
        fprintf(file, words[i]);
        fprintf(file, " ");
    }

    fclose(file);
}

// Print all notes one at a time with some kind of user input for each print.
// After a note has been printed, you may choose to delete it, leave it in new_notes or save it to a note collection.

// Note 1: 
// The exam requires you to write at least 40 answers to the questions.
// [save: s | delete: d | ignore: enter]

// When s is pressed a list of saved note collections show up:

// Save to Collection:
// [1] Chemistry, 32 notes
// [2] Ethics, 12 notes
// [3] Computer security, 4 notes
// [0] create new collection
// [select collection: <number>]
void reviewNewNotes() {
    struct _finddata_t fileinfo; // Struct to store file information
    intptr_t handle;             // Handle for directory searching
    char directoryPath[FILE_PATH_LENGTH] = ""; 
    strcat(directoryPath, NEW_NOTES_FOLDER);
    strcat(directoryPath, "/*");

    // Start searching for files
    handle = _findfirst(directoryPath, &fileinfo);
    if (handle == -1) {
        printf("No files found in the directory.\n");
        return;
    }

    // Iterate through all files in the directory
    do {
        if(isNoteFile(fileinfo.name)) {

            // open the file and print its contents
            FILE *file;               
            char ch;                  
            char path[FILE_PATH_LENGTH] = "";
            strcat(path, NEW_NOTES_FOLDER);
            strcat(path, "/");
            strcat(path, fileinfo.name);

            file = fopen(path, "r");
            if (file == NULL) {
                perror("Error opening file");
                return;
            }

            while ((ch = fgetc(file)) != EOF) {
                putchar(ch);
            }
            fclose(file);

            // prompt the user
            printf("\n[save: s | delete: d | ignore: enter]>");

            // get input
            char buffer[MAX_INPUT_LENGTH];
            if(fgets(buffer, MAX_INPUT_LENGTH-1, stdin) == NULL) {
                printf("Error reading input\n");
                return;
            }

            if (buffer[strlen(buffer) - 1] != '\n') {
                char c;
                while ((c = getchar()) != '\n' && c != EOF);
            }

            buffer[strlen(buffer) - 1] = '\0';

            // save, delete or ignore based on input
            switch (buffer[0]) {
            case 's':
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
                break;
            }

            printf("\n");
            

        }
    } while (_findnext(handle, &fileinfo) == 0);

    // Close the search handle
    _findclose(handle);
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
        createNewNote(argv, argc);
    }



    // pick a directory
    // Create a file to save the note


    return 0;
}