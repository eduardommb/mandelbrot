mandelbrot: src/main.c ; cc -std=c11 -Wall -Wextra -O3 -fopenmp -pthread src/main.c -o mandelbrot -lm
clean: ; rm -f mandelbrot *.o *.pgm times.txt
