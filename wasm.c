#include <math.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "wasm.h"
#include "base.h"
#include "nosimd.h"
#include "sse.h"
#include "linkedlist.h"

#include <stdint.h>

int multiply_array(double factor, double *arr, int length) {
	for (int i = 0; i <  length; i++) {
		arr[i] = factor * arr[i];
	}
	return 0;
}

void do_calculation_wasm(int *status, int width, int height, 
			  double remin, double immin, double remax, double immax,	
			  int max_iters, int thread_count, int instructions) {

	double realChange, imagChange, change;
	realChange = (remax - remin) / width;
	imagChange = (immax - immin) / height;
	change = (realChange + imagChange) / 2;

	for (int i = 0; i < thread_count; i++) {
		int topHeight = i * height / thread_count;
		int bottomHeight = (i + 1) * height / thread_count;
		switch (instructions) {
			case None:
				printf("none\n");
				do_calculation_naive(status, remin, immax, change, width, topHeight, bottomHeight, max_iters);
				break;
			case SSE:
				printf("sse\n");
				do_calculation_sse(status, remin, immax, change, width, topHeight, bottomHeight, max_iters);
				break;
			// case AVX2:
			// 	do_calculation_avx2(status, remin, immax, change, width, topHeight, bottomHeight, max_iters);
			// 	break;
		}
	}
}
