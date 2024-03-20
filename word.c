#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "word.h"
#include "code.h"

Word *word_create(uint8_t *syms, uint32_t len) {

    // ALLOCATE memory for Word.
    Word *w = (Word *) malloc(sizeof(Word));

    if (w == NULL) {
        return NULL;
    }

    // ALLOCATE memory for syms array.
    uint8_t *sym_copy = malloc(sizeof(uint8_t));

    w->len = len;

    for (uint32_t loc = 0; loc < w->len; loc++) {
        sym_copy[loc] = syms[loc];
    }

    w->syms = sym_copy;

    return w;
}

// Copy the array and then append to the new one.

Word *word_append_sym(Word *w, uint8_t sym) {

    uint8_t *new_syms = (uint8_t *) calloc(1, sizeof(uint8_t));

    if (w == NULL) {
        new_syms[0] = sym;

        // CONSTRUCT a new Word from the specified Word, w, appended with a symbol, sym.
        Word *new_word = word_create(new_syms, 1);

        return new_word;
    }

    for (uint8_t loc = 0; loc < w->len; loc += 1) {
        new_syms[loc] = w->syms[loc];
    }

    // APPEND 'sym' to end of array.
    new_syms[w->len] = sym;

    Word *new_word = word_create(new_syms, w->len + 1);

    if (new_word == NULL) {
        return NULL;
    }

    return new_word;
}

void word_delete(Word *w) {
    free(w);

    w = NULL;
}

WordTable *wt_create(void) {

    // CREATES a new WordTable, initialized with an empty Word at index EMPTY_CODE.
    WordTable *wt = calloc(MAX_CODE, sizeof(WordTable));

    if (wt == NULL) {
        return NULL;
    }

    // ALLOCATE memory for Word.
    Word *first_sym = word_create(NULL, 0);

    wt[EMPTY_CODE] = first_sym;

    return wt;
}

void wt_reset(WordTable *wt) {

    // RESETS a WordTable, wt, to contain just the empty Word. ALL OTHER words in the table are NULL.
    for (uint16_t loc = START_CODE; loc < MAX_CODE; loc += 1) {

        // DELETE entries that consist of a word.
        if (wt[loc] != NULL) {
            word_delete(wt[loc]);
            wt[loc] = NULL;
        }
    }
}

void wt_delete(WordTable *wt) {
    free(wt);
    wt = NULL;
}
