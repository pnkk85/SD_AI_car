CC=gcc
CFLAGS=-I.

mouse_controller_server.o: mouse_controller_server.c server_library.c
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm mouse_controller_server.o
