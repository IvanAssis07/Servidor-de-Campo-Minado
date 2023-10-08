#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h> 
#include <sys/socket.h>
#include <netdb.h>

#define BUFSZ 1024
struct action game;

// Exibe ao cliente argumentos esperados para iniciar o cliente.
void usage(int argc, char **argv) {
  printf("usage: %s <server IP> <server port>", argv[0]);
  printf("example: %s 127.0.0.1 51511", argv[0]);
  exit(EXIT_FAILURE);
};

// Checa se as coordenadas estão dentro dos limites do tabuleiro.
int checkBounds(int x, int y) {
  if (x < 0 || x > 3 || y < 0 || y > 3) {
    return 0;
  } else {
    return 1;
  }
}

// Lê o comando do usuário e armazena na struct game.
// Retorna 1 se o comando é válido e 0 caso contrário.
int getCommand() {
  char *line = NULL; 
  size_t len = 0;   
  char command[20];
  int x, y;

  getline(&line, &len, stdin);
  line[strcspn(line, "\r\n")] = 0;

  if (sscanf(line, "%s %d,%d", command, &x, &y) == 1) {
    if (strcmp(command, "start") == 0) {
      game.type = 0;
    } else if (strcmp(command, "reset") == 0) {
      game.type = 5;
    } else if (strcmp(command, "exit") == 0) {
      game.type = 7;
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
      if (game.board[x][y] >= 0) {
        printf("error: cell already revealed\n");
        return 0;
      } 
      game.type = 1;
      game.coordinates[0] = x;
      game.coordinates[1] = y;
    } else if (strcmp(command, "flag") == 0) {
      if (game.board[x][y] == -3) {
        printf("error: cell already has a flag\n");
        return 0;
      } else if (game.board[x][y] >= 0) {
        printf("error: cannot insert flag in revealed cell\n");
        return 0;
      }
      game.type = 2;
      game.coordinates[0] = x;
      game.coordinates[1] = y;
    } else if (strcmp(command, "remove_flag") == 0) {
      game.type = 4;
      game.coordinates[0] = x;
      game.coordinates[1] = y;
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

int main(int argc, char **argv) {
  // Checando se o usuário passou os argumentos corretamente.
  if (argc < 3) {
    usage(argc, argv);
  }

  struct sockaddr_storage storage;
  if (addrparse(argv[1], argv[2], &storage) != 0) {
    usage(argc, argv);
  }

  int socketfd;
  socketfd = socket(storage.ss_family , SOCK_STREAM, 0);
  if (socketfd == -1) {
    logexit("socket");
  }

  // Estabelecendo conexão com servidor
  struct sockaddr *addr = (struct sockaddr *)(&storage);
  if(connect(socketfd, addr, sizeof(storage)) != 0) {
    logexit("connect");
  }

  // Após conectado, o cliente pode enviar comandos e jogar.
  while (1) {
    // Lendo comando do cliente.
    int valid = getCommand();

    // Se o comando for inválido, o cliente deve inserir um novo comando.
    if (valid == 0) {
      continue;
    } else {
      size_t bytesCounter = send(socketfd, &game, sizeof(struct action), 0);
      if(bytesCounter != sizeof(struct action)) {
        logexit("send");
      }
      
      bytesCounter = recv(socketfd, &game, sizeof(struct action), 0);
      if (bytesCounter == 0) {
        break;
      } 
      
      // Checando reset.
      if (game.type == 5) {
        printBoard(game.board);
        break;
      }
      
      // Checando vitória.
      if (game.type == 6) {
        printf("YOU WIN!\n");
      }

      // Checando se o jogo acabou.
      if (game.type == 8) {
        printf("GAME OVER!\n");
      }

      printBoard(game.board);
    }
  }
  
  // Ao dar o comando "exit", o jogo é finalizado e a conexão é encerrada.
  close(socketfd);
  exit(EXIT_SUCCESS);
}

  