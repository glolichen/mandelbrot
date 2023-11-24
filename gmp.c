#include <math.h>
#include <gmp.h>

#include "gmp.h"
#include "base.h"
#include "linkedlist.h"

int get_iterations_gmp(long double real, long double imag, int max_iters) {
	// https://en.wikipedia.org/wiki/Plotting_algorithms_for_the_Mandelbrot_set#Cardioid_/_bulb_checking

	if ((real + 1.0) * (real + 1.0) + imag * imag <= 0.0625)
		return 0;
	long double q = (real - 0.25) * (real - 0.25) + imag * imag;
	if (q * (q + (real - 0.25)) <= (0.25 * imag * imag))
		return 0;
	
	// algorithm from https://en.wikipedia.org/wiki/Plotting_algorithms_for_the_Mandelbrot_set#Continuous_(smooth)_coloring
	mpf_t x, y;
	mpf_init_set_d(x, real);
	mpf_init_set_d(y, imag);

	mpf_t x2, y2, tempReal, norm;
	mpf_init(x2);
	mpf_init(y2);
	mpf_init(tempReal);
	mpf_init(norm);

	mpf_t mpfReal, mpfImag;
	mpf_init_set_d(mpfReal, real);
	mpf_init_set_d(mpfImag, imag);

	int iters = 1;
	for (; iters < max_iters; iters++) {
		mpf_mul(x2, x, x);
		mpf_mul(y2, y, y);

		mpf_sub(tempReal, x2, y2);
		mpf_add(tempReal, tempReal, mpfReal);

		mpf_mul(y, x, y);
		mpf_mul_ui(y, y, 2);
		mpf_add(y, y,mpfImag);

		mpf_set(x, tempReal);

		mpf_add(norm, x2, y2);
		if (mpf_cmp_d(norm, 4) > 0)
			break;
	}

	long double ldNorm = mpf_get_d(norm);

	mpf_clear(x);
	mpf_clear(y);
	mpf_clear(x2);
	mpf_clear(y2);
	mpf_clear(tempReal);
	mpf_clear(norm);
	mpf_clear(mpfReal);
	mpf_clear(mpfImag);

	return get_color(ldNorm, iters, max_iters);
}
void do_calculation_gmp(int *status, double remin, double immax, double realChange, double imagChange, int width, int top_height, int bottom_height, int max_iters) {	
	int iters;
	long double real, imag;
	// mpf_t real, imag;

	// mpf_t mpfRemin, mpfImmax, mpfRealChange, mpfImagChange;
	// mpf_init_set_d(mpfRemin, remin);
	// mpf_init_set_d(mpfImmax, immax);
	// mpf_init_set_d(mpfRealChange, realChange);
	// mpf_init_set_d(mpfImagChange, imagChange);

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

	int cur, cx, cy, nx, ny, index;
	while (linkedList.size != 0) {
		cur = linked_list_pop_front(&linkedList);
		cx = cur % width, cy = cur / width;

		// mpf_mul_ui(real, mpfRealChange, cx);
		// mpf_add(real, mpfRemin, real);

		// mpf_mul_ui(imag, mpfImagChange, cy);
		// mpf_sub(imag, mpfImmax, imag);
		// mpf_abs(imag, imag);

		real = remin + realChange * cx;
		imag = fabs(immax - imagChange * cy);

		iters = get_iterations_gmp(real, imag, max_iters);
		status[cur] = iters;
		if (iters == 0)
			continue;

		for (int i = 0; i < 4; i++) {
			nx = cx + DIRECTION[i][0];
			ny = cy + DIRECTION[i][1];
			if (nx < 0 || ny < top_height || nx >= width || ny >= bottom_height)	
				continue;
			index = ny * width + nx;
			if (status[index] == 0) {
				status[index] = 16777216;
				linked_list_add(&linkedList, index);
			}
		}
	}

	free_linked_list(&linkedList);
}
