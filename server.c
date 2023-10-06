#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h> 
#include <sys/socket.h>
#include <netdb.h>

#define BUFSZ 1024
int board[4][4];
struct action game;

void getMatrix(char *buf) {
  FILE *fp;
  fp = fopen(buf, "r");

  if(fp == NULL) {
    printf("Not able to open the file.");
  }

  char *buffer;
  size_t bufsize = 128;
  int rowsRead = 0;
  int colsRead = 0;

  buffer = (char *)malloc(bufsize * sizeof(char));

  while (rowsRead < 4 && (getline(&buffer, &bufsize, fp)) != -1) {
    char *token = strtok(buffer, ",");

    while (token != NULL && colsRead < 4) {
      int cell = atoi(token);
      board[rowsRead][colsRead] = cell;
      game.board[rowsRead][colsRead] = -2;
      token = strtok(NULL, ","); // Continue tokenizing the string you passed in first.

      colsRead++;
    }

    rowsRead++;
    colsRead = 0;
  }

  fclose(fp);
}

// Tipo protocolo, port
void usage(int argc, char **argv) {
  printf("usage: %s <v4|6> <server port>\n", argv[0]);
  printf("example: %s v4 51511\n", argv[0]);
  exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
  if (argc < 5) {
    usage(argc, argv);
  }

  getMatrix(argv[4]);
  // printBoard(board);

  struct sockaddr_storage storage;
  // arg[1] -> tipo protocolo, argv[2] -> porta
  if (server_sockaddr_init(argv[1], argv[2], &storage) != 0) {
    usage(argc, argv);
  }

  int s; // Socket que recebe conexões.
  s = socket(storage.ss_family , SOCK_STREAM, 0);
  if (s == -1) {
    logexit("socket");
  }

  int enable = 1;
  if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) != 0) {
    logexit("setsockopt");
  }

  struct sockaddr *addr = (struct sockaddr *)(&storage);
  if (bind(s, addr, sizeof(storage)) != 0) {
    logexit("bind");
  }

  // 10 -> número máximo de conexões pendentes
  if (listen(s, 10) != 0) {
    logexit("listen");
  }

  char addrstr[BUFSZ];
  addrtostr(addr, addrstr, BUFSZ);
  // printf("bound to %s, waiting connections\n", addrstr);

  while(1) {
    struct sockaddr_storage cstorage; // Endereço do cliente.
    struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
    socklen_t caddrlen = sizeof(cstorage);

    int csock = accept(s, caddr, &caddrlen); // Socket que conversa com cliente.
    if(csock == -1) {
      logexit("accept");
    }
    
    // char caddrstr[BUFSZ];
    // addrtostr(addr, caddrstr, BUFSZ);
    // printf("[log] connection from %s\n", caddrstr);
    printf("client connected\n");
    printBoard(board);
    while (1) {
      // Não trata msgs complexas do cliente, pensa que o cliente manda tudo de uma vez.
      // Se chegar incompleto, essa que vai ser a msg
      struct action clientGame;
      size_t count = recv(csock, &clientGame, sizeof(struct action) , 0); // Qtd de bytes recebidos.
      // printf("mensagem do cliente: %d\n", clientGame.type);

      // Fecha a conexão do cliente, caso ele tenha saido do jogo.
      if (clientGame.type == 7) {
        break;
      }
    
      // Enviando jogo para o servidor.
      count = send(csock, &game, sizeof(struct action), 0);

      // Testa se não enviou tudo.
      if (count != sizeof(struct action)) {
        logexit("send");
      } 
    }

    close(csock); // Fecha a conexão com o cliente.
  }

  exit(EXIT_SUCCESS);
}