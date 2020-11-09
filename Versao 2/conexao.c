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

int thread_count = 0; //contador do numero de threads ativas ao mesmo tempo

void *connection_handler(void *socket_desc) { //aqui vai receber a mensagem do cliente, manipular e administrar as threads
    printf("+++++++++++++++ENTROU NO CONNECTION HANDLER+++++++++++++++\n");
    int request;
    char client_reply[BUFFER_SIZE], *request_lines[100];
    char *file_name;
    char *conn_type;
    // pegar descritor do socket
    int sock = *((int *)socket_desc);
    // recupera o request do cliente
    request = recv(sock, client_reply, BUFFER_SIZE, 0);

    sem_wait(&mutex); // lock semaphore

    thread_count++; // incrementa qtd de threads

    // testa que quantidade atual de threads nao excede limite. Se excede, manda mensagem de ocupado.
    if(thread_count > CONNECTION_NUMBER){
      puts("Conexão recusada. Limite de Threads excedido!");
      puts("HTTP/1.1 503 Service Unavailable");
      char *message = "HTTP/1.1 503 Service Unavailable\r\nContent-Type: text/html\r\n\r\n<!doctype html><html><body>System is busy right now.</body></html>";
      write(sock, message, strlen(message)); //manda para cliente mensagem de ocupado
      thread_count--; // decrementa num threads
      sem_post(&mutex); // release semaphore
      free(socket_desc);
      shutdown(sock, SHUT_RDWR);//desliga socket, finalizando conexao - ReaDWRite
      close(sock); //destroi o socket
      sock = -1;
      pthread_exit(NULL);
    }
    sem_post(&mutex); // release semaphore

    if (request < 0) { // receive falhou
      puts("Recv falhou");
    } else if (request == 0){ // socket fechado, cliente desconectou.
      puts("Cliente se desconectou inesperadamente.");
    } else { // mensagem recebida
      printf("\n");
      puts ("====== Mensagem recebida:");
      printf("%s", client_reply);

      request_lines[0] = strtok(client_reply, " \t\n"); // pega a primeira parte da requisicao GET

      if (strncmp(request_lines[0], "GET\0", 4) == 0){ //se a req for um get

        // separa as outras partes
        request_lines[1] = strtok(NULL, " \t"); // endereço
        request_lines[2] = strtok(NULL, " \t\n"); // protocolo
        int cont = 2;
        while ( request_lines[cont] != NULL ){// loop para separar todo resto do request por espaços
          cont++;
          request_lines[cont] = strtok(NULL," \t\r\n\v"); // era para ser por linha mas não presta
          if (request_lines[cont] == NULL){
            break;
          }
        }

        for (int i = 0; i < 100; i++) { // procura pelo tipo de connection
          if(strcmp(request_lines[i],"Connection:\0")==0){
            conn_type = request_lines[i+1];
            printf("connection: %s\n", conn_type);
            break;
          }
        }

        // Bad request se não for do tipo HTTP
        if (strncmp(request_lines[2], "HTTP/1.0", 8) != 0 && strncmp(request_lines[2], "HTTP/1.1", 8) != 0){
          puts("Pedido errado.");
          char *message = "HTTP/1.1 400 Bad Request\r\nConnection: close\r\nContent-Type: text/html\r\n\r\n<!doctype html><html><body>400 Bad Request</body></html>";
          write(sock, message, strlen(message));
        }else{
            char *tokens[2]; // para dividir em nome do arquivo e extensao

            file_name = (char *)malloc(strlen(request_lines[1]) * sizeof(char));
            file_name[0] = '\0';
            strcpy(file_name, request_lines[1]);
            puts(file_name);

            // Getting the file name and extension
            tokens[0] = strtok(file_name, "."); //nome do arquivo
            tokens[1] = strtok(NULL, "."); // extensao

            if (tokens[0] == NULL || tokens[1] == NULL) { // se nao tem extensao ou request direto para localhost:PORT_NO

              puts("Pedido errado.");
              char *message = "HTTP/1.1 400 Bad Request\r\nConnection: close\r\n\r\n<!doctype html><html><body>400 Bad Request. (You need to request to image and text files)</body></html>";
              write(sock, message, strlen(message));

            }
            else if (strcmp(tokens[1], "html") == 0 || strcmp(tokens[1], "htm") == 0 || strcmp(tokens[1], "css") == 0 || strcmp(tokens[1], "js") == 0 || strcmp(tokens[1], "txt") == 0 ){

              sem_wait(&mutex); // Evita que duas ou mais threads façam operacoes de IO ao mesmo tempo - lock semaphore
              text_handler(sock, request_lines[1], tokens[1]); // chama func de ler arquivos texto
              sem_post(&mutex); // release semaphore

            }else if (strcmp(tokens[1], "jpeg") == 0 || strcmp(tokens[1], "jpg") == 0 || strcmp(tokens[1], "png") == 0 || strcmp(tokens[1], "bmp") == 0 || strcmp(tokens[1], "gif") == 0 || strcmp(tokens[1], "webp") == 0|| strcmp(tokens[1], "ico") == 0 ){//requisição do tipo img

              sem_wait(&mutex); // lock semaphore -- deve-se estudar aqui. Sera porque n pode ter mais de uma thread tentando escrever no socket ao mesmo tempo?
              printf("Solicitei uma imagem ao servidor!\n");
              image_handler(sock, request_lines[1], tokens[1]); // chama func de ler arquivos de imagem (binarios?) -- existe apenas um semaforo, o q significa que só uma thread pode entrar nesses caminhos por vez, independente do cliente?
              sem_post(&mutex); // release semaphore

            }else { // extensao que nao pertence as de acima.

              char *message = "HTTP/1.1 415 Unsupported Media Type\r\nConnection: close\r\n\r\n<!doctype html><html><body>415 Unsupported Media Type.</body></html>";
              write(sock, message, strlen(message));

            }
          free(file_name);
        }
      }
    }

    //sleep(10); // Descomentar para conseguir enxergar o limite de threads.

    if(strcmp(conn_type, "keep-alive") == 0 || strcmp(conn_type, "Keep-Alive") == 0 ){ //compara se é keep-alive;
      puts("conexão keep-alive");
      // não fechar conexão!!, só sai da thread.
      sem_wait(&mutex); // pega lock para decrementar contador
      thread_count--;
      sem_post(&mutex);
      pthread_exit(NULL); // sai da thread
    }else {
      puts("Encerrando conexão");
      free(socket_desc);
      shutdown(sock, SHUT_RDWR); // encerra conexao do socket
      close(sock); // destroi socket
      sock = -1;
      sem_wait(&mutex); // pega lock para decrementar contador
      thread_count--;
      sem_post(&mutex);
      pthread_exit(NULL); // sai da thread
  }
  }
