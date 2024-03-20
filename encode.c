#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "io.h"
#include "code.h"
#include "trie.h"
#include "word.h"

#define OPTIONS "vi:o:h"

// HELPER function for calculating bit length.

int bit_length(uint64_t bits) {
    return (int) log2(bits) + 1;
}

int main(int argc, char **argv) {

    int opt = 0;

    bool verbose_output = false;

    int infile = STDIN_FILENO;
    int outfile = STDOUT_FILENO;

    FileHeader header = { 0, 0 };

    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 'v': verbose_output = true; break;
        case 'i':
            infile = open(optarg, O_RDONLY);

            if (infile == -1) {
                fprintf(stderr, "Error: Encode could not access input file.\n");
                exit(1);
            }

            break;
        case 'o':
            outfile = open(optarg, O_CREAT | O_WRONLY | O_TRUNC);

            if (outfile == -1) {
                fprintf(stderr, "Error: Encode  could not access output file.\n");
                exit(1);
            }

            break;
        case 'h':
            printf("SYNOPSIS\n");
            printf("   Compresses files using the LZ78 compression algorithm.\n");
            printf("   Compressed files are decompressed with the corresponding decoder.\n\n");
            printf("USAGE\n");
            printf("   ./encode-test [-vh] [-i input] [-o output]\n\n");
            printf("OPTIONS\n");
            printf("   -v          Display compression statistics\n");
            printf("   -i input    Specify input to compress (stdin by default)\n");
            printf("   -o output   Specify output of compressed input (stdout by default)\n");
            printf("   -h          Display program help and usage\n");

            break;
        default:
            printf("SYNOPSIS\n");
            printf("   Compresses files using the LZ78 compression algorithm.\n");
            printf("   Compressed files are decompressed with the corresponding decoder.\n\n");
            printf("USAGE\n");
            printf("   ./encode-test [-vh] [-i input] [-o output]\n\n");
            printf("OPTIONS\n");
            printf("   -v          Display compression statistics\n");
            printf("   -i input    Specify input to compress (stdin by default)\n");
            printf("   -o output   Specify output of compressed input (stdout by default)\n");
            printf("   -h          Display program help and usage\n");

            return 0;
        }
    }

    // BEGIN code from Darrell Long.

    // SET the outfile file header.

    struct stat stats;
    fstat(infile, &stats);

    header.magic = MAGIC;
    header.protection = stats.st_mode;
    fchmod(outfile, header.protection);

    // END code from Darrell Long

    // WRITE the filled out file header to outfile.

    write_header(outfile, &header);

    // CREATE an empty parent trie and MAKE a copy.

    TrieNode *root = trie_create();
    TrieNode *curr_node = root;

    uint8_t curr_sym = 0;
    uint16_t prev_sym = 0;

    int next_code = START_CODE;

    // CREATE a variable to keep track of the previous trie node.

    TrieNode *prev_node = NULL;

    // LOOP to read in all symbols from infile.

    while (read_sym(infile, &curr_sym)) {
        TrieNode *next_node = trie_step(curr_node, curr_sym);

        if (next_node != NULL) {
            prev_node = curr_node;
            curr_node = next_node;
        } else {
            write_pair(outfile, curr_node->code, curr_sym, bit_length(next_code));
            curr_node->children[curr_sym] = trie_node_create(next_code);
            curr_node = root;
            next_code += 1;
        }

        if (next_code == MAX_CODE) {
            trie_reset(root);
            curr_node = root;
            next_code = START_CODE;
        }

        prev_sym = curr_sym;
    }

    // CHECK if current node points to the root trie node.

    if (curr_node != root) {
        write_pair(outfile, prev_node->code, prev_sym, bit_length(next_code));
        next_code = (next_code + 1) % MAX_CODE;
    }

    // SIGNAL the end of the compressed output.

    write_pair(outfile, STOP_CODE, 0, bit_length(next_code));

    // FLUSH any unwritten, buffered pairs.

    flush_pairs(outfile);

    // PRINT statistics if verbose output is enabled.

    if (verbose_output) {
        uint64_t compressed_size = total_syms;
        uint64_t uncompressed_size = total_bits / 8;

        double space_saved = 100 * (1 - (compressed_size / uncompressed_size));

        fprintf(stdout, "Compressed file size: %" PRIu64 " bytes\n", total_syms / 8);
        fprintf(stdout, "Uncompressed file size: %" PRIu64 " bytes\n", total_bits / 8);
        fprintf(stdout, "Space saving: %.2f%%\n", space_saved);
    }

    // CLOSE both files.

    close(infile);
    close(outfile);

    return 0;
}
