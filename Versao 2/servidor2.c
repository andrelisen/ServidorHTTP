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
  server.sin_port = htons(PORT_NO); // número de porta processo

  if (bind(server_sock, (struct sockaddr *)&server, sizeof(server)) < 0) { // vincula socket ao end e num porta especificados acima
    puts("Vinculação de um socket a um endereço falhou!");
    return 1;
  }

  listen(server_sock, 20); // inicia escuta por clientes - limite de 20 na fila
  puts("Esperando por uma conexão...");

  c = sizeof(struct sockaddr_in);

  int sd, max_sd, actv;
  pthread_t threads[20];

  for (;;){

    FD_ZERO(&testset); // limpa conjunto
    FD_SET(server_sock,&testset); //add socket de escuta do server ao conjunto
    max_sd = server_sock;
    for(int n = 0; n < 20; n++){ //adiciona outros sockets ao conjunto
      sd = clients[n]; // socket descriptor
      if(sd > 0) // se o descritor é valido, add ao conjunto
        FD_SET(sd,&testset);
      if(sd > max_sd) // maior numero de descritor
        max_sd = sd;
    }
    actv = select(max_sd+1,&testset, NULL, NULL, NULL); // espera por atividade em algum socket
    if ((actv < 0) && (errno!=EINTR))
      puts("select error");

    if(FD_ISSET(server_sock,&testset)){
      if((new_socket = accept(server_sock, (struct sockaddr *)&client, (socklen_t *)&c)) == -1){
        perror("Error no accept\n");
        exit(EXIT_FAILURE);
      }else{
        printf("\n#####\n");
        puts("Conexão aceita! \n");
        printf("Número de conexões:%d\n",++num_conn);
        printf("Valor do ip do cliente=%i \n", client.sin_addr.s_addr);
        printf("Valor do número da porta do socket=%i \n", client.sin_port);
        printf("Valor do new_socket=%i\n", new_socket);
        printf("#####\n");
    }
      // adiciona novo socket a lista
      for(int l = 0; l < 20;l++){
        if(clients[l] == -1){ // procura posicao vazia
          clients[l] = new_socket;
          new_sock = (int*) malloc(1);
          *new_sock = clients[l]; //num descritor socket
          new_thread = pthread_create(&threads[l], NULL, connection_handler, (void *)new_sock);
          if(new_thread < 0){
            puts("Não foi possível criar a thread!");
            break;
          }else{
            puts("Criou a thread para o socket novo!");
            printf("\n");
          }
          break;
        }
      }
    }

    for(int k = 0; k < 20; k++){
      if(FD_ISSET(clients[k], &testset)){ // Procura por Atividade em outro socket
        pthread_t t2 = pthread_create(&threads[k], NULL, connection_handler, (void *)&clients[k]);
        if(t2 < 0){
          puts("Não foi possível criar a thread!");
          break;
        }else{
          puts("Criou a thread para o socket existente!");
          printf("\n");
        }
      }
    }


    //verifica se existe esse cliente na fila
    //  if(vazia(filaCliente) == 1 || validaExistencia(filaCliente, idSocketClienteTmp) == 0){ //não tem nenhum elemento ou não existe esse elemento na fila
    //   printf("Não existe nenhum elemento na fila, logo new socket!\n");
    //   // insere(filaCliente, idSocketClienteTmp, (struct sockaddr *)&client);
    //   // struct sockaddr_in *socketClienteTmp = existeFila(filaCliente, idSocketClienteTmp);
    //   // new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c);
    //   // idSocketClienteTmp++;

    // }else{
    //   printf("Existe este socket na fila, captura ele!\n");
    //   //procurar na fila
    //   // struct sockaddr_in *socketClienteTmp = existeFila(filaCliente, idSocketClienteTmp);
    //   // new_socket = accept(socket_desc, socketClienteTmp, (socklen_t *)&c); //socketClienteTmp
    // }




    // pthread_t sniffer_thread; // nova thread
    // new_sock = (int*) malloc(1);
    // *new_sock = new_socket; //id da thread
    //
    // // cria uma thread para cada requisicao, passando socket novo
    // new_thread = pthread_create(&sniffer_thread, NULL, connection_handler, (void *)new_sock);
    //
    // if(new_thread < 0){
    //   puts("Não foi possível criar a thread!");
    //   break;
    // }else{
    //   puts("Criou a thread para o socket!");
    //   printf("\n");
    // }

  }
  return 0;
}
