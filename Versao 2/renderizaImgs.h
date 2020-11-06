//importante: ao tentar acessar imagens em outra pasta, ele esta dando erro de corrupted_size vs prev_size. Tem a ver com sobreescrita de tamanhos..
// possivelmente problema com alocação da memória? Acontece quando html chama as imagens.
void image_handler(int socket, char *file_name);