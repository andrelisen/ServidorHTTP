
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

#define PORT_NO 8080 // numero da porta
#define BUFFER_SIZE 1024 // tamanho buffer do socket
#define CONNECTION_NUMBER 20 // quantidade maxima de threads simultaneas

#define EOL "\r\n" // end of line
#define EOL_SIZE 2

  int thread_count = 0; //contador do numero de threads ativas ao mesmo tempo
  sem_t mutex; // para controlar o contador de threads

//importante: ao tentar acessar imagens em outra pasta, ele esta dando erro de corrupted_size vs prev_size. Tem a ver com sobreescrita de tamanhos..
// possivelmente problema com alocação da memória? Acontece quando html chama as imagens.
  void image_handler(int socket, char *file_name){ // le e escreve arquivos de imagem no socket
      char *tok, *nnfile, *ponto;
      char *nfile = (char*)malloc((strlen(file_name)-1)*sizeof(char)); // aloca espaço para nfile, com tamanho -1 de filename
      int fp;
      int flg = 0;
      char pt = '.';

      strcpy(nfile,&file_name[1]); // copia conteudo de filename a partir da 2 posicao para n pegar /
      printf("%s\n",nfile );

      tok = strtok(nfile,"/");// procura por uma / no meio da string
      if(tok != NULL){ // se encontra outro / em nfile, entao existe subpasta
          puts ("Found subfolders");
          ponto = (char*)malloc((strlen(file_name)+1)*sizeof(char)); // aumenta o tamanho para 1  a mais q file_name
          ponto[0] = '\0';//inicializa ponto com \0 indicando string vazia
          nnfile = (char*)malloc((strlen(file_name))*sizeof(char));
          strncat(ponto,&pt,1);
          strcpy(nnfile,file_name); // copia conteudo de file_name
          strcat(ponto,nnfile); // coloca na frente o . para string ficar: ./pasta/arquivo.ext
          flg = 1;
          printf("ponto: %s\n", ponto);
      }

      if (!flg) // nao sei se isso aqui esta fazendo certo, parece que  ele esta entrando no if do tok sempre
        fp = open(nfile, O_RDONLY); // mas seria para escolher qual string abrir, dependendo se tem subpasta ou n
      else
        fp = open(ponto, O_RDONLY);

      if (fp > 0){ //se fp nao é 0 então achou o arquivo
          puts("Image Found.");
          int bytes;
          char buffer[BUFFER_SIZE];

          send(socket, "HTTP/1.0 200 OK\r\nContent-Type: image/jpeg\r\n\r\n", 45, 0);
  	      while ( (bytes=read(fp, buffer, BUFFER_SIZE))>0 ) // ler o arquivo no buffer enquanto nao chega no fim
  			  write (socket, buffer, bytes); // envia o conteudo no buffer para o socket
      } else { //  nao achou arquivo
          puts("Image not found in the server!");
          write(socket, "HTTP/1.0 404 Not Found\r\nConnection: close\r\nContent-Type: text/html\r\n\r\n<!doctype html><html><body>404 File Not Found</body></html>", strlen("HTTP/1.0 404 Not Found\r\nConnection: close\r\nContent-Type: text/html\r\n\r\n<!doctype html><html><body>404 File Not Found</body></html>"));
      }
      close(fp);
      free(nfile);
      //free(nnfile);
      //free(ponto);
  }

  void text_handler(int socket, char *file_name) // le e escreve arquivos de texto no socket
  {
      char *buffer, *nfile, *nnfile, *tok;
      int flg = 0;
      nfile = (char*)malloc((strlen(file_name)-1)*sizeof(char)); //alocando mem para nfile tamanho -1 para remover /
      FILE *fp;
      char *ponto;
      char pt = '.';

      strcpy(nfile,&file_name[1]); //copia conteudo de file_name para nfile a partir da 2 posicao, removendo / -> para arquivos na mesma pasta
      printf("%s\n",nfile );

      tok = strtok(nfile,"/"); // procura por uma outra  /  na string
      if(tok != NULL){ // se encontra outra / em nfile, entao existe subpasta
        puts ("Found subfolders");
          ponto = (char*)malloc((strlen(file_name)+1)*sizeof(char)); // aumenta o tamanho para 1  a mais q file_name
          ponto[0] = '\0'; // inicializa com \0, string vazia (necessario por malloc deixar lixo)
          nnfile = (char*)malloc((strlen(file_name))*sizeof(char));
          strncat(ponto,&pt,1); // coloca . na string ponto
          strcpy(nnfile,file_name); // copia conteudo de file_name
          strcat(ponto,nnfile); // concatena ponto . com nome do arquivo para string ficar: ./pasta/arquivo.ext
          flg = 1; // flag de subpasta
          printf("ponto: %s\n", ponto);
      }

      if (!flg)
        fp = fopen(nfile, "r");
      else
        fp = fopen(ponto, "r");

      if (fp != NULL){ // encontrou arquivo
          puts("File Found.");

          fseek(fp, 0, SEEK_END); // procura tamanho do arquivo
          long bytes_read = ftell(fp);
          fseek(fp, 0, SEEK_SET);

          send(socket, "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n", 44, 0); // Envia cabeçalho de resposta bem sucedida
          buffer = (char *)malloc(bytes_read * sizeof(char)); // aloca buffer com tamanho do arquivo

          fread(buffer, bytes_read, 1, fp); //  ler o arquivo texto no buffer
          write (socket, buffer, bytes_read); // Envia o conteudo do buffer para o socket
          free(buffer);

          fclose(fp);
      } else { // não achou arquivo
          puts("File not found in the server!");
          write(socket, "HTTP/1.0 404 Not Found\r\nConnection: close\r\nContent-Type: text/html\r\n\r\n<!doctype html><html><body>404 File Not Found</body></html>", strlen("HTTP/1.0 404 Not Found\r\nConnection: close\r\nContent-Type: text/html\r\n\r\n<!doctype html><html><body>404 File Not Found</body></html>"));
      }
      free(nfile);
      free(nnfile);
      free(ponto);
  }



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
      puts("Connection refused. Thread limit exceeded");
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
      puts("Recv failed");
    } else if (request == 0){ // socket fechado, cliente desconectou.
      puts("Client disconnected upexpectedly.");
    } else { // mensagem recebida
      puts ("Message received:");
      printf("%s", client_reply);
      request_lines[0] = strtok(client_reply, " \t\n"); // pega a primeira parte da requisicao GET
      if (strncmp(request_lines[0], "GET\0", 4) == 0){
        // separa as outras partes
        request_lines[1] = strtok(NULL, " \t"); // endereco
        request_lines[2] = strtok(NULL, " \t\n"); // protocolo

        if (strncmp(request_lines[2], "HTTP/1.0", 8) != 0 && strncmp(request_lines[2], "HTTP/1.1", 8) != 0){ // Bad request se n for tipo HTTP
          puts("Wrong request.");
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
              puts("Wrong request");
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


  int main(int argc, char *argv[]){
    sem_init(&mutex, 0, 1); // Inıcializa mutex com 1.
    int socket_desc, new_socket, c, *new_sock;
    struct sockaddr_in server, client;

    socket_desc = socket(AF_INET, SOCK_STREAM, 0); // cria socket - af_inet: ipv4 - sock stream: TCP - 0: IP
    if (socket_desc == -1){
      puts("Could not create socket");
      return 1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY; // localhost
    server.sin_port = htons(PORT_NO); // num porta processo

    if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0) { // vincula socket ao end e num porta especificados acima
      puts("Binding failed");
      return 1;
    }

    listen(socket_desc, 20); // inicia escuta por clientes - limite de 20 na fila
    puts("Waiting for incoming connections...");

    c = sizeof(struct sockaddr_in);
    while ((new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c))){ // aceita conexoes - salva em novo socket
      puts("Connection accepted \n");

      pthread_t sniffer_thread; // nova thread
      new_sock = (int*) malloc(1);
      *new_sock = new_socket;

      if (pthread_create(&sniffer_thread, NULL, connection_handler, (void *)new_sock) < 0){ // cria uma thread para cada requisicao, passando socket novo
        puts("Could not create thread");
        return 1;
      }
    }
    return 0;
  }
