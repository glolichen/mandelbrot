#include <math.h>

#include "nosimd.h"
#include "base.h"
#include "linkedlist.h"

int get_iterations_naive(double real, double imag, int max_iters) {
	// https://en.wikipedia.org/wiki/Plotting_algorithms_for_the_Mandelbrot_set#Cardioid_/_bulb_checking
	if ((real + 1.0) * (real + 1.0) + imag * imag <= 0.0625)
		return 0;
	double q = (real - 0.25) * (real - 0.25) + imag * imag;
	if (q * (q + (real - 0.25)) <= (0.25 * imag * imag))
		return 0;
	
	// algorithm from https://en.wikipedia.org/wiki/Plotting_algorithms_for_the_Mandelbrot_set#Continuous_(smooth)_coloring
	double x = real, y = imag;
	double x2 = 0, y2 = 0, tempReal;

	int iters = 1;
	for (; iters < max_iters; iters++) {
		x2 = x * x;
		y2 = y * y;
		tempReal = x2 - y2 + real;
		y = x * y * 2 + imag;
		x = tempReal;
		if (x2 + y2 >= 4)
			break;
	}

	return get_color(x2 + y2, iters, max_iters);
}
void do_calculation_naive(int *status, double remin, double immax, double realChange, double imagChange, int width, int top_height, int bottom_height, int max_iters) {
	printf("%d %d %d\n", width, top_height, bottom_height);
	
	int iters;
	double real, imag;

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

		real = remin + realChange * cx;
		// imag = immax - change * cy;
		imag = fabs(immax - imagChange * cy);
		iters = get_iterations_naive(real, imag, max_iters);
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
