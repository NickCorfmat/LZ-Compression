#include <stdint.h>
#include <stdlib.h>

#include "trie.h"
#include "code.h"

TrieNode *trie_node_create(uint16_t index) {

    // ALLOCATE memory for the TrieNode.
    TrieNode *node = (TrieNode *) malloc(sizeof(TrieNode));

    if (node == NULL) {
        return NULL;
    }

    node->code = index;

    // ENSURE each of the children node pointers are NULL.
    for (uint16_t loc = 0; loc < ALPHABET; loc += 1) {
        node->children[loc] = NULL;
    }

    return node;
}

void trie_node_delete(TrieNode *n) {
    // DEREFERENCES TrieNode n
    free(n);
    n = NULL;
}

TrieNode *trie_create(void) {
    // INITIALIZES a root TrieNode with the code EMPTY_CODE, returning the root if successful, NULL otherwise.
    TrieNode *root = trie_node_create(EMPTY_CODE);

    if (root == NULL) {
        return NULL;
    }

    return root;
}

void trie_reset(TrieNode *root) {
    // RESETS a trie to just the root TrieNode. ENSURE each of root's children nodes are NULL.
    for (uint16_t loc = 0; loc < ALPHABET; loc += 1) {
        trie_delete(root->children[loc]);

        root->children[loc] = NULL;
    }
}

void trie_delete(TrieNode *n) {
    if (n == NULL) {
        return;
    }

    // DELETES a sub-trie starting from the trie rooted at node n.
    for (uint16_t loc = 0; loc < ALPHABET; loc += 1) {
        if (n->children[loc] != NULL) {
            trie_delete(n->children[loc]);
        }
    }

    trie_node_delete(n);
    n = NULL;
}

TrieNode *trie_step(TrieNode *n, uint8_t sym) {
    // RETURN a pointer to the child node representing the symbol 'sym' or NULL if it doesn't exist.
    if (n->children[sym] == NULL) {
        return NULL;
    }

    return n->children[sym];
}
