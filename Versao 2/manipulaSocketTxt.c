#include <stdlib.h>
#include <unistd.h>    // for write
#include "servidor2.h"
#include "manipulaSocketTxt.h"

void text_handler(int socket, char *file_name, char *ext){ // le e escreve arquivos de texto no socket
      puts("===== Entrou text handler =====");

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
        puts ("Achou subpasta");
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
          puts("Arquivo encontrado.");

          fseek(fp, 0, SEEK_END); // procura tamanho do arquivo
          long bytes_read = ftell(fp);
          fseek(fp, 0, SEEK_SET);

          char *mfull = (char*)malloc((strlen("HTTP/1.1 200 OK\r\nContent-Type: text/")+strlen(ext)+strlen("\r\nConnection: keep-alive\r\n\r\n"))*sizeof(char));
          mfull[0] = '\0'; // inicializando mfull com string vazia para remover lixos de malloc
          strcat(mfull,"HTTP/1.1 200 OK\r\nContent-Type: text/");
          strcat(mfull,ext);
          strcat(mfull, "\r\nConnection: keep-alive\r\n\r\n");
          puts(mfull); // pequena alteração aqui para fazer com que a mensagem do server seja de acordo com a extensao. Antes estava fixo jpeg.

          send(socket, mfull, strlen(mfull), 0); // Envia cabeçalho de resposta bem sucedida
          buffer = (char *)malloc(bytes_read * sizeof(char)); // aloca buffer com tamanho do arquivo

          fread(buffer, bytes_read, 1, fp); //  ler o arquivo texto no buffer
          write (socket, buffer, bytes_read); // Envia o conteudo do buffer para o socket
          free(buffer);

          fclose(fp);
      } else { // não achou arquivo
          puts("Arquivo não encontrado no servidor!");
          write(socket, "HTTP/1.1 404 Not Found\r\nConnection: close\r\nContent-Type: text/html\r\n\r\n<!doctype html><html><body>404: File Not Found :(</body></html>", strlen("HTTP/1.1 404 Not Found\r\nConnection: close\r\nContent-Type: text/html\r\n\r\n<!doctype html><html><body>404: File Not Found :(</body></html>"));
      }
      puts("===== Fim da resposta =====\n");
      free(nfile);
      free(nnfile);
      free(ponto);
  }
