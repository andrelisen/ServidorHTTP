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
int imp = 0;
fd_set testset;
struct cliente **clients;

int main(int argc, char *argv[]){

  sem_init(&mutex, 0, 1); // Inıcializa mutex com 1.
  int server_sock, new_socket, c, *new_sock, new_thread;
  struct sockaddr_in server, client;

  clients = (struct cliente**)malloc(MAX_CON*sizeof(struct cliente));

  //inicializa struct que guarda informações do cliente
  for(int m = 0; m <  MAX_CON; m++){
    clients[m] = (struct cliente*)malloc(sizeof(struct cliente));
    clients[m]->num_socket = -1;
    clients[m]->num_set = -1;
    clients[m]->qtd_conn = 0;
    clients[m]->taxa = -1;
    clients[m]->ip = 0;
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

  for (;;){ // aceita conexoes - salva em novo socket
    printf("Entrei no for ;;\n");
    FD_ZERO(&testset); // limpa conjunto
    FD_SET(server_sock,&testset); //add socket server ao conjunto
    for(int n = 0; n <  MAX_CON; n++){
      sd = clients[n]->num_socket;
      if(sd > 0)
        FD_SET(sd,&testset);
      if(sd > max_sd)
        max_sd = sd;
    }
    actv = select(max_sd+1,&testset, NULL, NULL, NULL);
    if ((actv < 0) && (errno!=EINTR)){
      puts("select error");
    }

    if(FD_ISSET(server_sock,&testset)){
      if((new_socket = accept(server_sock, (struct sockaddr *)&client, (socklen_t *)&c)) == -1){
          perror("Error no accept\n");
          exit(EXIT_FAILURE);
      } else
       puts("Accept executado");

      printf("\n#####\n");
      puts("Conexão aceita!");
      printf("Número de conexões:%d\n",++num_conn);
      printf("Valor do ip do cliente=%i \n", client.sin_addr.s_addr);
      printf("Valor do número da porta do socket=%i \n", client.sin_port);
      printf("Valor do new_socket=%i\n", new_socket);
      printf("#####\n");

      for(int l = 0; l <  MAX_CON;l++){
        if(clients[l]->num_socket == -1){
           clients[l]->num_socket = new_socket;
           clients[l]->num_set = l;
           clients[l]->ip =  client.sin_addr.s_addr;
           pthread_t sock_thread; // nova thread
           new_thread = pthread_create(&sock_thread, NULL, connection_handler, (void *)clients[l]);
           if(new_thread < 0){
             puts("Não foi possível criar a thread!");
             break;
           }else{
             puts("Criou a thread para o novo socket!");
             printf("\n");
           }
           break;
        }
      }
  }

  for (int i = 0; i < MAX_CON; i++){
  //  puts("Dentro do for para clientes");
  //  printf("no for loop\n");
      if (FD_ISSET(clients[i]->num_socket, &testset)){
        puts("Atividade nova em um socket ativo");
        //atividade em um socket
        pthread_t sock_thread; // nova thread
        pthread_create(&sock_thread,NULL,connection_handler,(void *)clients[i]);
      }

      if(!FD_ISSET(clients[i]->num_socket, &testset) && (clients[i]->num_socket != -1)){
        //socket aberto sem nada novo; disparar contador
          puts("Nenhuma atividade em socket ativo");
          struct thread_data tdata;
          tdata.n_s = clients[i]->num_set;
          tdata.res = 0;
          pthread_t new_thread; // nova thread
          pthread_create(&new_thread, NULL,timer, (void*)&tdata);
          pthread_join(new_thread, NULL);

          if(tdata.res == 1){
            puts("Timer encerrado antes: atividade nova no socket");
            pthread_t tid; // nova thread
            pthread_create(&tid,NULL,connection_handler,(void *)clients[i]);
          }
      }
  }
}
  return 0;
}

/////////////////////////////////////////////////////////////////////
void *timer(void * arg){

  struct thread_data *tdata = (struct thread_data*)arg;
  int x = 0;
  puts("Iniciando contagem para matar socket...");
  clock_t start = clock(); // inicia tempo
  printf("\\----\\\n");
  printf("Tempo inicial = %ld\n", start);
  while(((clock() - start)/CLOCKS_PER_SEC) < 5){ //conta 5 segundos
  //  printf("Estou dentro do while do tempo\n");
  //  printf("Valor do start = %ld\n", start);
  //  printf("Valor do clock = %ld\n", clock());
    if(FD_ISSET(clients[tdata->n_s]->num_socket, &testset)){ // le o socket de novo, se chegou algo no socket, sai do contador
      printf("Teve uma nova requisição para este socket, abortando contagem\n");
      x = 1;
      break; //sai do while da contagem dos 5s
    }
  }
  if (x == 0){
    clock_t end = clock();
    long tempo_contador = (end - start)/CLOCKS_PER_SEC;
    printf("O valor do contador final é: %ld\n", tempo_contador);

    if (tempo_contador >= 5){
      printf("Dentro if temp > 5s\n");
      puts("Encerrando conexão");
      shutdown(clients[tdata->n_s]->num_socket, SHUT_RDWR); // encerra conexao do socket
      close(clients[tdata->n_s]->num_socket); // destroi socket
      clients[tdata->n_s]->num_socket = -1;
      clients[tdata->n_s]->num_set = -1;
      clients[tdata->n_s]->qtd_conn = 0;
      clients[tdata->n_s]->taxa = -1;
      clients[tdata->n_s]->ip = 0;
    }
  }

  puts("Saiu do timer");
  tdata->res = x;
  pthread_exit(NULL);
}
////////////////////////////////////////////////////////
