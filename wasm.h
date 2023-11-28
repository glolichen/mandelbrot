#ifndef WASM_H
#define WASM_H

#include <stdio.h>

void do_calculation_wasm(int *status, int width, int height, 
			  double remin, double immin, double remax, double immax,	
			  int max_iters, int thread_count, int instructions);
void do_calculation_wasm_no_thread(int *status, int width, int height, 
			  double remin, double immin, double remax, double immax,	
			  int max_iters, int _, int instructions);

#endif
