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
#include "word.h"
#include "trie.h"
#include "code.h"

#define OPTIONS "vi:o:h"

// HELPER function to calculate bit length.
int bit_length(uint64_t bits) {
    return (int) log2(bits) + 1;
}

int main(int argc, char **argv) {

    uint64_t opt = 0;

    bool verbose_output = false;

    int infile = STDIN_FILENO;
    int outfile = STDOUT_FILENO;

    FileHeader header;

    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 'v': verbose_output = true; break;
        case 'i':
            infile = open(optarg, O_RDONLY);

            if (infile == -1) {
                fprintf(stderr, "Error: Decode could not access input file.\n");
                exit(1);
            }

            break;
        case 'o':
            outfile = open(optarg, O_CREAT | O_WRONLY | O_TRUNC);

            if (outfile == -1) {
                fprintf(stderr, "Error: Decode  could not access output file.\n");
                exit(1);
            }

            break;
        case 'h':
            printf("SYNOPSIS\n");
            printf("   Decompresses files with the LZ78 decompression algorithm.\n");
            printf("   Used with files compressed with the corresponding encoder.\n\n");
            printf("USAGE\n");
            printf("   ./decode-test [-vh] [-i input] [-o output]\n\n");
            printf("OPTIONS\n");
            printf("   -v          Display decompression statistics\n");
            printf("   -i input    Specify input to decompress (stdin by default)\n");
            printf("   -o output   Specify output of decompressed input (stdout by default)\n");
            printf("   -h          Display program usage\n");

            break;

        default:
            printf("SYNOPSIS\n");
            printf("   Decompresses files with the LZ78 decompression algorithm.\n");
            printf("   Used with files compressed with the corresponding encoder.\n\n");
            printf("USAGE\n");
            printf("   ./decode-test [-vh] [-i input] [-o output]\n\n");
            printf("OPTIONS\n");
            printf("   -v          Display decompression statistics\n");
            printf("   -i input    Specify input to decompress (stdin by default)\n");
            printf("   -o output   Specify output of decompressed input (stdout by default)\n");
            printf("   -h          Display program usage\n");

            return 0;
        }
    }

    uint8_t curr_sym = 0;

    uint16_t curr_code = 0;
    uint16_t next_code = START_CODE;

    // READ file header. This also verifies the magic number.

    read_header(infile, &header);

    // CREATE an empty word table.

    WordTable *table = wt_create();

    // LOOP to read all pairs from infile.

    while ((read_pair(infile, &curr_code, &curr_sym, bit_length(next_code)))) {

        // APPEND current sym to next word.
        table[next_code] = word_append_sym(table[curr_code], curr_sym);

        // WRITE the word that was constructed.
        write_word(outfile, table[next_code]);

        // INCREMENT next_code
        next_code += 1;

        if (next_code == MAX_CODE) {
            wt_reset(table);
            next_code = START_CODE;
        }
    }

    // FLUSH any buffered words.

    flush_words(outfile);

    // DEREFERENCE WordTable.
    wt_delete(table);

    // PRINT statistics if verbose output is enabled.

    if (verbose_output) {
        uint64_t compressed_size = total_syms;
        uint64_t uncompressed_size = total_bits / 8;

        double space_saved = 100 * (1 - (compressed_size / uncompressed_size));

        fprintf(stdout, "Compressed file size: %" PRIu64 " bytes\n", total_syms / 8);
        fprintf(stdout, "Uncompressed file size: %" PRIu64 " bytes\n", total_bits / 8);
        fprintf(stdout, "Space saving: %.2f%%\n", space_saved);
    }

    // CLOSE files.

    close(infile);
    close(outfile);

    return 0;
}
