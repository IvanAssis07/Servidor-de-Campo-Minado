#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netdb.h>

#define BUFSZ 1024
struct action clientGame;

int checkBounds(int x, int y) {
  if (x < 0 || x > 3 || y < 0 || y > 3) {
    return 0;
  } else {
    return 1;
  }
}

int getPlay() {
  char *line = NULL; // Dynamic allocation for the line
  size_t len = 0;    // Size of the allocated buffer
  char command[20];
  int x, y;

  getline(&line, &len, stdin);
  line[strcspn(line, "\r\n")] = 0;

  if (sscanf(line, "%s %d,%d", command, &x, &y) == 1) {
    if (strcmp(command, "start") == 0) {
      clientGame.type = 0;
    } else if (strcmp(command, "reset") == 0) {
      clientGame.type = 5;
    } else if (strcmp(command, "exit") == 0) {
      clientGame.type = 7;
    } else {
      printf("error: command not found\n");
      return 0;
    }
  } else if (sscanf(line, "%s %d,%d", command, &x, &y) == 3) {
    if (checkBounds(x, y) == 0) {
      printf("error: invalid cell\n");
      return 0;
    }
    if(strcmp(command, "reveal") == 0) {
      if (clientGame.board[x][y] >= 0) {
        printf("error: cell already revealed\n");
        return 0;
      } 
      clientGame.type = 1;
      clientGame.coordinates[0] = x;
      clientGame.coordinates[1] = y;
    } else if (strcmp(command, "flag") == 0) {
      if (clientGame.board[x][y] == -3) {
        printf("error: cell already has a flag\n");
        return 0;
      } else if (clientGame.board[x][y] >= 0) {
        printf("error: cannot insert flag in revealed cell\n");
        return 0;
      }
      clientGame.type = 2;
      clientGame.coordinates[0] = x;
      clientGame.coordinates[1] = y;
    } else if (strcmp(command, "remove_flag") == 0) {
      clientGame.type = 4;
      clientGame.coordinates[0] = x;
      clientGame.coordinates[1] = y;
    } else {
      printf("error: command not found\n");
      return 0;
    }
  } else {
    printf("error: command not found\n");
    return 0;
  }

  return 1;
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

  while (1) {
    int valid = getPlay();

    if (valid == 0) {
      continue;
    } else {
      size_t count = send(s, &clientGame, sizeof(struct action), 0);
      if(count != sizeof(struct action)) {
        logexit("send");
      }
      
      count = recv(s, &clientGame, sizeof(struct action), 0);
      if (count == 0) {
        break;
      } 
      
      if (clientGame.type == 5) {
        printf("starting new game\n");
        printBoard(clientGame.board);
        break;
      }
      
      if (clientGame.type == 6) {
        printf("YOU WIN!\n");
        printBoard(clientGame.board);
        break;
      }
      if (clientGame.type == 8) {
        printf("GAME OVER!\n");
        // printBoard(clientGame.board);
        // break;
      }

      printBoard(clientGame.board);
    }
  }
  
  close(s);
  exit(EXIT_SUCCESS);
}

  