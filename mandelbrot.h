#ifndef MANDELBROT_H
#define MANDELBROT_H

#include <stdio.h>
#include "complex.h"

int write_png(const char *file_name, int width, int height, 
			  double bottom_left_real, double bottom_left_imag,
			  double top_right_real, double top_right_imag,	
			  int max_iters, int thread_count);

#endif
