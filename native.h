#ifndef NATIVE_H
#define NATIVE_H

#include <stdio.h>
#include "base.h"

enum { None, SSE, AVX2, GMP };

int write_png(const char *file_name, int width, int height, 
			  double remin, double immin, double remax, double immax,	
			  int max_iters, int thread_count, int instructions);

void do_calculation_native(int *status, int width, int height, 
			  double remin, double immin, double remax, double immax,	
			  int max_iters, int thread_count, int instructions);

#endif
