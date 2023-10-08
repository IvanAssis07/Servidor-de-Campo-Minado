#pragma once // incluir só uma vez

#include <stdlib.h>
#include <arpa/inet.h>

struct action
{
  int type;
  int coordinates[2];
  int board[4][4];
};

void printBoard(int board[4][4]);
int addrparse(const char *addtstr, const char *portstr, struct sockaddr_storage *storage);
int server_sockaddr_init(const char *proto, const char *portstr, struct sockaddr_storage *storage);
void logexit(const char *msg);
