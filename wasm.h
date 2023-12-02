#ifndef WASM_H
#define WASM_H

#include <stdio.h>

void do_calculation_wasm(int *status, int width, int height, 
			  double remin, double immin, double remax, double immax,	
			  int max_iters, int thread_count);
void do_calculation_wasm_no_thread(int *status, int width, int height, 
			  double remin, double immin, double remax, double immax,	
			  int max_iters, int _);
void do_calculation_wasm_no_thread_with_past(int *status, int width, int height, 
			  double remin, double immin, double remax, double immax,	
			  int max_iters, int _, int *pastColors,
			  double pastremin, double pastimmin, double pastremax, double pastimmax);

#endif
