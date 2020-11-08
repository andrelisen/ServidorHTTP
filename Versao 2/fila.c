#include <stdio.h>
#include <stdlib.h>
#include "fila.h"
//primeiro que entra é o último que sai - fila
//tamanho até o momento será igual ao limite de clientes aguardando conexão

//variáveis globais de controle da fila e limite dela
#define tamanhoFila 20
int tamAtualFila = 0;

void inicia(node *FILA)
{
    FILA->prox = NULL;
    tamAtualFila=0;
}

int vazia(node *FILA)
{
    if(FILA->prox == NULL){
        return 1;    
    }else{
        return 0;
    }
}

node *aloca(int id, struct sockaddr_in *dadosSocket)
{
    node *novo=(node *) malloc(sizeof(node));
    if(!novo){
        printf("Sem memoria disponivel!\n");
        exit(1);
    }else{
        novo->idSocket = id;
        novo->socketCliente = dadosSocket;
        return novo;
    }
}

void insere(node *FILA, int id, struct sockaddr_in *dadosSocket)
{
    if(tamAtualFila == tamanhoFila || tamAtualFila > tamanhoFila){
        printf("Fila cheia, remove item do início da fila e insere novo no final!");
        node *tmp = retira(FILA);
        insere(tmp, id, dadosSocket);
    }else{
        node *novo=aloca(id, dadosSocket);
        novo->prox = NULL;

        if(vazia(FILA))
            FILA->prox=novo;
        else{
            node *tmp = FILA->prox;

            while(tmp->prox != NULL){
                tmp = tmp->prox;
                tmp->prox = novo;
            }
        }
        tamAtualFila++;
    }
}


node *retira(node *FILA)
{
    if(FILA->prox == NULL){
        printf("Fila ja esta vazia\n");
        return NULL;
    }else{
        node *tmp = FILA->prox;
        FILA->prox = tmp->prox;
        tamAtualFila--;
        return tmp;
    }
}


void exibe(node *FILA)
{
    if(vazia(FILA)){
        printf("Fila vazia!\n\n");
        exit(1) ;
    }

    node *tmp;
    tmp = FILA->prox;
    printf("Fila :");

    while( tmp != NULL){
        printf("ID DA FILA DE SOCKETS: %5d \n", tmp->idSocket);
        tmp = tmp->prox;
    }
}

void libera(node *FILA)
{
    if(!vazia(FILA)){
        node *proxNode, *atual;
        atual = FILA->prox;
        while(atual != NULL){
            proxNode = atual->prox;
            free(atual);
            atual = proxNode;
        }
    }
}

struct sockaddr_in *existeFila(node *FILA, int id){
    if(vazia(FILA)){
        printf("Fila vazia!\n\n");
        return NULL;
    }

    node *tmp;
    tmp = FILA->prox;
    printf("Fila :");

    while( tmp != NULL){
        if(tmp->idSocket == id){
            struct sockaddr_in *retorno = tmp->socketCliente;
            printf("ID do socket verificado é igual ao da struct: %i \n", tmp->idSocket);
            return retorno;
        }
        printf("ID DA FILA DE SOCKETS: %i \n", tmp->idSocket);
        tmp = tmp->prox;
    }

    return NULL;
}

int validaExistencia (node *FILA, int id){
    if(vazia(FILA)){
        printf("Fila vazia!\n\n");
        return 0;
    }

    node *tmp;
    tmp = FILA->prox;
    printf("Fila :");

    while( tmp != NULL){
        if(tmp->idSocket == id){
            printf("ID do socket verificado é igual ao da struct: %i \n", tmp->idSocket);
            return 1;
        }
        printf("ID DA FILA DE SOCKETS: %i \n", tmp->idSocket);
        tmp = tmp->prox;
    }

    return 0;
}
