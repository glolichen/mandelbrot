#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#include "mandelbrot.h"
#include "linkedlist.h"

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
	assert(argc >= 2);

	if (strcmp(argv[1], "--help") == 0) {
		printf("%s", HELP_TEXT);
		return 0;
	}

	assert(argc >= 6);

	int userProvided = atoi(argv[1]);
	double bottomLeftReal = atof(argv[2]);
	double bottomLeftImag = atof(argv[3]);
	double topRightReal = atof(argv[4]);
	double topRightImag = atof(argv[5]);

	assert(bottomLeftReal < topRightReal);
	assert(bottomLeftImag < topRightImag);

	double wthRatio = (0.0 + atof(argv[4]) - atof(argv[2])) / (0.0 + atof(argv[5]) - atof(argv[3]));
	int width, height;

	int threads = -1, iters = 1000;
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
	}
	if (threads == -1)
		threads = get_threads();
	if (fileName == NULL)
		fileName = "mandelbrot.png";

	if (wthRatio >= 1) {
		width = userProvided;
		height = userProvided / wthRatio;
	}
	else {
		height = userProvided;
		width = userProvided * wthRatio;
	}

	int res = write_png(fileName, width, height, bottomLeftReal, bottomLeftImag, topRightReal, topRightImag, iters, threads);
	return res;
}
