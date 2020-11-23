#include <stdio.h>
#include <string.h> // for strlen
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h> // for inet_addr
#include <unistd.h>    // for write
#include <pthread.h>   // for threading, link with lpthrsead
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include "conexao.h"


#define PORT_NO 8080 // numero da porta
#define BUFFER_SIZE 8192 // tamanho buffer do socket
#define MAX_CON 50 // quantidade maxima de threads simultaneas

#define EOL "\r\n" // end of line
#define EOL_SIZE 2

struct  cliente{
  int num_socket;
  int num_set;
  int qtd_conn;
  int taxa;
  char *ip;
  int flag_tempo; //se 0, não dispara, se 1 dispara
};

struct thread_data{
  int n_s;
  int res;
};

sem_t mutex, mutex2; // para controlar o contador de threads

void image_handler(int socket, char *file_name, char *ext);
void text_handler(int socket, char *file_name, char *ext);

void *timer(void* arg);
void *thread_client(void* arg);

int main(int argc, char *argv[]);
