#include <math.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <png.h>
#include <omp.h>

#include "native.h"
#include "base.h"
#include "nosimd.h"
#include "sse.h"
#include "avx2.h"
#include "linkedlist.h"

void do_calculation_native(int *status, int width, int height, 
			  double remin, double immin, double remax, double immax,	
			  int max_iters, int thread_count, int instructions) {

	double realChange, imagChange;
	realChange = (remax - remin) / width;
	imagChange = (immax - immin) / height;

	#pragma omp parallel for
	for (int i = 0; i < thread_count; i++) {
		int topHeight = i * height / thread_count;
		int bottomHeight = (i + 1) * height / thread_count;
		switch (instructions) {
			case None:
				do_calculation_naive(status, remin, immax, realChange, imagChange, width, topHeight, bottomHeight, max_iters);
				break;
			case SSE:
				do_calculation_sse(status, remin, immax, realChange, imagChange, width, topHeight, bottomHeight, max_iters);
				break;
			case AVX2:
				do_calculation_avx2(status, remin, immax, realChange, imagChange, width, topHeight, bottomHeight, max_iters);
				break;
		}
	}
}

int write_png(const char *file_name, int width, int height, 
			  double remin, double immin, double remax, double immax,	
			  int max_iters, int thread_count, int instructions) {	

	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	png_byte **row_pointers = NULL;
	int pixel_size = 3;
	int depth = 8;
	
	FILE *fp = fopen(file_name, "wb");
	if (!fp)
		goto fopen_failed;

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL)
		goto png_create_write_struct_failed;
	
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
		goto png_create_info_struct_failed;
	
	if (setjmp(png_jmpbuf(png_ptr)))
		goto png_failure;
	
	png_set_IHDR(png_ptr, info_ptr, width, height, depth, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	
	row_pointers = (png_byte **) png_malloc(png_ptr, height * sizeof(png_byte *));
	
	int *status = (int *) calloc((height * width) * sizeof(int), sizeof(int)), color;
	do_calculation_native(status, width, height, remin, immin, remax, immax, max_iters, thread_count, instructions);

	for (int y = 0; y < height; y++) {
		png_byte *row = png_malloc(png_ptr, sizeof(uint8_t) * width * pixel_size);
		row_pointers[y] = row;
		for (int x = 0; x < width; x++) {
			color = status[y * width + x];
			*(row + 3 * x) = GET_RED(color);
			*(row + 3 * x + 1) = GET_GREEN(color);
			*(row + 3 * x + 2) = GET_BLUE(color);
		}
	}

	free(status);
	
	png_init_io(png_ptr, fp);
	png_set_rows(png_ptr, info_ptr, row_pointers);
	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

	for (int y = 0; y < height; y++)
		png_free(png_ptr, row_pointers[y]);
	png_free(png_ptr, row_pointers);

	int exitStatus = 0;
	
	if (0) {
		png_failure:
		png_create_info_struct_failed:
		png_destroy_write_struct(&png_ptr, &info_ptr);

		png_create_write_struct_failed:
		fclose(fp);

		fopen_failed:
		exitStatus = -1;
	}

	return exitStatus;
}