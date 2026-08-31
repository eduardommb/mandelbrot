# Mandelbrot

Gerador do Conjunto de Mandelbrot em C com suporte a execução sequencial e paralela em memória compartilhada. Implementa quatro estratégias de processamento (Serial, OpenMP, Pthreads com divisão por blocos e Pthreads com divisão intercalada), gerando as imagens resultantes em formato PGM e gravando o tempo de processamento de cada abordagem no arquivo `times.txt`.

## Arquivos

- `src/main.c` — todo o código (cálculo do pixel, rotinas serial/OpenMP/Pthreads, temporização e I/O)
- `Makefile` — compilação com flags de otimização (`-O3`, `-fopenmp`, `-pthread`) e limpeza

## Como compilar

```sh
make
```

Limpar os arquivos compilados e saídas geradas:

```sh
make clean
```

## Como executar

```sh
./mandelbrot <largura> <altura> <max_iteracoes> <num_threads>
```

Exemplo:

```sh
./mandelbrot 1200 800 1000 4
```

Parâmetros:

- `<largura>` — resolução horizontal da imagem em pixels
- `<altura>` — resolução vertical da imagem em pixels
- `<max_iteracoes>` — limite máximo de iterações por pixel
- `<num_threads>` — quantidade de threads para as execuções paralelas

## Arquivos gerados

```text
mandelbrot_lemb_serial.pgm    imagem gerada pela versão sequencial
mandelbrot_lemb_openmp.pgm    imagem gerada pela versão OpenMP
mandelbrot_lemb_pthreads1.pgm imagem gerada com Pthreads (divisão por blocos)
mandelbrot_lemb_pthreads2.pgm imagem gerada com Pthreads (divisão intercalada)
times.txt                     registro dos tempos de execução de cada modo
```
