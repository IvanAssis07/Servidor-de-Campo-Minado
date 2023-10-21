## Trabalho Prático 1 de Redes

Este código implementa o primeiro trabalho da disciplina de redes da UFMG, o qual consiste em um jogo de campo minado entre um único cliente e um servidor interagindo através do uso de sockets.

O programa foi projetado para funcionar com amobos os protocolos IPv4 e IPv6 junto com um número de porta para fazer o endereçamento.

Para executar o programa localmente será necessário abrir 2 terminais. Antes de tudo você deve rodar o comando _make_.Depois, no terminal que atuará como servidor, você deve entrar na pasta __bin__ e executar o seguinte comando:

**IPv4**: ./server v4 51511 -i ../input/jogo1.txt

**IPv6**: ./server v4 51511 -i ../input/jogo1.txt

Com o servidor do cliente rodando, você deve entrar no terminal do cliente dentro da pasta _bin_ e rodar os seguintes comandos para se conectar com o servidor e poder jogar:

**IPv4**: ./client 127.0.0.1 51511

**IPv6**: ./client ::1 51511

Depois disso o jogo já pode ser iniciado, para checar os comandos possíveis olhe no arquivo pdf _Trabalho Prático 1 - Servidor de Campo Minado_.
