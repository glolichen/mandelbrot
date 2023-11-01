#ifndef MANDELBROT_H
#define MANDELBROT_H

#include <stdio.h>
#include "complex.h"

int get_iterations(const Complex *num);
int write_png(const char *file_name, int width, int height, const Complex *bottom_left, const Complex *top_right);

#endif