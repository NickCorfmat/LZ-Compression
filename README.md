Nicolas Corfmat
ncorfmat
ID: 1890805
# Assignment 6: Lempel-Ziv Compression
```
DESCRIPTION
``` 
This project consists of two main programs: *encode* and *decode*, which implement LZ78 compression and decompression respectively. The *encode* program is used to compress either text or binary files while *decode* decompresses compressed files.
```
BUILD
``` 
To build, run `make` or `make all` on the terminal command line within the assignment 6 directory. This creates the **'encode'**, **'decode'**, **'io'**, **'word'**, and **'trie'** executable files which can then be run.
```
CLEAN
```
Running `make clean` will remove all the executable (.o) files from the assignment 6 directory as well as other compiler-generated files.
```
RUNNING
```
To run, first compress the text/binary file by typing `./encode` followed by any of these arguments: 
+ `-v` Prints compression statistics to stderr
+ `-i` followed by the input to compress (default: stdin)
+ `-o` followed by the output of compressed input (default: stdout)
+ `-h` Display program usage

To decompress the text/binary file, run `./decode` followed by any of these arguments:
+ `-v` Prints compression statistics to stderr
+ `-i` followed by the input to decompress (default: stdin)
+ `-o` followed by the output of decompressed input (default: stdout)
+ `-h` Display program usage
```
PIPING
```
To pipe these commands together, run `./encode` | `./decode` with any valid arguments after both commands.


