#include "conexao.h"
#include "servidor2.c"

void *connection_handler(void *socket_desc) { //aqui vai receber a mensagem do cliente, manipular e administrar as threads
    int request;
    char client_reply[BUFFER_SIZE], *request_lines[3];
    char *file_name;

    // pegar descritor do socket
    int sock = *((int *)socket_desc);

    // recupera o request do cliente
    request = recv(sock, client_reply, BUFFER_SIZE, 0);

    sem_wait(&mutex); // lock semaphore
    thread_count++; // incrementa qtd de threads

    if(thread_count > CONNECTION_NUMBER){ // testa que quantidade atual de threads nao excede limite. Se excede, manda mensagem de ocupado.
      puts("Conexão recusada. Limite da Thread excedido!");
      char *message = "HTTP/1.0 400 Bad Request\r\nContent-Type: text/html\r\n\r\n<!doctype html><html><body>System is busy right now.</body></html>";
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
      puts ("Mensagem recebida:");
      printf("%s", client_reply);
      request_lines[0] = strtok(client_reply, " \t\n"); // pega a primeira parte da requisicao GET
      if (strncmp(request_lines[0], "GET\0", 4) == 0){
        // separa as outras partes
        request_lines[1] = strtok(NULL, " \t"); // endereco
        request_lines[2] = strtok(NULL, " \t\n"); // protocolo

        if (strncmp(request_lines[2], "HTTP/1.0", 8) != 0 && strncmp(request_lines[2], "HTTP/1.1", 8) != 0){ // Bad request se n for tipo HTTP
          puts("Pedido errado.");
          char *message = "HTTP/1.0 400 Bad Request\r\nConnection: close\r\nContent-Type: text/html\r\n\r\n<!doctype html><html><body>400 Bad Request</body></html>";
          write(sock, message, strlen(message));
        } else  {
            char *tokens[2]; // para dividir em nome do arquivo e extensao

            file_name = (char *)malloc(strlen(request_lines[1]) * sizeof(char));
            strcpy(file_name, request_lines[1]);
            puts(file_name);

            // Getting the file name and extension
            tokens[0] = strtok(file_name, "."); //nome do arquivo
            tokens[1] = strtok(NULL, "."); // extensao

            if (tokens[0] == NULL || tokens[1] == NULL) { // se nao tem extensao ou request direto para localhost:PORT_NO
              puts("Pedido errado.");
              char *message = "HTTP/1.0 400 Bad Request\r\nConnection: close\r\n\r\n<!doctype html><html><body>400 Bad Request. (You need to request to image and text files)</body></html>";
              write(sock, message, strlen(message));
            } else if (strcmp(tokens[1], "html") == 0 || strcmp(tokens[1], "htm") == 0 || strcmp(tokens[1], "css") == 0 || strcmp(tokens[1], "js") == 0 || strcmp(tokens[1], "txt") == 0 ){
              sem_wait(&mutex); // Evita que duas ou mais threads façam operacoes de IO ao mesmo tempo - lock semaphore
              text_handler(sock, request_lines[1]); // chama func de ler arquivos texto
              sem_post(&mutex); // release semaphore
            } else if (strcmp(tokens[1], "jpeg") == 0 || strcmp(tokens[1], "jpg") == 0 || strcmp(tokens[1], "png") == 0 || strcmp(tokens[1], "bmp") == 0 || strcmp(tokens[1], "gif") == 0 || strcmp(tokens[1], "webp") == 0|| strcmp(tokens[1], "ico") == 0 ){
              sem_wait(&mutex); // lock semaphore -- deve-se estudar aqui. Sera porque n pode ter mais de uma thread tentando escrever no socket ao mesmo tempo?
              image_handler(sock, request_lines[1]); // chama func de ler arquivos de imagem (binarios?) -- existe apenas um semaforo, o q significa que só uma thread pode entrar nesses caminhos por vez, independente do cliente?
              sem_post(&mutex); // release semaphore
            } else { // extensao que nao pertence as de acima.
              char *message = "HTTP/1.0 400 Bad Request\r\nConnection: close\r\n\r\n<!doctype html><html><body>400 Bad Request. Not Supported File Type (Suppoerted File Types: html and jpeg)</body></html>";
              write(sock, message, strlen(message));
            }
          free(file_name);
        }
      }
    }

    //sleep(10); // Descomentar para conseguir enxergar o limite de threads.

    free(socket_desc);
    shutdown(sock, SHUT_RDWR); // encerra conexao do socket
    close(sock); // destroi socket
    sock = -1;
    sem_wait(&mutex); // pega lock para decrementar contador
    thread_count--;
    sem_post(&mutex);
    pthread_exit(NULL); // sai da thread
  }
