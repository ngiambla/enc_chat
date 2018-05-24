
CC 		= gcc
LIBS 	= -lncurses -lpthread

all: client.c server.c enc.c
	$(CC) -o client client.c enc.c $(LIBS)
	$(CC) -o server server.c enc.c

clean:
	rm client server
