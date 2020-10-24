#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

char client_message[30000];
char server_answer[1024];
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int thread_counter = 0;

void * socketThread (void *arg){
  int newSocket = *((int *)arg);
  recv(newSocket, client_message, 30000, 0);
  printf("%s\n", client_message);
  printf("---> Thread number %d\n", thread_counter++);
  pthread_mutex_lock(&lock);
  // char *message = malloc(sizeof(client_message)+20);
  // strcpy(message, "Hello Client : \n");
  // strcat(message, client_message);
  // strcat(message, "\n");
  // strcpy(server_answer, message);
  // free(message);
  strcpy(server_answer,"Hello from server\n");
  pthread_mutex_unlock(&lock);
  //sleep(1);
  send(newSocket, server_answer,strlen(server_answer) ,0);
  printf("Exit socketThread\n" );
  close(newSocket);
  pthread_exit(NULL);
}

int main (){
  int serverSocket, newSocket;
  struct sockaddr_in serverAddr;
  struct sockaddr_storage serverStorage;
  socklen_t addr_size;

  serverSocket = socket(PF_INET, SOCK_STREAM,0);
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(7799);
  serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

  memset(serverAddr.sin_zero,'\0', sizeof serverAddr.sin_zero);

  bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

  if(listen(serverSocket,50)==0)
    printf("Listening\n" );
  else
    printf("Error\n" );

  pthread_t tid[60];
  int i = 0;
  while(1){
    addr_size = sizeof serverStorage;
    newSocket = accept(serverSocket, (struct sockaddr*)&serverStorage, &addr_size);
    if( pthread_create(&tid[i++],NULL, socketThread, &newSocket) != 0)
    printf("failed to create thread\n" );

    if (i >= 50){
      i = 0;
      while (i < 50)
      pthread_join(tid[i++], NULL);
      i = 0;
    }
  }
  return 0;
}
