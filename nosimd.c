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
void *do_calculation_naive(void *arguments) {	
	Arguments *args = (Arguments *) arguments;

	int iters;
	double real, imag;

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

	int cur, cx, cy, nx, ny, index;
	while (linkedList.size != 0) {
		cur = linked_list_pop_front(&linkedList);
		cx = cur % args->width, cy = cur / args->width;

		real = args->remin + args->realChange * cx;
		// imag = immax - change * cy;
		imag = fabs(args->immax - args->imagChange * cy);
		iters = get_iterations_naive(real, imag, args->max_iters);
		args->status[cur] = iters;
		if (iters == 0)
			continue;

		for (int i = 0; i < 4; i++) {
			nx = cx + DIRECTION[i][0];
			ny = cy + DIRECTION[i][1];
			if (nx < 0 || ny < args->top_height || nx >= args->width || ny >= args->bottom_height)	
				continue;
			index = ny * args->width + nx;
			if (args->status[index] == 0) {
				args->status[index] = 16777216;
				linked_list_add(&linkedList, index);
			}
		}
	}

	free_linked_list(&linkedList);
	return NULL;
}

void *do_calculation_naive_with_past(void *arguments) {	
	ExtraArguments *args = (ExtraArguments *) arguments;

	int iters;
	double real, imag;

	double pastRealChange, pastImagChange;
	pastRealChange = (args->pastremax - args->pastremin) / args->width;
	pastImagChange = (args->pastimmax - args->pastimmin) / (args->bottom_height - args->top_height);

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

	int cur, cx, cy, nx, ny, index;
	double pastcx, pastcy, doubleIndex;
	while (linkedList.size != 0) {
		cur = linked_list_pop_front(&linkedList);
		cx = cur % args->width, cy = cur / args->width;

		real = args->remin + args->realChange * cx;
		imag = args->immax - args->imagChange * cy;

		if (real > args->pastremin && real < args->pastremax &&
			imag > args->pastimmin && imag < args->pastimmax) {

			pastcx = (real - args->pastremin) / pastRealChange;
			pastcy = (args->pastimmax - imag) / pastImagChange;

			doubleIndex = pastcy * args->width + pastcx;
			if (doubleIndex > 0 && doubleIndex < args->width * (args->bottom_height - args->top_height))
				args->status[cur] = args->pastColors[(int) doubleIndex];
		}
		else {
			imag = fabs(imag);
			iters = get_iterations_naive(real, imag, args->max_iters);
			args->status[cur] = iters;
		}

		if (args->status[cur] == 0)
			continue;

		for (int i = 0; i < 4; i++) {
			nx = cx + DIRECTION[i][0];
			ny = cy + DIRECTION[i][1];
			if (nx < 0 || ny < args->top_height || nx >= args->width || ny >= args->bottom_height)	
				continue;
			index = ny * args->width + nx;
			if (args->status[index] == 0) {
				args->status[index] = 16777216;
				linked_list_add(&linkedList, index);
			}
		}
	}

	free_linked_list(&linkedList);
	return NULL;
}
