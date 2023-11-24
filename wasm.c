#include <math.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <wasm_simd128.h>

#include "wasm.h"
#include "base.h"
#include "nosimd.h"
#include "sse.h"
#include "linkedlist.h"

v128_t oneWasm, twoWasm, fourWasm, quarterWasm;

int *get_iterations_wasm_simd(const v128_t *real, const v128_t *imag, int num_nums, int max_iters) {
	int *result = (int *) calloc(2 * sizeof(int), sizeof(int));

	v128_t x = *real, y = *imag;

	int skipCalculation = 0;

	// https://en.wikipedia.org/wiki/Plotting_algorithms_for_the_Mandelbrot_set#Cardioid_/_bulb_checking
	// but simd-ified
	// TODO: BAD! some preicsion problems that result in a screw up picture
	v128_t p = wasm_f64x2_add(wasm_f64x2_mul(wasm_f64x2_add(x, oneWasm), wasm_f64x2_add(x, oneWasm)), wasm_f64x2_mul(y, y));
	v128_t q = wasm_f64x2_add(wasm_f64x2_mul(wasm_f64x2_sub(x, quarterWasm), wasm_f64x2_sub(x, quarterWasm)), wasm_f64x2_mul(y, y));
	v128_t r = wasm_f64x2_mul(q, wasm_f64x2_add(q, wasm_f64x2_sub(x, quarterWasm)));
	v128_t s = wasm_f64x2_mul(quarterWasm, wasm_f64x2_mul(y, y));
	for (int i = 0; i < num_nums; i++) {
		if (p[i] <= 0.0625 || r[i] <= s[i]) {
			result[i] = 0;
			skipCalculation |= 1 << i;
		}
	}
	for (int i = num_nums; i < 2; i++)
		skipCalculation |= 1 << i;
	
	// algorithm from https://en.wikipedia.org/wiki/Plotting_algorithms_for_the_Mandelbrot_set#Continuous_(smooth)_coloring
	v128_t x2, y2, tempReal, norm;

	int prevEscaped = 0, justEscaped = 0;
	int iters = 1;
	for (; iters < max_iters; iters++) {
		x2 = wasm_f64x2_mul(x, x);
		y2 = wasm_f64x2_mul(y, y);
		tempReal = wasm_f64x2_add(wasm_f64x2_sub(x2, y2), *real);
		y = wasm_f64x2_add(wasm_f64x2_mul(wasm_f64x2_mul(x, y), twoWasm), *imag);
		x = tempReal;

		norm = wasm_f64x2_add(x2, y2);
		justEscaped = wasm_i64x2_bitmask(wasm_f64x2_ge(norm, fourWasm));
		for (int i = 0; i < num_nums; i++) {
			if ((justEscaped & (1 << i)) && !(prevEscaped & (1 << i)))
				result[i] = get_color(i == 0 ? wasm_f64x2_extract_lane(norm, 0) : wasm_f64x2_extract_lane(norm, 1), iters, max_iters);
		}
		prevEscaped |= justEscaped;
		// 0b11 (all true) = 3
		if ((prevEscaped | skipCalculation) == 3)
			return result;
	}

	return result;
}

void do_calculation_wasm_simd(int *status, double remin, double immax, double realChange, double imagChange, int width, int top_height, int bottom_height, int max_iters) {
	oneWasm = wasm_f64x2_splat(1);
	twoWasm = wasm_f64x2_splat(2);
	fourWasm = wasm_f64x2_splat(4);
	quarterWasm = wasm_f64x2_splat(0.25);
	
	v128_t remins = wasm_f64x2_splat(remin);
	v128_t immaxes = wasm_f64x2_splat(immax);
	v128_t realChanges = wasm_f64x2_splat(realChange);
	v128_t imagChanges = wasm_f64x2_splat(imagChange);
	
	int *iterses;

	LinkedList linkedList; // either a stack or deque
	make_linked_list(&linkedList);

	for (int i = 0; i < width; i++) {
		linked_list_add(&linkedList, top_height * (width - 1) + i);
		status[top_height * (width - 1) + i] = 1;
		linked_list_add(&linkedList, (bottom_height - 1) * width + i);
		status[(bottom_height - 1) * width + i] = 1;
	}
	for (int i = top_height + 1; i < bottom_height - 1; i++) {
		linked_list_add(&linkedList, i * width);
		status[i * width] = 1;
		linked_list_add(&linkedList, i * width + width - 1);
		status[i * width + width - 1] = 1;
	}

	int *cur, *cx, *cy, nx, ny, numPoppable, index;
	cur = (int *) malloc(2 * sizeof(int));
	cx = (int *) malloc(2 * sizeof(int));
	cy = (int *) malloc(2 * sizeof(int));

	v128_t reals, imags;

	while (linkedList.size != 0) {
		numPoppable = min(linkedList.size, 2);

		for (int i = 0; i < numPoppable; i++) {
			cur[i] = linked_list_pop_front(&linkedList);
			cx[i] = cur[i] % width, cy[i] = cur[i] / width;
		}
		for (int i = numPoppable; i < 2; i++)
			cur[i] = 0;

		reals = wasm_f64x2_add(remins, wasm_f64x2_mul(realChanges, wasm_f64x2_make(cx[0], cx[1])));
		imags = wasm_f64x2_sub(immaxes, wasm_f64x2_mul(imagChanges, wasm_f64x2_make(cy[0], cy[1])));

		iterses = get_iterations_wasm_simd(&reals, &imags, numPoppable, max_iters);
		for (int i = 0; i < numPoppable; i++) {
			status[cur[i]] = iterses[i];
			if (iterses[i] == 0)
				continue;

			for (int j = 0; j < 4; j++) {
				nx = cx[i] + DIRECTION[j][0];
				ny = cy[i] + DIRECTION[j][1];
				if (nx < 0 || ny < top_height || nx >= width || ny >= bottom_height)	
					continue;
				index = ny * width + nx;
				if (status[index] == 0) {
					status[index] = 16777216;
					linked_list_add(&linkedList, index);
				}
			}
		}
		free(iterses);
	}

	free(cur);
	free(cx);
	free(cy);

	free_linked_list(&linkedList);

}

enum { None, SSE, Wasm };

void do_calculation_wasm(int *status, int width, int height, 
			  double remin, double immin, double remax, double immax,	
			  int max_iters, int thread_count, int instructions) {

	clock_t before = clock();

	double realChange, imagChange;
	realChange = (remax - remin) / width;
	imagChange = (immax - immin) / height;

	for (int i = 0; i < thread_count; i++) {
		int topHeight = i * height / thread_count;
		int bottomHeight = (i + 1) * height / thread_count;
		switch (instructions) {
			case None:
				do_calculation_naive(status, remin, immax, realChange, imagChange, width, topHeight, bottomHeight, max_iters);
				break;
			case SSE:
				do_calculation_sse(status, remin, immax, realChange, imagChange, width, topHeight, bottomHeight, max_iters);
				break;
			case Wasm:
				do_calculation_wasm_simd(status, remin, immax, realChange, imagChange, width, topHeight, bottomHeight, max_iters);
				break;
		}
	}

	clock_t difference = clock() - before;
	int msec = difference * 1000 / CLOCKS_PER_SEC;

	printf("calculation time: %dms\n", msec);
}
