#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

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

const bool SIMD_SUPPORT[] = {
	true,
	SSE_SUPPORT,
	AVX2_SUPPORT
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
 --sse                Use SSE Single Instruction, Multiple Data instruction set.\n\
                      Returns 1 if SSE is unsupported on CPU.\n\
 --avx2               Use AVX2 SIMD instruction set.\n\
                      Returns 1 if SSE is unsupported on CPU.\n\
 --nosimd             Do not use SIMD. This is the default.\n\
 --gmp                Use GNU GMP arbitrary precision floating point numbers.\n\
";

// https://stackoverflow.com/a/76866890
int get_threads() {
	return 8;
}

int main(int argc, char *argv[]) {
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

	double wthRatio = (remax - remin) / (immax - immin);
	int width, height;

	bool gmp = false;
	int threads = -1, iters = 1000, instructions = None;
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
		else if (strcmp(current, "--nosimd") == 0)
			instructions = None;
		else if (strcmp(argv[i], "--sse") == 0)
			instructions = SSE;
		else if (strcmp(argv[i], "--avx2") == 0)
			instructions = AVX2;
		else if (strcmp(argv[i], "--gmp") == 0)
			gmp = true;
	}
	if (gmp && instructions != None) {
		fprintf(stderr, "Cannot use GMP with SIMD.\n");
		return 2;
	}
	if (threads == -1)
		threads = get_threads();
	if (fileName == NULL)
		fileName = "mandelbrot.png";
	if (!SIMD_SUPPORT[instructions]) {
		switch (instructions) {
			case SSE:
				fprintf(stderr, "SSE");
				break;
			case AVX2:
				fprintf(stderr, "AVX2");
				break;
		}
		fprintf(stderr, " is not supported on your OS or CPU.\n");
		return 1;
	}
	instructions = gmp ? GMP : instructions;

	if (wthRatio >= 1) {
		width = userProvided;
		height = userProvided / wthRatio;
	}
	else {
		height = userProvided;
		width = userProvided * wthRatio;
	}

	int res = write_png(fileName, width, height, remin, immin, remax, immax, iters, threads, instructions);
	if (res != 0)
		fprintf(stderr, "png writer failed with exit status %d", res);

	return res;
}
