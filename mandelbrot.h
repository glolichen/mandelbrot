#ifndef MANDELBROT_H
#define MANDELBROT_H

#include <stdio.h>
#include "complex.h"

int get_iterations(const mpf_t real, const mpf_t imag);
int write_png(const char *file_name, int width, int height, 
			  const mpf_t bottom_left_real, const mpf_t bottom_left_imag,
			  const mpf_t top_right_real, const mpf_t top_right_imag);

#endif