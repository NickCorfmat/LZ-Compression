#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "io.h"
#include "endian.h"
#include "code.h"

int sym_index = 0; // read_buffer index
int bit_index = 0; // write_buffer index

uint64_t total_syms = 0;
uint64_t total_bits = 0;

static uint8_t read_buffer[BLOCK];
static uint8_t write_buffer[BLOCK];

static int negative_one = -1;

// HELPER functions for bit arithmetic and conversion.

int get_bit(uint8_t *buf, uint64_t bits) {
    return (buf[bits / 8] >> (bits % 8)) & 1;
}

void set_bit(uint8_t *buf, uint16_t bits) {
    buf[bits / 8] |= 1 << (bits % 8);
}

uint64_t bits_to_bytes(uint64_t bits) {
    if (bits > 0 && bits < 8) {
        return 1;
    }

    return bits / 8;
}

int read_bytes(int infile, uint8_t *buf, int to_read) {
    int total_bytes = 0;
    int bytes_read = 0;

    // LOOP until all bytes were read OR there are no more bytes to read.
    do {
        bytes_read = read(infile, buf + total_bytes, to_read);

        if (bytes_read == -1) {
            fprintf(stderr, "Error: Failure reading bytes.\n");
            exit(1);
        }

        total_bytes += bytes_read;

    } while (total_bytes != to_read && bytes_read != 0);

    return total_bytes;
}

int write_bytes(int outfile, uint8_t *buf, int to_write) {
    int total_bytes = 0;
    int bytes_written = 0;

    // LOOP until all bytes were written OR there are no more bytes to write.
    do {
        bytes_written = write(outfile, buf + total_bytes, to_write);

        if (bytes_written == -1) {
            fprintf(stderr, "Error: Failure reading bytes.\n");
            exit(1);
        }

        total_bytes += bytes_written;

    } while (total_bytes != to_write && bytes_written != 0);

    return total_bytes;
}

void read_header(int infile, FileHeader *header) {
    read_bytes(infile, (uint8_t *) header, sizeof(FileHeader));

    total_bits += sizeof(header) * 8;

    // SWAP endianness if byte order isn't little endian.
    if (!little_endian()) {
        header->magic = swap32(header->magic);
        header->protection = swap16(header->protection);
    }

    // VERIFY the magic number.
    if (header->magic != MAGIC) {
        fprintf(stderr, "Error: Magic number is incorrect.\n");
        exit(1);
    }
}

void write_header(int outfile, FileHeader *header) {

    total_bits += sizeof(header) * 8;

    // SWAP endianness if byte order isn't little endian.
    if (!little_endian()) {
        header->magic = swap32(header->magic);
        header->protection = swap16(header->protection);
    }

    // VERIFY the magic number.
    if (header->magic != MAGIC) {
        fprintf(stderr, "Error: Magic number is incorrect.\n");
        exit(1);
    }

    write(outfile, (uint8_t *) header, sizeof(FileHeader));
}

bool read_sym(int infile, uint8_t *sym) { // check this

    if (sym_index == 0) {
        uint16_t bytes_read = read_bytes(infile, read_buffer, BLOCK);

        // CHECK if reached end of BLOCK.
        if (bytes_read < BLOCK) {
            negative_one = bytes_read + 1;
        }
    }

    // RETRIEVE next symbol and INCREMENT value of symbol index.

    *sym = read_buffer[sym_index];
    sym_index = (sym_index + 1) % BLOCK;

    if (sym_index == negative_one) {
        total_syms += 1;
        return false;
    }

    return true;
}

void write_pair(int outfile, uint16_t code, uint8_t sym, int bitlen) { // check this

    // WRITE bitlen bits of code to outfile.
    for (int i = 0; i < bitlen; i += 1) {

        uint16_t bit = ((code >> (i % 16)) & 1);

        if (bit) {
            set_bit(write_buffer, bit_index);
        } else {
            write_buffer[bit_index / 8] &= ~(1 << (bit_index % 8));
        }

        bit_index += 1;
        total_bits += 1;

        if (bit_index == BLOCK * 8) {
            flush_pairs(outfile);
            bit_index = 0;
        }
    }

    // WRITE all eight bits of sym to outfile.
    for (int i = 0; i < 8; i += 1) {

        uint8_t bit = ((sym >> (i % 8)) & 1);

        if (bit) {
            set_bit(write_buffer, bit_index);
        } else {
            write_buffer[bit_index / 8] &= ~(1 << (bit_index % 8));
        }

        bit_index += 1;
        total_bits += 1;

        if (bit_index == BLOCK * 8) {
            flush_pairs(outfile);
            bit_index = 0;
        }
    }
}

void flush_pairs(int outfile) {

    // WRITE out remaining pairs of codes and symbols to outfile.
    write_bytes(outfile, write_buffer, bits_to_bytes(bit_index));
}

bool read_pair(int infile, uint16_t *code, uint8_t *sym, int bitlen) { // check this

    // INITIALIZE code and sym.
    *code = 0;
    *sym = 0;

    // WRITE bitlen bits of code to outfile.
    for (int i = 0; i < bitlen; i += 1) {

        if (bit_index == 0) {
            read_bytes(infile, write_buffer, BLOCK);
        }

        *code |= get_bit(write_buffer, bit_index) << i;

        bit_index = (bit_index + 1) % (BLOCK * 8);
    }

    // WRITE all eight bits of sym to outfile.
    for (int i = 0; i < 8; i += 1) {

        if (bit_index == 0) {
            read_bytes(infile, write_buffer, BLOCK);
        }

        *sym |= get_bit(write_buffer, bit_index) << i;

        bit_index = (bit_index + 1) % (BLOCK * 8);
    }

    if (*code == STOP_CODE) {
        return false;
    }

    return true;
}

void write_word(int outfile, Word *w) {

    // WRITE every symbol from w to outfile.
    for (uint32_t i = 0; i < w->len; i += 1) {

        read_buffer[sym_index] = w->syms[i];

        sym_index += 1;
        total_syms += 1;

        if (sym_index == BLOCK) {
            flush_words(outfile);
            sym_index = 0;
        }
    }
}

void flush_words(int outfile) {

    // WRITE out any remaining symbols in the read_buffer to outfile.
    write_bytes(outfile, read_buffer, sym_index);
}
