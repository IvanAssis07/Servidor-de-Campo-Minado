#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h> 
#include <sys/socket.h>
#include <netdb.h>

#define BUFSZ 1024

// Tipo protocolo, port
void usage(int argc, char **argv) {
  printf("usage: %s <v4|6> <server port>", argv[0]);
  printf("example: %s v4 51511\n", argv[0]);
  exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
  if (argc < 3) {
    usage(argc, argv);
  }

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
  printf("bound to %s, waiting connections\n", addrstr);

  while(1) {
    struct sockaddr_storage cstorage; // Endereço do cliente.
    struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
    socklen_t caddrlen = sizeof(cstorage);

    int csock = accept(s, caddr, &caddrlen); // Socket que conversa com cliente.
    if(csock == -1) {
      logexit("accept");
    }

    char caddrstr[BUFSZ];
    addrtostr(addr, caddrstr, BUFSZ);
    printf("[log] connection from %s\n", caddrstr);

    // Não trata msgs complexas do cliente, pensa que o cliente manda tudo de uma vez.
    // Se chegar incompleto, essa que vai ser a msg
    char buf[BUFSZ];
    memset(buf, 0, BUFSZ); // Zera o buffer.
    size_t count = recv(csock, buf, BUFSZ, 0); // Qtd de bytes recebidos.
    printf("[msg] %s, %d bytes: %s\n", caddrstr, (int)count, buf);
  
    // Envia a msg de volta para o cliente.
    sprintf(buf, "remove endpoint: %.1000s\n", caddrstr);
    count = send(csock, buf, strlen(buf) + 1, 0);

    // Testa se não enviou tudo.
    if (count != strlen(buf) + 1) {
      logexit("send");
    }

    close(csock); // Fecha a conexão com o cliente.
  }

  exit(EXIT_SUCCESS);
}