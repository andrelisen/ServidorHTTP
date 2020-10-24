
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

//#define PATH "/home/jiovana/Documentos/HTTPServerV1" // Last char of this PATH cannot be "/", please mind this. (PATH connot finish "/")
                                                        // For example PATH cannot be "/home/ozgur/workspace/assignment2/sources/"
#define PORT_NO 8888
#define BUFFER_SIZE 1024
#define CONNECTION_NUMBER 10

#define EOL "\r\n" // end of line
#define EOL_SIZE 2

typedef struct { // struct para organizar os tipos de arquivos
 char *ext;
 char *mediatype;
} extn;

//Possible media types
extn extensions[] ={
 {"gif", "image/gif" },
 {"txt", "text/plain" },
 {"jpg", "image/jpg" },
 {"jpeg","image/jpeg"},
 {"png", "image/png" },
 {"bmp", "image/bmp" },
 {"webp", "image/webp" },
 {"ico", "image/ico" },
 {"htm", "text/html" },
 {"html","text/html" },
 {"css", "text/css"},
 {"js", "text/javascript"},
 {"pdf","application/pdf"},
 {0,0} };

int thread_count = 0; //numero de threads ativas ao mesmo tempo
sem_t mutex; // para controlar o contador de threads

//
int get_file_size(int fd) {
 struct stat stat_struct;
 if (fstat(fd, &stat_struct) == -1)
  return (1);
 return (int) stat_struct.st_size;
}
//
void send_new(int fd, char *msg) {
 if (send(fd, msg, strlen(msg), 0) == -1)
  printf("Error in send\n");
}

 //recebe o buffer até achar EOL
int recv_buff(int fd, char *buffer) {
 char *p = buffer; // pointer to the buffer
 int eol_matched = 0; // check whether the rcv byte matched the buffer byte or not
 while (recv(fd, p, 1, 0) != 0) { // Start receiving 1 byte at a time
  if (*p == EOL[eol_matched]) { // if the byte matches with the first eol byte that is '\r'
   ++eol_matched;
   if (eol_matched == EOL_SIZE) { // if both the bytes matches with the EOL
    *(p + 1 - EOL_SIZE) = '\0'; // End the string
    return (strlen(buffer)); // Return the bytes recieved
   }
  } else {
   eol_matched = 0;
  }
  p++; // Increment the pointer to receive next byte
 }
 return (0);
}

// //metodo para lidar com arquivos de imagem
// void image_handler(int socket, char *file_name){
//     //char *buffer;
//   //  char *full_path = (char *)malloc((strlen(PATH) + strlen(file_name)) * sizeof(char));
//     int fp;
//
//   //  strcpy(full_path, PATH); // Merge the file name that requested and path of the root folder
//   //  strcat(full_path, file_name);
//     puts("Procurando por:");
//     puts(file_name);
//
//     if ((fp=open(file_name, O_RDONLY)) > 0) //arquivo encontrado
//     {
//         puts("Imagem encontrada!");
//         int bytes;
//         char buffer[BUFFER_SIZE];
//
//         send(socket, "HTTP/1.0 200 OK\r\nContent-Type: image/jpeg\r\n\r\n", 45, 0);
// 	    while ( (bytes=read(fp, buffer, BUFFER_SIZE))>0 ) // Read the file to buffer. If not the end of the file, then continue reading the file
// 			write (socket, buffer, bytes); // Send the part of the jpeg to client.
//     }
//     else // If there is not such a file.
//     {
//         write(socket, "HTTP/1.0 404 Not Found\r\nConnection: close\r\nContent-Type: text/html\r\n\r\n<!doctype html><html><body>404 File Not Found</body></html>", strlen("HTTP/1.0 404 Not Found\r\nConnection: close\r\nContent-Type: text/html\r\n\r\n<!doctype html><html><body>404 File Not Found</body></html>"));
//     }
//
//     //free(full_path);
//     close(fp);
// }

// void html_handler(int socket, char *file_name) // handle html files
// {
//     char *buffer;
//     char *full_path = (char *)malloc((strlen(PATH) + strlen(file_name)) * sizeof(char));
//     FILE *fp;
//
//     strcpy(full_path, PATH); // Merge the file name that requested and path of the root folder
//     strcat(full_path, file_name);
//
//     fp = fopen(file_name, "r");
//     if (fp != NULL) //FILE FOUND
//     {
//         puts("File Found.");
//
//         fseek(fp, 0, SEEK_END); // Find the file size.
//         long bytes_read = ftell(fp);
//         fseek(fp, 0, SEEK_SET);
//
//         send(socket, "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n", 44, 0); // Send the header for succesful respond.
//         buffer = (char *)malloc(bytes_read * sizeof(char));
//
//         fread(buffer, bytes_read, 1, fp); // Read the html file to buffer.
//         write (socket, buffer, bytes_read); // Send the content of the html file to client.
//         free(buffer);
//
//         fclose(fp);
//     }
//     else // If there is not such a file.
//     {
//         write(socket, "HTTP/1.0 404 Not Found\r\nConnection: close\r\nContent-Type: text/html\r\n\r\n<!doctype html><html><body>404 File Not Found</body></html>", strlen("HTTP/1.0 404 Not Found\r\nConnection: close\r\nContent-Type: text/html\r\n\r\n<!doctype html><html><body>404 File Not Found</body></html>"));
//     }
//
//     free(full_path);
// }

void file_handler(int socket, char *file_name, char *media_type){
  int fp;
  char *message = "HTTP/1.0 200 OK\r\nContent-Type: ";
  char *end = "\r\n\r\n";
  char *fullmessage = (char *) malloc((strlen(message) + strlen(media_type)+strlen(end)) * sizeof(char));
  strcpy(fullmessage, message);
  strcat(fullmessage, media_type);
  strcat(fullmessage,end);

  fp = open(file_name,O_RDONLY,0);
  if (fp > 0){
    int bytes;
    char buffer[BUFFER_SIZE];
    send(socket,fullmessage , 45, 0);
    while ( (bytes=read(fp, buffer, BUFFER_SIZE))>0 ) // Read the file to buffer. If not the end of the file, then continue reading the file
      write (socket, buffer, bytes); // Send the part of the file to client.
  } else {
      write(socket, "HTTP/1.0 404 Not Found\r\nConnection: close\r\nContent-Type: text/html\r\n\r\n<!doctype html><html><body>404 File Not Found</body></html>", strlen("HTTP/1.0 404 Not Found\r\nConnection: close\r\nContent-Type: text/html\r\n\r\n<!doctype html><html><body>404 File Not Found</body></html>"));
  }
  close(fp);
  free(fullmessage);
}

void *connection_handler(void *socket_desc){
    int request;
    char client_reply[BUFFER_SIZE], *request_lines[3];
    char *file_name;
    char *extension;

    // Get the socket descriptor.
    int sock = *((int *)socket_desc);

    // Get the request.
    request = recv(sock, client_reply, BUFFER_SIZE, 0);

    sem_wait(&mutex);
    thread_count++;

    if(thread_count > 10) // If there is 10 request at the same time, other request will be refused.
    {
        char *message = "HTTP/1.0 400 Bad Request\r\nContent-Type: text/html\r\n\r\n<!doctype html><html><body>System is busy right now.</body></html>";
        write(sock, message, strlen(message));
        thread_count--;
        sem_post(&mutex);
        free(socket_desc);
        shutdown(sock, SHUT_RDWR);
        close(sock);
        sock = -1;
        pthread_exit(NULL);
    }
    sem_post(&mutex);

    if (request < 0) { // Receive failed.
        puts("Recv failed");
    } else if (request == 0){ // receive socket closed. Client disconnected upexpectedly.
        puts("Client disconnected upexpectedly.");
    } else{ // Message received.
        printf("%s", client_reply);
        request_lines[0] = strtok(client_reply, " \t\n");
        if (strncmp(request_lines[0], "GET\0", 4) == 0){
            // Parsing the request header.
            request_lines[1] = strtok(NULL, " \t");
            request_lines[2] = strtok(NULL, " \t\n");

            if (strncmp(request_lines[2], "HTTP/1.0", 8) != 0 && strncmp(request_lines[2], "HTTP/1.1", 8) != 0){ // Bad request if not HTTP 1.0 or 1.1
                char *message = "HTTP/1.0 400 Bad Request\r\nConnection: close\r\nContent-Type: text/html\r\n\r\n<!doctype html><html><body>400 Bad Request</body></html>";
                write(sock, message, strlen(message));
            } else{
                char *tokens[2]; // For parsing the file name and extension.
                file_name = (char *)malloc(strlen(request_lines[1]) * sizeof(char));
                strcpy(file_name, request_lines[1]);
                puts(file_name);

                // Getting the file name and extension
                tokens[0] = strtok(file_name, ".");
                tokens[1] = strtok(NULL, ".");


                if (tokens[0] == NULL || tokens[1] == NULL) // If there is not an extension in request or request to just localhost:8888/
                {
                    char *message = "HTTP/1.0 400 Bad Request\r\nConnection: close\r\n\r\n<!doctype html><html><body>400 Bad Request. (You need to request to jpeg and html files)</body></html>";
                    write(sock, message, strlen(message));
                } else {
                  int i;
                  for (i = 0; extensions[i].ext != NULL; i++) {
                    printf ("%s\n", tokens[1]);
                    printf ("%s\n",  extensions[i].ext);
                    if (strcmp(tokens[1], "html") != 0) {
                        char *message = "HTTP/1.0 400 Bad Request\r\nConnection: close\r\n\r\n<!doctype html><html><body>400 Bad Request. Not Supported File Type (Supported File Types: html and jpeg)</body></html>";
                        write(sock, message, strlen(message));
                    } else{
                            sem_wait(&mutex); // Prevent two or more thread do some IO operation same time.
                            file_handler(sock, request_lines[1],extensions[i].mediatype);
                            sem_post(&mutex);
                    }
                  }

                    //free(extension);
                }
                free(file_name);
            }
        }
    }

    //sleep(10); // If you want to see just 10 thread is working simultaneously, you can sleep here.
    // After send 10 request, 11th request will be responded as "System is busy right now".

    free(socket_desc);
    shutdown(sock, SHUT_RDWR);
    close(sock);
    sock = -1;
    sem_wait(&mutex);
    thread_count--;
    sem_post(&mutex);
    pthread_exit(NULL);
}


int main(int argc, char *argv[])
{
    sem_init(&mutex, 0, 1); // Inıtialize the mutex from 1.
    int socket_desc, new_socket, c, *new_sock;
    struct sockaddr_in server, client;

    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1)
    {
        puts("Could not create socket");
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT_NO);

    if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        puts("Binding failed");
        return 1;
    }

    listen(socket_desc, 10);

    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    while ((new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c))) // Accept the connection.
    {
        puts("Connection accepted \n");

        pthread_t sniffer_thread;
        new_sock = (int*) malloc(1);
        *new_sock = new_socket;

        if (pthread_create(&sniffer_thread, NULL, connection_handler, (void *)new_sock) < 0) // Create a thread for each request.
        {
            puts("Could not create thread");
            return 1;
        }
    }

    return 0;
}
