#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>

void logexit(const char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

void printBoard(int board[4][4]) {
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

int addrparse(const char *addtstr, const char *portstr, struct sockaddr_storage *storage)
{
  if (addtstr == NULL || portstr == NULL)
  {
    return -1;
  }
  // Padrão internet port tem 16bits
  uint16_t port = (uint16_t)atoi(portstr); // unsigned short int
  if (port == 0)
  {
    return -1;
  }
  port = htons(port); // Converte para bigEndian

  // Fazer o parser do endereço (IPv4 ou IPv6)
  struct in_addr inaddr4; // IPv4 
  if (inet_pton(AF_INET, addtstr, &inaddr4))
  {
    struct sockaddr_in *addr4 = (struct sockaddr_in *)storage;
    addr4->sin_family = AF_INET;
    addr4->sin_port = htons(port);
    addr4->sin_addr = inaddr4;
    return 0;
  }

  struct in6_addr inaddr6; // IPv6
  if (inet_pton(AF_INET6, addtstr, &inaddr6))
  {
    struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)storage;
    addr6->sin6_family = AF_INET6;
    addr6->sin6_port = htons(port);
    memcpy(&(addr6->sin6_addr), &inaddr6, sizeof(inaddr6));
    return 0;
  }

  // Não é IPv4, nem IPv6
  return -1;
}

void addrtostr(const struct sockaddr *addr, char *str, size_t strsize)
{
  int version;
  char addstr[INET6_ADDRSTRLEN + 1] = "";
  uint16_t port;

  if (addr->sa_family == AF_INET) // IPv4
  {
    version = 4;
    struct sockaddr_in *addr4 = (struct sockaddr_in *)addr;
    // network to presentation (text)
    if (!inet_ntop(AF_INET, &(addr4->sin_addr), addstr, INET6_ADDRSTRLEN + 1))
    {
      logexit("ntop");
    }
    port = ntohs(addr4->sin_port); // network to host (dispositivo)
  }
  else if (addr->sa_family == AF_INET6) // IPv6
  {
    version = 6;
    struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)addr;
    if (!inet_ntop(AF_INET6, &(addr6->sin6_addr), addstr, INET6_ADDRSTRLEN + 1))
    {
      logexit("ntop");
    }
    port = ntohs(addr6->sin6_port);
  }
  else
  {
    printf("Unknown family: %d\n", addr->sa_family);
    exit(EXIT_FAILURE);
  }
  if (str)
  {
    snprintf(str, strsize, "IPv%d %s %hu", version, addstr, port);
  }
}

int server_sockaddr_init(const char *proto, const char* portstr, struct sockaddr_storage *storage) {
  // Padrão internet port tem 16bits
  uint16_t port = (uint16_t)atoi(portstr); // unsigned short int
  if (port == 0)
  {
    return -1;
  }
  port = htons(port); // Converte para bigEndian

  memset(storage, 0, sizeof(*storage));
  if (strcmp(proto, "v4") == 0) {
    struct sockaddr_in *addr4 = (struct sockaddr_in *)storage;
    addr4->sin_family = AF_INET;
    addr4->sin_addr.s_addr = INADDR_ANY; // Roda em qualquer endereço disponível do PC
    addr4->sin_port = htons(port);
    return 0;
  } else if(strcmp(proto, "v6") == 0) {
    struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)storage;
    addr6->sin6_family = AF_INET6;
    addr6->sin6_addr = in6addr_any; // Roda em qualquer disponível endereço do PC
    addr6->sin6_port = htons(port);
    return 0;
  } else {
    return -1;
  }
}