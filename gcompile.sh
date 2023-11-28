gcc main.c base.c nosimd.c sse.c avx2.c gmp.c native.c linkedlist.c -o mandelbrot \
	-Wall -Wextra -Wpedantic -pthread -lpng -lm -lgmp \
	-mavx2 -mtune=native -march=native -pthread \
	-fsanitize=undefined \
	-O3 \
	# -O0 -g \