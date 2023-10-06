#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h> 
#include <sys/socket.h>
#include <netdb.h>

#define BUFSZ 1024
struct action clientGame;

void usage(int argc, char **argv) {
  printf("usage: %s <server IP> <server port>", argv[0]);
  printf("example: %s 127.0.0.1 51511", argv[0]);
  exit(EXIT_FAILURE);
};
 
int main(int argc, char **argv) {
  if (argc < 3) {
    usage(argc, argv);
  }

  struct sockaddr_storage storage;
  if (addrparse(argv[1], argv[2], &storage) != 0) {
    usage(argc, argv);
  }

  int s;
  s = socket(storage.ss_family , SOCK_STREAM, 0);
  if (s == -1) {
    logexit("socket");
  }

  // struct sockaddr "uma interface" para os 2 tipos de endereço"
  struct sockaddr *addr = (struct sockaddr *)(&storage);
  if(connect(s, addr, sizeof(storage)) != 0) {
    logexit("connect");
  }

  char addrstr[BUFSZ];
  addrtostr(addr, addrstr, BUFSZ);

  printf("connected to %s\n", addrstr);

  size_t count = send(s, &clientGame, sizeof(struct action), 0);
  if(count != sizeof(struct action)) {
    logexit("send");
  }
  
  count = recv(s, &clientGame, sizeof(struct action), 0);
  if (count == 0) {
    printf("Conexão fechada\n");
  }
  close(s);
  printf("mensagem do servidor: \n");
  printBoard(clientGame.board);

  exit(EXIT_SUCCESS);
}

  