#include <stdlib.h>
#include <unistd.h>    // for write
#include "servidor2.h"
#include "manipulaSocketImg.h"

//importante: ao tentar acessar imagens em outra pasta, ele esta dando erro de corrupted_size vs prev_size. Tem a ver com sobreescrita de tamanhos..
// possivelmente problema com alocação da memória? Acontece quando html chama as imagens.
void image_handler(int socket, char *file_name, char *ext){ // le e escreve arquivos de imagem no socket
      puts("===== Entrou em image handler =====");

      char *tok, *nnfile, *ponto;
      char *nfile = (char*)malloc((strlen(file_name)-1)*sizeof(char)); // aloca espaço para nfile, com tamanho -1 de filename
      int fp;
      int flg = 0;
      char pt = '.';

      strcpy(nfile,&file_name[1]); // copia conteudo de filename a partir da 2 posicao para n pegar /
      printf("%s\n",nfile );

      tok = strtok(nfile,"/");// procura por uma / no meio da string
      if(tok != NULL){ // se encontra outro / em nfile, entao existe subpasta
          puts ("Encontrou uma subpasta");
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
        puts("Imagem Encontrada.");
        int bytes;
        char buffer[BUFFER_SIZE];
        char *mfull = (char*)malloc((strlen("HTTP/1.1 200 OK\r\nContent-Type: image/")+strlen(ext)+strlen("\r\nConnection: keep-alive\r\n\r\n"))*sizeof(char));
        mfull[0] = '\0'; // inicializando mfull com string vazia para remover lixos de malloc
        strcat(mfull,"HTTP/1.1 200 OK\r\nContent-Type: image/");
        strcat(mfull,ext);
        strcat(mfull, "\r\nConnection: keep-alive\r\n\r\n");
        puts(mfull); // pequena alteração aqui para fazer com que a mensagem do server seja de acordo com a extensao. Antes estava fixo jpeg.

        send(socket, mfull, strlen(mfull), 0);// send é um write especifico de socket, com mais opções, mas tem mesma função de write
        while ( (bytes=read(fp, buffer, BUFFER_SIZE))>0 ) // ler o arquivo no buffer enquanto nao chega no fim
  			write (socket, buffer, bytes); // envia o conteudo no buffer para o socket
      } else { //  nao achou arquivo
          puts("Imagem não encontrada no Servidor! - 404 not found");
          write(socket, "HTTP/1.1 404 Not Found\r\nConnection: close\r\nContent-Type: text/html\r\n\r\n<!doctype html><html><body>404: File Not Found :(</body></html>", strlen("HTTP/1.1 404 Not Found\r\nConnection: close\r\nContent-Type: text/html\r\n\r\n<!doctype html><html><body>404: File Not Found :(</body></html>"));
        }
        puts("===== Fim da resposta =====\n");
      close(fp);
      free(nfile);
      //free(nnfile); // estava dando erro aqui tbm, por isso comentado..
      //free(ponto);
}
