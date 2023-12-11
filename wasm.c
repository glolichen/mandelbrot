#include <math.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <wasm_simd128.h>
#include <pthread.h>
#include <emscripten.h>

#include "wasm.h"
#include "base.h"
#include "nosimd.h"
#include "sse.h"
#include "linkedlist.h"

void do_calculation_wasm(int *status, int width, int height, 
			  double remin, double immin, double remax, double immax,	
			  int max_iters, int thread_count) {

	clock_t before = clock();

	double realChange, imagChange;
	realChange = (remax - remin) / width;
	imagChange = (immax - immin) / height;

	int returnCode;

	// thanks wikipedia https://en.wikipedia.org/wiki/Pthreads#Example

	pthread_t *threads = (pthread_t *) malloc(thread_count * sizeof(pthread_t));
	Arguments *threadArgs = (Arguments *) malloc(thread_count * sizeof(Arguments));

	for (int i = 0; i < thread_count; i++) {
		int topHeight = i * height / thread_count;
		int bottomHeight = (i + 1) * height / thread_count;
		Arguments args = {
			.status = status, .remin = remin, .immax = immax, .realChange = realChange, .imagChange = imagChange,
			.width = width, .top_height = topHeight, .bottom_height = bottomHeight, .max_iters = max_iters
		};
		threadArgs[i] = args;

		returnCode = pthread_create(&threads[i], NULL, do_calculation_naive, &threadArgs[i]);
   		assert(!returnCode);
	}

	printf("made all threads\n");

	for (int i = 0; i < thread_count; i++) {
		printf("joining thread %d\n", i);
		returnCode = pthread_join(threads[i], NULL);
		assert(!returnCode);
	}

	free(threads);
	free(threadArgs);

	clock_t difference = clock() - before;
	int msec = difference * 1000 / CLOCKS_PER_SEC;

	printf("calculation time: %dms\n", msec);
}

void do_calculation_wasm_no_thread(int *status, int width, int height, 
			  double remin, double immin, double remax, double immax,	
			  int max_iters, int _) {
		
	clock_t before = clock();

	double realChange, imagChange;
	realChange = (remax - remin) / width;
	imagChange = (immax - immin) / height;

	Arguments args = {
		.status = status, .remin = remin, .immax = immax, .realChange = realChange, .imagChange = imagChange,
		.width = width, .top_height = 0, .bottom_height = height, .max_iters = max_iters
	};
	do_calculation_naive(&args);

	clock_t difference = clock() - before;
	int msec = difference * 1000 / CLOCKS_PER_SEC;

	printf("calculation time: %dms\n", msec);		
}

void do_calculation_wasm_no_thread_with_past(int *status, int width, int height, 
			  double remin, double immin, double remax, double immax,	
			  int max_iters, int _, int *pastColors,
			  double pastremin, double pastimmin, double pastremax, double pastimmax) {
		
	clock_t before = clock();

	double realChange, imagChange;
	realChange = (remax - remin) / width;
	imagChange = (immax - immin) / height;

	ExtraArguments args = {
		.status = status, .remin = remin, .immax = immax, .realChange = realChange, .imagChange = imagChange,
		.pastremin = pastremin, .pastimmin = pastimmin, .pastremax = pastremax, .pastimmax = pastimmax,
		.pastColors = pastColors, .width = width, .top_height = 0, .bottom_height = height, .max_iters = max_iters
	};

	do_calculation_naive_with_past((void *) &args);

	clock_t difference = clock() - before;
	int msec = difference * 1000 / CLOCKS_PER_SEC;

	printf("calculation time: %dms\n", msec);		
}
