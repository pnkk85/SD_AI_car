CC=gcc
CFLAGS=-I.

data_collection_server.o: data_collection_server.c
	$(CC) -o $@ $^ -I.

.PHONY: clean

clean:
	rm data_collection_server.o
