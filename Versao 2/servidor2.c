//biblioteca das funções
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h> // for inet_addr
#include <unistd.h>    // for write
#include <pthread.h>   // for threading, link with lpthread
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include "conexao.h"
#include "servidor2.h"
#include "manipulaSocketImg.h"
#include "manipulaSocketTxt.h"
#include "fila.h"

#define PORT_NO 8080 // numero da porta

int idSocketClienteTmp = 0;

int main(int argc, char *argv[]){

  fd_set testset;

  // node *filaCliente = (node *) malloc(sizeof(node));
  sem_init(&mutex, 0, 1); // Inıcializa mutex com 1.
  int server_sock, new_socket, c, *new_sock, new_thread;
  struct sockaddr_in server, client;


  for(int m = 0; m < 30; m++){
    clients[m] = -1;
  }

  int num_conn = 0;

  server_sock = socket(AF_INET, SOCK_STREAM, 0); // cria socket - af_inet: ipv4 - sock stream: TCP - 0: IP

  if (server_sock == -1){
    puts("Não foi possível criar o socket");
    return 1;
  }

  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY; // localhost
  server.sin_port = htons(PORT_NO); // número de porta processoa

  if (bind(server_sock, (struct sockaddr *)&server, sizeof(server)) < 0) { // vincula socket ao end e num porta especificados acima
    puts("Vinculação de um socket a um endereço falhou!");
    return 1;
  }

  listen(server_sock, 20); // inicia escuta por clientes - limite de 20 na fila
  puts("Esperando por uma conexão...");

  c = sizeof(struct sockaddr_in);

  int sd, max_sd, actv;
  pthread_t threads[20];

  for (;;){ // aceita conexoes - salva em novo socket

      new_socket = accept(server_sock, (struct sockaddr *)&client, (socklen_t *)&c);

      printf("\n#####\n");
      puts("Conexão aceita! \n");
      printf("Número de conexões:%d\n",++num_conn);
      printf("Valor do ip do cliente=%i \n", client.sin_addr.s_addr);
      printf("Valor do número da porta do socket=%i \n", client.sin_port);
      printf("Descritor é:%i\n", server_sock);
      printf("Valor do new_socket=%i\n", new_socket);
      printf("\n#####\n");

      pthread_t sniffer_thread; // nova thread
      new_sock = (int*) malloc(1);
      *new_sock = new_socket; //id da thread

      // cria uma thread para cada requisicao, passando socket novo
      new_thread = pthread_create(&sniffer_thread, NULL, connection_handler, (void *)new_sock);

      if(new_thread < 0){
        puts("Não foi possível criar a thread!");
        break;
      }else{
        puts("Criou a thread para o socket!");
        printf("\n");
      }

    }
    return 0;
  }
