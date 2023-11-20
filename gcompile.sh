gcc main.c base.c nosimd.c sse.c avx2.c native.c linkedlist.c -o mandelbrot \
	-Wall -Wextra -pedantic -fopenmp -lpng -lm \
	-mavx2 -mavx512f -mtune=native -march=native -O3