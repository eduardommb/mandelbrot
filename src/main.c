#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <pthread.h>






// ==================
// CALCULO DO PIXEL
// ==================
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






// ==================
// TEMPO E ARQUIVOS
// ==================
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






// ==================
// SERIAL
// ==================
//
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






// ==================
// OPENMP
// ==================
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






// ==================
// PTHREADS 1 (BLOCOS)
// ==================

typedef struct
{
    unsigned char *imagem;
    int largura;
    int altura;
    int max_iteracoes;
    int y_inicio;
    int y_fim;
} ThreadBlock;

void *worker_pthreads1(void *arg)
{
    ThreadBlock *dados = (ThreadBlock *)arg;

    for (int y = dados->y_inicio; y < dados->y_fim; y++)
    {
        for (int x = 0; x < dados->largura; x++)
        {
            dados->imagem[y * dados->largura + x] = calcular_pixel(x, y, dados->largura, dados->altura, dados->max_iteracoes);
        }
    }

    pthread_exit(NULL);
}






int pthreads1(unsigned char *imagem, int largura, int altura, int max_iteracoes, int num_threads)
{
    pthread_t *threads = (pthread_t *)malloc(num_threads * sizeof(pthread_t));
    ThreadBlock *args = (ThreadBlock *)malloc(num_threads * sizeof(ThreadBlock));

    if (threads == NULL || args == NULL) {
        fprintf(stderr, "Erro: Falha na alocacao de memoria para Pthreads 1.\n");
        free(threads);
        free(args);
        return 0;
    }

    int linhas_por_thread = altura / num_threads;
    int resto = altura % num_threads;
    int linha_atual = 0;

    for (int i = 0; i < num_threads; i++)
    {
        args[i].imagem = imagem;
        args[i].largura = largura;
        args[i].altura = altura;
        args[i].max_iteracoes = max_iteracoes;
        args[i].y_inicio = linha_atual;

        /* distribui o resto da divisao para a ultima thread cobrir toda a imagem */
        int linhas_dessa_thread = linhas_por_thread + (i == num_threads - 1 ? resto : 0);
        args[i].y_fim = linha_atual + linhas_dessa_thread;
        linha_atual = args[i].y_fim;

        if (pthread_create(&threads[i], NULL, worker_pthreads1, (void *)&args[i]) != 0)
        {
            fprintf(stderr, "Erro: Falha ao criar a thread %d no Pthreads 1.\n", i);
            free(threads);
            free(args);
            return 0;
        }
    }

    /* aguarda todas as threads terminarem */
    for (int i = 0; i < num_threads; i++)
    {
        pthread_join(threads[i], NULL);
    }
    free(threads);
    free(args);
    return 1;
}






// ==================
// PTHREADS 2 (INTERCALADA)
// ==================
typedef struct {
    unsigned char *imagem;
    int largura;
    int altura;
    int max_iteracoes;
    int thread_id;
    int num_threads;
} ThreadIntercalada;

void *worker_pthreads2(void *arg) {
    ThreadIntercalada *dados = (ThreadIntercalada *)arg;

    for (int y = dados->thread_id; y < dados->altura; y += dados->num_threads) {
        for (int x = 0; x < dados->largura; x++) {
            dados->imagem[y * dados->largura + x] = calcular_pixel(x, y, dados->largura, dados->altura, dados->max_iteracoes);
        }
    }

    pthread_exit(NULL);
}

int pthreads2(unsigned char *imagem, int largura, int altura, int max_iteracoes, int num_threads) {

    pthread_t *threads = (pthread_t *)malloc(num_threads * sizeof(pthread_t));
    ThreadIntercalada *args = (ThreadIntercalada *)malloc(num_threads * sizeof(ThreadIntercalada));

    if (threads == NULL || args == NULL) {
        fprintf(stderr, "Erro: Falha na alocacao de memoria para Pthreads 2.\n");
        free(threads);
        free(args);
        return 0;
    }

    for (int i = 0; i < num_threads; i++) {
        args[i].imagem = imagem;
        args[i].largura = largura;
        args[i].altura = altura;
        args[i].max_iteracoes = max_iteracoes;
        args[i].thread_id = i;
        args[i].num_threads = num_threads;

        if (pthread_create(&threads[i], NULL, worker_pthreads2, (void *)&args[i]) != 0) {
            fprintf(stderr, "Erro: Falha ao criar a thread %d no Pthreads 2.\n", i);
            free(threads);
            free(args);
            return 0;
        }
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    free(threads);
    free(args);
    return 1;
}






// ==================
// MAIN
// ==================

int main (int argc, char *argv[])
{
    if (argc != 5)
    {
        fprintf(stderr, "Erro: use ./mandelbrot <largura> <altura> <max_iteracoes> <num_threads>\n");
        return 1;
    }

    int largura = atoi(argv[1]);
    int altura = atoi(argv[2]);
    int max_iteracoes = atoi(argv[3]);
    int num_threads = atoi(argv[4]);

    if (largura <= 0 || altura <= 0 || max_iteracoes <= 0 || num_threads <= 0)
    {
        fprintf(stderr, "Erro: todos os valores devem ser maiores que zero.\n");
        return 1;
    }

    size_t total_pixels = (size_t)largura * altura;
    unsigned char *imagem = (unsigned char *)malloc(total_pixels * sizeof(unsigned char));

    if (imagem == NULL)
    {
        fprintf(stderr, "Erro: falha ao alocar memoria para imagem");
        return 1;
    }

    double tempo_serial = 0.0;
    double tempo_openmp = 0.0;
    double tempo_pth1   = 0.0;
    double tempo_pth2   = 0.0;




    // ==================
    // SERIAL/OPENMP/PTHREADS_1/PTHREADS_2
    // ==================



    // Serial //
    double inicio = pegaTempo();
    serial(imagem, largura, altura, max_iteracoes);
    double fim = pegaTempo();
    tempo_serial = fim - inicio;

    if (!salvarImagem("mandelbrot_lemb_serial.pgm", imagem, largura, altura))
    {
        free(imagem);
        return 1;
    }



    // OpenMP //
    inicio = pegaTempo();
    openmp(imagem, largura, altura, max_iteracoes, num_threads);
    fim = pegaTempo();
    tempo_openmp = fim - inicio;

    if (!salvarImagem("mandelbrot_lemb_openmp.pgm", imagem, largura, altura))
    {
        free(imagem);
        return 1;
    }



    // Pthreads 1 //
    inicio = pegaTempo();
    if (!pthreads1(imagem, largura, altura, max_iteracoes, num_threads))
    {
        free(imagem);
        return 1;
    }
    fim = pegaTempo();
    tempo_pth1 = fim - inicio;

    if(!salvarImagem("mandelbrot_lemb_pthreads1.pgm", imagem, largura, altura))
    {
        free(imagem);
        return 1;
    }



    // 4. Pthreads 2
    inicio = pegaTempo();
    if (!pthreads2(imagem, largura, altura, max_iteracoes, num_threads)) {
        free(imagem);
        return 1;
    }
    fim = pegaTempo();
    tempo_pth2 = fim - inicio;

    if (!salvarImagem("mandelbrot_lemb_pthreads2.pgm", imagem, largura, altura))
    {
        free(imagem);
        return 1;
    }

    return 0;
}
