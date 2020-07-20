CC=gcc
CFLAGS=-I.

cnn_controller_server.o: cnn_controller_server.c server_library.c
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm cnn_controller_server.o
