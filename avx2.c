#include <math.h>
#include <immintrin.h>

#include "avx2.h"
#include "base.h"
#include "linkedlist.h"

__m256d oneAVX2, twoAVX2, fourAVX2, quarterAVX2, sixteenthAVX2;

__m256i get_iterations_avx2(const __m256d *real, const __m256d *imag, int num_nums, int max_iters) {
	__m256i result = _mm256_set1_epi16(0);
	__m256d x = *real, y = *imag;

	__mmask8 skipCalculation = 0;

	// https://en.wikipedia.org/wiki/Plotting_algorithms_for_the_Mandelbrot_set#Cardioid_/_bulb_checking
	// but simd-ified
	__m256d p = _mm256_add_pd(_mm256_mul_pd(_mm256_add_pd(x, oneAVX2), _mm256_add_pd(x, oneAVX2)), _mm256_mul_pd(y, y));
	__m256d q = _mm256_add_pd(_mm256_mul_pd(_mm256_sub_pd(x, quarterAVX2), _mm256_sub_pd(x, quarterAVX2)), _mm256_mul_pd(y, y));
	__m256d r = _mm256_mul_pd(q, _mm256_add_pd(q, _mm256_sub_pd(x, quarterAVX2)));
	__m256d s = _mm256_mul_pd(quarterAVX2, _mm256_mul_pd(y, y));
	int cond1 = _mm256_cmp_pd_mask(p, sixteenthAVX2, _CMP_LE_OS);
	int cond2 = _mm256_cmp_pd_mask(r, s, _CMP_LE_OS);
	for (int i = 0; i < num_nums; i++) {
		if (((cond1 >> i) & 1) || ((cond2 >> i) & 1)) {
			result[i] = 0;
			skipCalculation |= 1 << i;
		}
	}
	for (int i = num_nums; i < 4; i++)
		skipCalculation |= 1 << i;
	
	// algorithm from https://en.wikipedia.org/wiki/Plotting_algorithms_for_the_Mandelbrot_set#Continuous_(smooth)_coloring
	__m256d x2, y2, tempReal, norm;

	__mmask8 prevEscaped = 0, justEscaped = 0;
	int iters = 1;
	for (; iters < max_iters; iters++) {
		x2 = _mm256_mul_pd(x, x);
		y2 = _mm256_mul_pd(y, y);
		tempReal = _mm256_add_pd(_mm256_sub_pd(x2, y2), *real);
		y = _mm256_add_pd(_mm256_mul_pd(_mm256_mul_pd(x, y), twoAVX2), *imag);
		x = tempReal;

		norm = _mm256_add_pd(x2, y2);
		justEscaped = _mm256_cmp_pd_mask(_mm256_add_pd(x2, y2), fourAVX2, _CMP_GE_OS);
		for (int i = 0; i < num_nums; i++) {
			if ((justEscaped & (1 << i)) && !(prevEscaped & (1 << i)))
				result[i] = get_color(norm[i], iters, max_iters);
		}
		prevEscaped |= justEscaped;
		// 0b1111 (all true) = 15
		if ((prevEscaped | skipCalculation) == 15)
			return result;
	}

	return result;
}
void do_calculation_avx2(int *status, double remin, double immax, double realChange, double imagChange, int width, int top_height, int bottom_height, int max_iters) {
	oneAVX2 = _mm256_set1_pd(1);
	twoAVX2 = _mm256_set1_pd(2);
	fourAVX2 = _mm256_set1_pd(4);
	quarterAVX2 = _mm256_set1_pd(0.25);
	sixteenthAVX2 = _mm256_set1_pd(0.0625);
	
	__m256d remins = _mm256_set1_pd(remin);
	__m256d immaxes = _mm256_set1_pd(immax);
	__m256d realChanges = _mm256_set1_pd(realChange);
	__m256d imagChanges = _mm256_set1_pd(imagChange);
	
	__m256i iterses;

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
	cur = (int *) malloc(4 * sizeof(int));
	cx = (int *) malloc(4 * sizeof(int));
	cy = (int *) malloc(4 * sizeof(int));

	__m256d reals, imags;

	while (linkedList.size != 0) {
		numPoppable = min(linkedList.size, 4);

		for (int i = 0; i < numPoppable; i++) {
			cur[i] = linked_list_pop_front(&linkedList);
			cx[i] = cur[i] % width, cy[i] = cur[i] / width;
		}
		for (int i = numPoppable; i < 4; i++)
			cur[i] = 0;

		reals = _mm256_add_pd(remins, _mm256_mul_pd(realChanges, _mm256_setr_pd(cx[0], cx[1], cx[2], cx[3])));
		imags = _mm256_sub_pd(immaxes, _mm256_mul_pd(imagChanges, _mm256_setr_pd(cy[0], cy[1], cy[2], cy[3])));

		iterses = get_iterations_avx2(&reals, &imags, numPoppable, max_iters);
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
	}

	free(cur);
	free(cx);
	free(cy);

	free_linked_list(&linkedList);
}