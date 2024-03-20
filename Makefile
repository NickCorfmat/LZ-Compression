CC = clang
CFLAGS = -Wall -Wextra -Werror -Wpedantic -gdwarf-4
LFLAGS = -lm

all: encode decode

encode: encode.o io.o word.o trie.o
	$(CC) -o $@ $^ $(CFLAGS) $(LFLAGS)

decode: decode.o io.o word.o trie.o
	$(CC) -o $@ $^ $(CFLAGS) $(LFLAGS)
	
%.o: %.c
	$(CC) $(CFLAGS) -c $<
	
clean:
	rm -f encode *.o decode *.o io *.o word *.o trie *.o
	
format:
	clang-format -i -style=file *.[ch]
