//biblioteca das funções
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h> // for inet_addr
#include <unistd.h>    // for write
#include <pthread.h>   // for threading, link with lpthread
#include <sys/types.h>
#include <sys/stat.h>
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

    node *filaCliente = (node *) malloc(sizeof(node));
    sem_init(&mutex, 0, 1); // Inıcializa mutex com 1.
    int socket_desc, new_socket, c, *new_sock;
    struct sockaddr_in server, client;

    int num_conn = 0;

    socket_desc = socket(AF_INET, SOCK_STREAM, 0); // cria socket - af_inet: ipv4 - sock stream: TCP - 0: IP
    
    if (socket_desc == -1){
      puts("Não foi possível criar o socket");
      return 1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY; // localhost
    server.sin_port = htons(PORT_NO); // num porta processo

    if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0) { // vincula socket ao end e num porta especificados acima
      puts("Vinculação de um socket a um endereço falhou!");
      return 1;
    }

    listen(socket_desc, 20); // inicia escuta por clientes - limite de 20 na fila
    puts("Esperando por uma conexão...");

    c = sizeof(struct sockaddr_in);
       
    while (1){ // aceita conexoes - salva em novo socket

      //verifica se existe esse cliente na fila
       if(vazia(filaCliente) == 1 || validaExistencia(filaCliente, idSocketClienteTmp) == 0){ //não tem nenhum elemento ou não existe esse elemento na fila
        printf("Não existe nenhum elemento na fila, logo new socket!");
        // insere(filaCliente, idSocketClienteTmp, (struct sockaddr *)&client);
        // struct sockaddr_in *socketClienteTmp = existeFila(filaCliente, idSocketClienteTmp);
        // new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c); 
        // idSocketClienteTmp++;

      }else{
        printf("Existe este socket na fila, captura ele!");
        //procurar na fila
        // struct sockaddr_in *socketClienteTmp = existeFila(filaCliente, idSocketClienteTmp);
        // new_socket = accept(socket_desc, socketClienteTmp, (socklen_t *)&c); //socketClienteTmp
      }
      new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c);
      printf("---\n");
      printf("\n Valor do ip do cliente=%i \n", client.sin_addr.s_addr);
      printf("---\n");
      printf("Descritor é:%i", socket_desc);
      printf("\nValor do new_socket=%i\n", new_socket);
      printf("\n#####\n");

      puts("Conexão aceita! \n");
      
      printf("Número de conexões:%d\n",++num_conn);

      pthread_t sniffer_thread; // nova thread
      new_sock = (int*) malloc(1);
      *new_sock = new_socket;
      
      if (pthread_create(&sniffer_thread, NULL, connection_handler, (void *)new_sock) < 0){ // cria uma thread para cada requisicao, passando socket novo
        puts("Não foi possível criar a thread!");
        break;
      }
    }
    return 0;
  }
