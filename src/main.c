#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

double pegaTempo(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

int salvaTempo(const char *nome_arquivo, double t_serial, double t_openmp, double t_pth1, double t_pth2)
{
    FILE *arquivo = fopen(nome_arquivo, "w");
    if (arquivo == NULL)
    {
        fprintf(stderr, "erro: nao foi possivel criar o arquivo %s\n", nome_arquivo);
        return 0;
    }

    fprintf(arquivo, "Serial: %.6f s\n", t_serial);
    fprintf(arquivo, "OpenMP: %.6f s\n", t_openmp);
    fprintf(arquivo, "Pthreads 1: %.6f s\n", t_pth1);
    fprintf(arquivo, "Pthreads 2: %.6f s\n", t_pth2);

    fclose(arquivo);
    return 1;
}

int salvarImagem(const char *nome_arquivo, unsigned char *imagem, int largura, int altura)
{
    FILE *arquivo = fopen(nome_arquivo, "w");
    if (arquivo == NULL)
    {
        fprintf(stderr, "erro: nao foi possivel criar o arquivo %s", nome_arquivo);
        return 0;
    }

    for (int y = 0; y < altura; y++)
    {
        for (int x = 0; x < largura; x++)
        {
            if (x > 0)
            {
                fprintf(arquivo, " ");
            }
            fprintf(arquivo, "%u", imagem[y * largura + x]);
        }
        fprintf(arquivo, "\n");
    }

    fclose(arquivo);
    return 1;
}

unsigned char calcular_pixel (int x, int y, int largura, int altura, int max_iteracoes)
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

void openmp(unsigned char *imagem, int largura, int altura, int max_iteracoes, int num_threads)
{
    // define o numero de threads
    omp_set_num_threads(num_threads);

    // paraleliza as linhas da imagem com balanceamento dinamico de carga
    #pragma omp parallel for schedule(dynamic)
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
        return 1;
    }

    /* variaveis de tempo */
    double tempo_serial = 0.0;
    double tempo_openmp = 0.0;
    double tempo_pth1   = 0.0;
    double tempo_pth2   = 0.0;

    // Medição Serial
    double inicio = pegaTempo();
    serial(imagem, largura, altura, max_iteracoes);
    double fim = pegaTempo();
    tempo_serial = fim - inicio;
    //

    // Medição OpenMP
    inicio = pegaTempo();
    openmp(imagem, largura, altura, max_iteracoes, num_threads);
    fim = pegaTempo();
    tempo_openmp = fim - inicio;
    //

    if(!salvarImagem("mandelbrot_teste_serial.pgm", imagem, largura, altura))
    {
        free(imagem);
        return 1;
    }


    return 0;
}
