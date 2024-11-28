
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <getopt.h>

#define FILE_NAME_SIZE 30

void getFileName(char* buffer, char** words, int size) {
    strcpy(buffer, "");
    if(size <= 0) return;

    int num_words_in_filename = 3;
    if(size < num_words_in_filename) {
        num_words_in_filename = size;
    }

    char* ending = "-.txt\0";
    printf("%d\n", strlen(ending));

    for(int i=1; i<num_words_in_filename + 1; i++) {
        if(strlen(buffer) + strlen(words[i]) + strlen(ending) + 1 > FILE_NAME_SIZE) {

            // Cut off the word and concatinate onto the buffer.
            int sliceLength = FILE_NAME_SIZE - strlen(buffer) - strlen(ending) - 1;
            printf("%d\n", sliceLength);
            char slice[FILE_NAME_SIZE] = "";
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

void saveNote(char** words, int size) {

}

int main(int argc, char** argv) {

    if(argc < 2) {
        printf("Usage: %s <text>\n", argv[0]);
        return 1;
    }

    if(hasFlag("-r", argv, argc)) { // review

    }else {                         // default: create note
        char filename[FILE_NAME_SIZE];
        getFileName(filename, argv, argc);

        FILE *fptr;

        fptr = fopen(filename, "w");

        for (int i = 1; i < argc; i++) {
            fprintf(fptr, argv[i]);
            fprintf(fptr, " ");
        }

        fclose(fptr);
    }



    // pick a directory
    // Create a file to save the note


    return 0;
}