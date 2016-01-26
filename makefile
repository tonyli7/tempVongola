all: server client

server: server.c game_funct.c game_funct.h
	gcc -o server server.c game_funct.c

client: client.c
	gcc -o client client.c

clean:
	rm -f *~
	rm -f *#
	rm -f .#*
	rm -f server
	rm -f client
