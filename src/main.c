
#include <stdio.h>

#include "../include/io.h"
#include "../include/collection.h"
#include "../include/core.h"

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