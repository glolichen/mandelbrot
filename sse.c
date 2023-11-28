#include <math.h>
#include <emmintrin.h>

#include "sse.h"
#include "base.h"
#include "linkedlist.h"

__m128d oneSSE, twoSSE, fourSSE, quarterSSE, sixteenthSSE;

__m128i get_iterations_sse(const __m128d *real, const __m128d *imag, int num_nums, int max_iters) {
	__m128i result = _mm_set1_epi16(0);
	__m128d x = *real, y = *imag;

	int skipCalculation = 0;

	// https://en.wikipedia.org/wiki/Plotting_algorithms_for_the_Mandelbrot_set#Cardioid_/_bulb_checking
	// but simd-ified
	__m128d p = _mm_add_pd(_mm_mul_pd(_mm_add_pd(x, oneSSE), _mm_add_pd(x, oneSSE)), _mm_mul_pd(y, y));
	__m128d q = _mm_add_pd(_mm_mul_pd(_mm_sub_pd(x, quarterSSE), _mm_sub_pd(x, quarterSSE)), _mm_mul_pd(y, y));
	__m128d r = _mm_mul_pd(q, _mm_add_pd(q, _mm_sub_pd(x, quarterSSE)));
	__m128d s = _mm_mul_pd(quarterSSE, _mm_mul_pd(y, y));
	int cond1 = _mm_movemask_pd(_mm_cmple_pd(p, sixteenthSSE));
	int cond2 = _mm_movemask_pd(_mm_cmple_pd(r, s));
	for (int i = 0; i < num_nums; i++) {
		if (((cond1 >> i) & 1) || ((cond2 >> i) & 1)) {
			result[i] = 0;
			skipCalculation |= 1 << i;
		}
	}
	for (int i = num_nums; i < 2; i++)
		skipCalculation |= 1 << i;
	
	// algorithm from https://en.wikipedia.org/wiki/Plotting_algorithms_for_the_Mandelbrot_set#Continuous_(smooth)_coloring
	__m128d x2, y2, tempReal, norm;

	int prevEscaped = 0, justEscaped = 0;
	int iters = 1;
	for (; iters < max_iters; iters++) {
		x2 = _mm_mul_pd(x, x);
		y2 = _mm_mul_pd(y, y);
		tempReal = _mm_add_pd(_mm_sub_pd(x2, y2), *real);
		y = _mm_add_pd(_mm_mul_pd(_mm_mul_pd(x, y), twoSSE), *imag);
		x = tempReal;

		norm = _mm_add_pd(x2, y2);
		justEscaped = _mm_movemask_pd(_mm_cmpge_pd(norm, fourSSE));
		for (int i = 0; i < num_nums; i++) {
			if ((justEscaped & (1 << i)) && !(prevEscaped & (1 << i)))
				result[i] = get_color(norm[i], iters, max_iters);
		}
		prevEscaped |= justEscaped;
		// 0b11 (all true) = 3
		if ((prevEscaped | skipCalculation) == 3)
			return result;
	}

	return result;
}
void *do_calculation_sse(void *arguments) {
	Arguments *args = (Arguments *) arguments;

	oneSSE = _mm_set1_pd(1);
	twoSSE = _mm_set1_pd(2);
	fourSSE = _mm_set1_pd(4);
	quarterSSE = _mm_set1_pd(0.25);
	sixteenthSSE = _mm_set1_pd(0.0625);
	
	__m128d remins = _mm_set1_pd(args->remin);
	__m128d immaxes = _mm_set1_pd(args->immax);
	__m128d realChanges = _mm_set1_pd(args->realChange);
	__m128d imagChanges = _mm_set1_pd(args->imagChange);
	
	__m128i iterses;

	LinkedList linkedList; // either a stack or deque
	make_linked_list(&linkedList);

	for (int i = 0; i < args->width; i++) {
		linked_list_add(&linkedList, args->top_height * (args->width - 1) + i);
		args->status[args->top_height * (args->width - 1) + i] = 1;
		linked_list_add(&linkedList, (args->bottom_height - 1) * args->width + i);
		args->status[(args->bottom_height - 1) * args->width + i] = 1;
	}
	for (int i = args->top_height + 1; i < args->bottom_height - 1; i++) {
		linked_list_add(&linkedList, i * args->width);
		args->status[i * args->width] = 1;
		linked_list_add(&linkedList, i * args->width + args->width - 1);
		args->status[i * args->width + args->width - 1] = 1;
	}

	int *cur, *cx, *cy, nx, ny, numPoppable, index;
	cur = (int *) malloc(2 * sizeof(int));
	cx = (int *) malloc(2 * sizeof(int));
	cy = (int *) malloc(2 * sizeof(int));

	__m128d reals, imags;

	while (linkedList.size != 0) {
		numPoppable = min(linkedList.size, 2);

		for (int i = 0; i < numPoppable; i++) {
			cur[i] = linked_list_pop_front(&linkedList);
			cx[i] = cur[i] % args->width, cy[i] = cur[i] / args->width;
		}
		for (int i = numPoppable; i < 2; i++)
			cur[i] = 0;

		reals = _mm_add_pd(remins, _mm_mul_pd(realChanges, _mm_setr_pd(cx[0], cx[1])));
		imags = _mm_sub_pd(immaxes, _mm_mul_pd(imagChanges, _mm_setr_pd(cy[0], cy[1])));

		iterses = get_iterations_sse(&reals, &imags, numPoppable, args->max_iters);
		for (int i = 0; i < numPoppable; i++) {
			args->status[cur[i]] = iterses[i];
			if (iterses[i] == 0)
				continue;

			for (int j = 0; j < 4; j++) {
				nx = cx[i] + DIRECTION[j][0];
				ny = cy[i] + DIRECTION[j][1];
				if (nx < 0 || ny < args->top_height || nx >= args->width || ny >= args->bottom_height)	
					continue;
				index = ny * args->width + nx;
				if (args->status[index] == 0) {
					args->status[index] = 16777216;
					linked_list_add(&linkedList, index);
				}
			}
		}
	}

	free(cur);
	free(cx);
	free(cy);

	free_linked_list(&linkedList);
	return NULL;
}
