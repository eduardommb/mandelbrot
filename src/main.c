#include <stdio.h>
#include <stdlib.h>

unsigned char calcular_pixel (int x, int y, int altura, int largura, int max_iteracoes)
{
    /* converte pixel p/ plano complexo */
    double cr = -2.0 + ((double)x / largura) * 3.0;
    double ci = -1.5 + ((double)y / altura) * 3.0;

    double zr = 0.0;
    double zi = 0.0;
    int iter = 0;

    while (iter < max_iteracoes && (zr * zr + zi * zi) <= 4.0)
    {
        double zr_temp = zr * zr - zi * zi + cr;
        zi = 2.0 * zr * zi + ci;
        zr = zr_temp;
        iter++;
    }

    return (unsigned char)(((double)iter / max_iteracoes) * 255.0);
}

void serial(unsigned char *imagem, int largura, int altura, int max_iteracoes)
{
    for (int y = 0; y < altura; y++)
    {
        for (int x = 0; x < largura; x++)
        {
            imagem[y * largura + x] = calcular_pixel(x, y, largura, altura, max_iteracoes);
        }
    }
}


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
