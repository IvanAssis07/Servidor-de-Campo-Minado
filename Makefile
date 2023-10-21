all:
	gcc -Wall -c common.c -o obj/common.o
	gcc -Wall client.c obj/common.o -o bin/client
	gcc -Wall server.c obj/common.o -o bin/server