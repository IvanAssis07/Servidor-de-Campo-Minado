#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>

// Trata erros de execução nas operações com socket.
void logexit(const char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

void printBoard(int board[4][4]) {
  for(int i = 0; i < 4; i++) {
    for(int j = 0; j < 4; j++) {
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

int addrparse(const char *addtstr, const char *portstr, struct sockaddr_storage *storage) {
  if (addtstr == NULL || portstr == NULL) {
    return -1;
  }

  // Padrão internet port tem 16bits
  uint16_t port = (uint16_t)atoi(portstr); 
  if (port == 0) {
    return -1;
  }

  port = htons(port); // Converte para bigEndian

  // Fazer o parser do endereço (IPv4 ou IPv6)
  struct in_addr inaddr4; // IPv4 
  if (inet_pton(AF_INET, addtstr, &inaddr4)) { // Converte notação de pontos para binário
    struct sockaddr_in *addr4 = (struct sockaddr_in *)storage;
    addr4->sin_family = AF_INET;
    addr4->sin_port = htons(port);
    addr4->sin_addr = inaddr4;

    return 0;
  }

  struct in6_addr inaddr6; // IPv6
  if (inet_pton(AF_INET6, addtstr, &inaddr6)) { // Converte notação de pontos para binário
    struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)storage;
    addr6->sin6_family = AF_INET6;
    addr6->sin6_port = htons(port);
    memcpy(&(addr6->sin6_addr), &inaddr6, sizeof(inaddr6));

    return 0;
  }

  // Não é IPv4, nem IPv6
  return -1;
}

int server_sockaddr_init(const char *ipVersion, const char* portstr, struct sockaddr_storage *storage) {
  // Padrão internet port tem 16bits
  uint16_t port = (uint16_t)atoi(portstr); 
  if (port == 0) {
    return -1;
  }

  port = htons(port); // Converte para bigEndian.

  memset(storage, 0, sizeof(*storage));
  if (strcmp(ipVersion, "v4") == 0) {
    struct sockaddr_in *addr4 = (struct sockaddr_in *)storage;
    addr4->sin_family = AF_INET;
    addr4->sin_addr.s_addr = INADDR_ANY; // Roda em qualquer endereço disponível do PC
    addr4->sin_port = htons(port);

    return 0;
  } else if(strcmp(ipVersion, "v6") == 0) {
    struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)storage;
    addr6->sin6_family = AF_INET6;
    addr6->sin6_addr = in6addr_any; // Roda em qualquer disponível endereço do PC
    addr6->sin6_port = htons(port);

    return 0;
  } else {
    return -1;
  }
}