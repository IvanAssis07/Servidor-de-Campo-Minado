#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h> 
#include <sys/socket.h>
#include <netdb.h>

#define BUFSZ 1024
int BOMBS_NUMBER = 3;
int originalBoard[4][4]; // Resposta do jogo.
struct action game; // Contém o jogo atual que é atualizado de acordo com as jogadas do cliente.

// Exibe ao cliente argumentos esperados para iniciar o servidor.
void usage(int argc, char **argv) {
  printf("usage: %s <v4|6> <server port to listen on>\n", argv[0]);
  printf("example: %s v4 51511\n", argv[0]);
  exit(EXIT_FAILURE);
}

// Lê o arquivo que contém o tabuleiro do jogo. E armazena na matriz originalBoard.
void getBoard(char *filePath) {
  FILE *fp;
  fp = fopen(filePath, "r");

  if(fp == NULL) {
    printf("Not able to open the file.");
  }

  char *buffer;
  size_t bufsize = 128;
  buffer = (char *)malloc(bufsize * sizeof(char));
  
  int rowsRead = 0;
  int colsRead = 0;


  while (rowsRead < 4 && (getline(&buffer, &bufsize, fp)) != -1) {
    char *token = strtok(buffer, ",");

    while (token != NULL && colsRead < 4) {
      int cell = atoi(token);
      originalBoard[rowsRead][colsRead] = cell; // Salva a matriz original do jogo.
      token = strtok(NULL, ",");

      colsRead++;
    }

    rowsRead++;
    colsRead = 0;
  }

  fclose(fp);
}

// Volta a matriz do jogo para o estado inicial.
void resetBoard() {
  int i, j;
  for(i = 0; i < 4; i++) {
    for(j = 0; j < 4; j++) {
      game.board[i][j] = -2;
    }
  }
}

// Checa se todas as células a serem reveladas são bombas.
int checkWin() {
  int countHiddenCells = 0;
  for(int i = 0; i < 4; i++) {
    for(int j = 0; j < 4; j++) {
      if (game.board[i][j] == -2 || game.board[i][j] == -3) {
        countHiddenCells++;
      }
    }
  }

  if (countHiddenCells == BOMBS_NUMBER) {
    return 1; // Ganhou.
  } else {
    return 0;
  }
}

void setBoard() {
  int i, j;
  for(i = 0; i < 4; i++) {
    for(j = 0; j < 4; j++) {
      game.board[i][j] = originalBoard[i][j];
    }
  }
}


void handleClientCommand (struct action clientGame) {
  int x = clientGame.coordinates[0];
  int y = clientGame.coordinates[1];

  switch (clientGame.type) {
    case 0: // start
      resetBoard();
      game.type = 3;
      break;
    case 1: // reveal
      // Se a célula revelada for uma bomba, o jogo acaba.
      if (originalBoard[x][y] == -1) {
        game.type = 8;
        setBoard();
      } else {
        game.board[x][y] = originalBoard[x][y];
        game.type = 3;
      }

      if (checkWin() == 1) {
        game.type = 6;
        setBoard();
      }

      break;
    case 2: // flag
      game.board[x][y] = -3;
      game.type = 3;
      break;
    case 4: // remove_flag
      game.board[x][y] = -2;
      game.type = 3;
      break;
    case 5: // reset
      printf("starting new game\n");
      game.type = 3;
      resetBoard();
      break;
  }
}

int main(int argc, char **argv) {
  // Checando se o usuário passou os argumentos corretamente.
  if (argc < 5) {
    usage(argc, argv);
  }

  // Lendo o arquivo que contém o tabuleiro do jogo.
  getBoard(argv[4]);

  struct sockaddr_storage storage;
  if (server_sockaddr_init(argv[1], argv[2], &storage) != 0) {
    usage(argc, argv);
  }

  // Criando o socket que vai receber as conexões.
  int sockfd;
  sockfd = socket(storage.ss_family , SOCK_STREAM, 0);
  if (sockfd == -1) {
    logexit("socket");
  }

  // Impedir de receber o erro ao iniciar o servidor de "address already in use".
  int enable = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) != 0) {
    logexit("Error in setsockopt operation.");
  }

  struct sockaddr *addr = (struct sockaddr *)(&storage);
  if (bind(sockfd, addr, sizeof(storage)) != 0) {
    logexit("Error in bind operation.");
  }

  if (listen(sockfd, 10) != 0) {
    logexit("Error in listen operation.");
  }

  // Imprimindo estado inicial do jogo.
  printBoard(originalBoard);

  while(1) {
    struct sockaddr_storage cstorage; // Endereço do cliente.
    struct sockaddr *clientAddr = (struct sockaddr *)(&cstorage);
    socklen_t clientAddrlen = sizeof(cstorage);

    int clientSocket = accept(sockfd, clientAddr, &clientAddrlen); // Socket que conversa com cliente.
    if(clientSocket == -1) {
      logexit("Error in accept operation.");
    }
    
    printf("client connected\n");

    while (1) {
      // Struct que corresponderá ao jogo do cliente.
      struct action clientGame;

      size_t bytesCounter = recv(clientSocket, &clientGame, sizeof(struct action) , 0); // Qtd de bytes recebidos.

      // Checa se o cliente deu o comando "exit", caso sim, fecha a conexão.
      // Caso contrário, trata o comando do cliente.
      if (clientGame.type == 7) {
        printf("client disconnected\n");
        resetBoard();
        game.type = 3;
        break;
      } else {
        handleClientCommand(clientGame);
      }

      // Enviando jogo para o servidor.
      bytesCounter = send(clientSocket, &game, sizeof(struct action), 0);

      // Testa se toda a mensagem foi enviada.
      if (bytesCounter != sizeof(struct action)) {
        logexit("Error in send operation.");
      } 
    }

    close(clientSocket); // Fecha a conexão com o cliente.
  }

  exit(EXIT_SUCCESS);
}