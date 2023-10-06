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

void initBoard() {
  for(int i = 0; i < 4; i++) {
    for(int j = 0; j < 4; j++) {
      printf("-\t\t");
    }
    printf("\n");
  }
}

void clientPrintBoard(int board[4][4]) {
  int i, j;
  for(i = 0; i < 4; i++) {
    for(j = 0; j < 4; j++) {
      switch (board[i][j]) {
        case (-1):
          printf("*\t\t");
          break;
        case (-2):
          printf("-\t\t");
          break;
        case (-3):
          printf(">\t\t");
          break;
        default:
          printf("%d\t\t", board[i][j]);
      }
    }
    printf("\n");
  }
}

void getPlay() {
  char *line = NULL; // Dynamic allocation for the line
  size_t len = 0;    // Size of the allocated buffer
  char command[20];
  int x, y;
  ssize_t read;

  getline(&line, &len, stdin);
  line[strcspn(line, "\r\n")] = 0;

  if (sscanf(line, "%s %d,%d", command, &x, &y) == 1) {
    if(strcmp(command, "start") == 0) {
      printf("Start command\n");
      clientGame.type = 0;
    } else if (strcmp(command, "state") == 0) {
      printf("State command\n");
      clientGame.type = 3;
    } else if (strcmp(command, "win") == 0) {
      printf("Win command\n");
      clientGame.type = 6;
    } else if (strcmp(command, "reset") == 0) {
      printf("Reset command\n");
      clientGame.type = 5;
    } else if (strcmp(command, "exit") == 0) {
      printf("Exit command\n");
      clientGame.type = 7;
    } else if (strcmp(command, "game_over") == 0) {
      printf("Game_over command\n");
      clientGame.type = 7;
    } else {
      printf("Invalid command\n");
    }
  } else if (sscanf(line, "%s %d,%d", command, &x, &y) == 3) {
    if(strcmp(command, "reveal") == 0) {
      printf("reveal command\n");
      clientGame.type = 1;
      clientGame.coordinates[0] = x;
      clientGame.coordinates[1] = y;
    } else if (strcmp(command, "flag") == 0) {
      printf("flag command: %d, %d\n", x, y);
      clientGame.type = 2;
      clientGame.coordinates[0] = x;
      clientGame.coordinates[1] = y;
    } else if (strcmp(command, "remove_flag") == 0) {
      printf("remove_flagcommand: %d, %d\n", x, y);
      clientGame.type = 4;
      clientGame.coordinates[0] = x;
      clientGame.coordinates[1] = y;
    } else {
      printf("Invalid command\n");
    }
  } else {
    printf("Invalid command\n");
  }
}

void usage(int argc, char **argv) {
  printf("usage: %s <server IP> <server port>", argv[0]);
  printf("example: %s 127.0.0.1 51511", argv[0]);
  exit(EXIT_FAILURE);
};
 
int main(int argc, char **argv) {
  if (argc < 3) {
    usage(argc, argv);
  }
  initBoard();
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
  getPlay();

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

  