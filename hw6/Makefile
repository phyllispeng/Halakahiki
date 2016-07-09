CFLAGS = -Werror -Wall -g -std=gnu11 -pthread -lcrypto -lssl
CC = gcc
OBJFLG = -c
OBJECTS = bin/chatroom.o bin/user.o bin/parser.o bin/wrap.o bin/protocol.o bin/fileManager.o bin/password.o bin/signalHandlers.o bin/threadpool.o bin/pineappleProtocol.o

all: bin/server bin/client

bin/server: src/server.c $(OBJECTS)
	$(CC) $^ -o $@ $(CFLAGS)

bin/client: src/client.c $(OBJECTS)
	$(CC) $^ -o $@ $(CFLAGS)

bin/chatroom.o: src/chatroom.c 
	$(CC) $(CFLAGS) $(OBJFLG) $^ -o $@
bin/user.o: src/user.c
	$(CC) $(CFLAGS) $(OBJFLG) $^ -o $@
bin/parser.o: src/parser.c
	$(CC) $(CFLAGS) $(OBJFLG) $^ -o $@
bin/protocol.o: src/protocol.c
	$(CC) $(CFLAGS) $(OBJFLG) $^ -o $@
bin/pineappleProtocol.o: src/pineappleProtocol.c
	$(CC) $(CFLAGS) $(OBJFLG) $^ -o $@
bin/wrap.o: src/wrap.c
	$(CC) $(CFLAGS) $(OBJFLG) $^ -o $@
bin/password.o: src/password.c
	$(CC) $(CFLAGS) $(OBJFLG) $^ -o $@
bin/fileManager.o: src/fileManager.c
	$(CC) $(CFLAGS) $(OBJFLG) $^ -o $@
bin/signalHandlers.o: src/signalHandlers.c
	$(CC) $(CFLAGS) $(OBJFLG) $^ -o $@
bin/threadpool.o: src/threadpool.c
	$(CC) $(CFLAGS) $(OBJFLG) $^ -o $@

directory:
	mkdir bin

clean:
	rm -f bin/*
