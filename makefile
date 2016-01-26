all: server client

server: server.c game_funct.c game_funct.h commands.c commands.h
	gcc -o server server.c game_funct.c commands.c

client: client.c
	gcc -o client client.c

clean:
	rm -f *~
	rm -f *#
	rm -f server
	rm -f client
