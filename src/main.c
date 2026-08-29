#include <stdio.h>
#include <stdlib.h>

int main (int argc, char *argv[])
{
    /* verifica qtd de parametros */
    if (argc != 5)
    {
        fprintf(stderr, "erro: use ./mandelbrot <largura> <altura> <max_iteracoes> <num_threads>\n");
        return 1;
    }

    /* converte p/ inteiro */
    int largura = atoi(argv[1]);
    int altura = atoi(argv[2]);
    int max_iteracoes = atoi(argv[3]);
    int num_threads = atoi(argv[4]);

    /* nenhum num. negativo ou igual a zero */
    if (largura <= 0 || altura <= 0 || max_iteracoes <= 0 || num_threads <= 0)
    {
        fprintf(stderr, "erro: todos os valores devem ser maiores que zero.\n");
        return 1;
    }

    /* alocacao da matriz de pixels */
    size_t total_pixels = (size_t)largura * altura;
    unsigned char *imagem = (unsigned char *)malloc(total_pixels * sizeof(unsigned char));

    /* tratamento de erro na alocacao */
    if (imagem == NULL)
    {
        fprintf(stderr, "erro: falha ao alocar memoria para imagem");
    }

    free(imagem);

    return 0;
}
