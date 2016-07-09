CFLAGS = -Werror -Wall -g -std=gnu11 -pthread
CC = gcc
OBJFLG = -c
OBJECTS = bin/chatroom.o bin/user.o bin/parser.o bin/wrap.o bin/Hashmap.o bin/pineappleProtocol.o bin/filemanager.o bin/pass.o

all: bin/server bin/client

bin/server: src/server.c $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@

bin/client: src/client.c $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@

bin/chatroom.o: src/chatroom.c 
	$(CC) $(CFLAGS) $(OBJFLG) $^ -o $@
bin/user.o: src/user.c
	$(CC) $(CFLAGS) $(OBJFLG) $^ -o $@
bin/parser.o: src/parser.c
	$(CC) $(CFLAGS) $(OBJFLG) $^ -o $@
bin/pineappleProtocol.o: src/pineappleProtocol.c
	$(CC) $(CFLAGS) $(OBJFLG) $^ -o $@
bin/wrap.o: src/wrap.c
	$(CC) $(CFLAGS) $(OBJFLG) $^ -o $@
bin/Hashmap.o: src/Hashmap.c
	$(CC) $(CFLAGS) $(OBJFLG) $^ -o $@
bin/pass.o: src/pass.c
	$(CC) $(CFLAGS) $(OBJFLG) $^ -o $@
bin/filemanager.o: src/filemanager.c
	$(CC) $(CFLAGS) $(OBJFLG) $^ -o $@
directory:
	mkdir bin

clean:
	rm -f bin/*
