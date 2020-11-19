.PHONY: all

all: fileCompressor.c
	gcc -o fileCompressor fileCompressor.c

.PHONY: clean

clean: 
	rm fileCompressor