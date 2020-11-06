#include <stdio.h>
#include <string.h> // for strlen
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


#define PORT_NO 8080 // numero da porta
#define BUFFER_SIZE 1024 // tamanho buffer do socket
#define CONNECTION_NUMBER 20 // quantidade maxima de threads simultaneas

#define EOL "\r\n" // end of line
#define EOL_SIZE 2

sem_t mutex; // para controlar o contador de threads

void image_handler(int socket, char *file_name);
void text_handler(int socket, char *file_name);

int main(int argc, char *argv[]);