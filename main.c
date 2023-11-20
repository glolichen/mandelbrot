#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <omp.h>

#include "native.h"
#include "linkedlist.h"

#ifdef __SSE__
	#define SSE_SUPPORT true
#else
	#define SSE_SUPPORT false
#endif

#ifdef __AVX2__
	#define AVX2_SUPPORT true
#else
	#define AVX2_SUPPORT false
#endif

#define AVX512_SUPPORT false

const bool SIMD_SUPPORT[] = {
	true,
	SSE_SUPPORT,
	AVX2_SUPPORT,
	AVX512_SUPPORT
};

const char *HELP_TEXT = "\
Usage: mandelbrot [RESOLUTION] [START REAL] [START IMAG] [END REAL] [END IMAG] [OPTION]...\n\
Generate an image of the mandelbrot set from (START REAL, START IMAG) to (END REAL, END IMAG).\n\n\
START REAL must be less than END REAL and START IMAG muct be less than END IMAG.\n\
The length of the longer side of the image is provided by the RESOLUTION argument.\n\
The length of the shorter side is calculate by the program.\n\
The default name of the PNG image is mandelbrot.png, however, the name can be user-specified.\n\
Options must be placed after the required parameters.\n\
\n\
Options:\n\
 -i, --iterations [n] Assume membership in mandelbrot set after [n] iterations.\n\
                      Default: 1000\n\
 -t, --threads [n]    Use [n] threads.\n\
                      Default: maximum on computer\n\
 -o, --out [name]     The output file is called [name].\n\
                      Default: mandelbrot.png\n\
 --simd [set]         The Single Instruction, Multiple Data instruction set to\n\
                      use. Either sse, avx2, avx512, or none.\n\
					  avx512 can actually be slower for some reason\n\
					  Default: avx2 if supported, else sse, else none\n\
";

// https://stackoverflow.com/a/76866890
int get_threads() {
	int num_threads = 1;
	#pragma omp parallel
	{
		#pragma omp single
		num_threads = omp_get_num_threads();
	}
	return num_threads;
}

int main(int argc, char *argv[]) {
	// __m512d a = _mm512_setr_pd(-0.53, -0.55, -1.04, -0.15, 0.2, 0, 0, 0);
	// __m512d b = _mm512_setr_pd(0.62, 0.55,  0.27,  0.32, -0.11, 0, 0, 0);
	// // __mmask64 mask = _mm512_cmp_pd_mask(a, b, _CMP_GE_OS);
	// __m512i out = get_iterations_avx512(&a, &b, 6, 1000);
	// for (int i = 0; i < 8; i++)
	// 	printf("%d ", out[i]);
	// printf("\n");

	// return 0;

	assert(argc >= 2);

	if (strcmp(argv[1], "--help") == 0) {
		printf("%s", HELP_TEXT);
		return 0;
	}

	assert(argc >= 6);

	int userProvided = atoi(argv[1]);
	double remin = atof(argv[2]);
	double immin = atof(argv[3]);
	double remax = atof(argv[4]);
	double immax = atof(argv[5]);

	assert(remin < remax);
	assert(immin < immax);

	double wthRatio = (atof(argv[4]) - atof(argv[2])) / (atof(argv[5]) - atof(argv[3]));
	int width, height;

	int threads = -1, iters = 1000, instructions = -1;
	char *fileName = NULL;
	for (int i = 6; i < argc; i++) {
		char *current = argv[i];
		if (strcmp(current, "-i") == 0 || strcmp(current, "--iterations") == 0) {
			assert(i + 1 < argc);
			iters = atoi(argv[++i]);
		}
		else if (strcmp(current, "-t") == 0 || strcmp(current, "--threads") == 0) {
			assert(i + 1 < argc);
			threads = atoi(argv[++i]);
		}
		else if (strcmp(current, "-o") == 0 || strcmp(current, "--out") == 0) {
			assert(i + 1 < argc);
			fileName = argv[++i];
		}
		else if (strcmp(current, "--simd") == 0) {
			assert(i + 1 < argc);
			i++;
			if (strcmp(argv[i], "none") == 0)
				instructions = None;
			else if (strcmp(argv[i], "sse") == 0)
				instructions = SSE;
			else if (strcmp(argv[i], "avx2") == 0)
				instructions = AVX2;
			else if (strcmp(argv[i], "avx512") == 0)
				instructions = AVX512;
		}
	}
	if (threads == -1)
		threads = get_threads();
	if (fileName == NULL)
		fileName = "mandelbrot.png";
	if (instructions == -1) {
		for (int i = 3; i >= 0; i--) {
			if (SIMD_SUPPORT[i]) {
				instructions = i;
				break;
			}
		}
	}

	if (wthRatio >= 1) {
		width = userProvided;
		height = userProvided / wthRatio;
	}
	else {
		height = userProvided;
		width = userProvided * wthRatio;
	}

	int res = write_png(fileName, width, height, remin, immin, remax, immax, iters, threads, instructions);
	return res;
}
