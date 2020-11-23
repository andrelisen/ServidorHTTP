

struct Node{
    int idSocket;
    struct Node *prox;
    struct sockaddr_in *socketCliente; 
};
typedef struct Node node;

void inicia(node *FILA);
int vazia(node *FILA);
node *aloca(int numero, struct sockaddr_in *dadosSocket);
void insere(node *FILA, int id, struct sockaddr_in *dadosSocket);
node *retira(node *FILA);
void exibe(node *FILA);
void libera(node *FILA);
struct sockaddr_in *existeFila(node *FILA, int id);
int validaExistencia (node *FILA, int id);
