all:
	gcc -Wall -c common.c -o bin/common.o
	gcc -Wall client.c bin/common.o -o bin/client
	gcc -Wall server.c bin/common.o -o bin/server