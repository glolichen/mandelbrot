#include <math.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <omp.h>
#include <png.h>

#include "mandelbrot.h"
#include "linkedlist.h"

#define PERIOD 9

const int Direction[4][2] = {
	{ 0, 1 },
	{ 1, 0 },
	{ 0, -1 },
	{ -1, 0 },
};

int get_iterations(double real, double imag, int max_iters) {
	// https://en.wikipedia.org/wiki/Plotting_algorithms_for_the_Mandelbrot_set#Cardioid_/_bulb_checking
	if ((real + 1.0) * (real + 1.0) + imag * imag <= 0.0625)
		return max_iters;
	double q = (real - 0.25) * (real - 0.25) + imag * imag;
	if (q * (q + (real - 0.25)) <= (0.25 * imag * imag))
		return max_iters;
	
	// algorithm from https://en.wikipedia.org/wiki/Plotting_algorithms_for_the_Mandelbrot_set#Continuous_(smooth)_coloring
	double x = real, y = imag;
	double x2, y2, tempReal;

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

	return iters;
}

typedef struct {
	int red, green, blue;
} Pixel;

void get_color(Pixel *color, int iter_count) {
	// floor value of log2 = the location of the largest bit set to 1, counting from the right
	// the location of the largest bit from the left = __builtin_clz(int) - count leading zeros
	// length of an int is 32 - to find value from right do 32 - x + 1 = 31 - x
	if (iter_count == 0) {
		color->red = 0, color->green = 0, color->blue = 0;
		return;
	}
	int value = iter_count < 1000 ? 255 - 20 * (31 - __builtin_clz(iter_count)) : 0;
	color->red = value, color->green = value, color->blue = value;
}

int write_png(const char *file_name, int width, int height, 
			  double bottom_left_real, double bottom_left_imag,
			  double top_right_real, double top_right_imag,	
			  int max_iters, int thread_count) {
	double realChange, imagChange, change;
	realChange = (top_right_real - bottom_left_real) / width;
	imagChange = (top_right_imag - bottom_left_imag) / height;

	change = (realChange + imagChange) / 2;
	
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

	int iters;
	double real, imag;
	Pixel color;

	int *status = (int *) calloc((height * width) * sizeof(int), sizeof(int));

	#pragma omp parallel for
	for (int i = 0; i < thread_count; i++) {
		int topHeight = i * height / thread_count;
		int bottomHeight = (i + 1) * height / thread_count;
		
		LinkedList linkedList; // either a stack or deque
		make_linked_list(&linkedList);

		for (int i = 0; i < width; i++) {
			linked_list_add(&linkedList, topHeight * (width - 1) + i);
			status[i] = 1;
			linked_list_add(&linkedList, bottomHeight * (width - 1) + i);
			status[height * (width - 1) + i] = 1;
		}
		for (int i = topHeight + 1; i < bottomHeight - 1; i++) {
			linked_list_add(&linkedList, i * width);
			status[i * width] = 1;
			linked_list_add(&linkedList, i * width + width - 1);
			status[i * width + width - 1] = 1;
		}

		int cur, cx, cy, nx, ny, index;
		while (linkedList.size != 0) {
			cur = linked_list_pop_front(&linkedList);
			cx = cur % width, cy = cur / width;

			real = bottom_left_real + change * cx;
			imag = fabs(top_right_imag - change * cy);
			iters = get_iterations(real, imag, max_iters);
			status[cur] = iters << 1;
			if (iters == max_iters)
				continue;

			for (int i = 0; i < 4; i++) {
				nx = cx + Direction[i][0];
				ny = cy + Direction[i][1];
				if (nx < 0 || ny < topHeight || nx >= width || ny >= bottomHeight)	
					continue;
				index = ny * width + nx;
				if (status[index] == 0) {
					status[index] = 1;
					linked_list_add(&linkedList, index);
				}
			}
		}

		free_linked_list(&linkedList);
	}

	for (int y = 0; y < height; y++) {
		png_byte *row = png_malloc(png_ptr, sizeof(uint8_t) * width * pixel_size);
		row_pointers[y] = row;
		for (int x = 0; x < width; x++) {
			iters = status[y * width + x] >> 1;
			get_color(&color, iters);
			*(row + 3 * x) = color.red;
			*(row + 3 * x + 1) = color.green;
			*(row + 3 * x + 2) = color.blue;
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
	png_failure:
	png_create_info_struct_failed:
	png_destroy_write_struct(&png_ptr, &info_ptr);

	png_create_write_struct_failed:
	fclose(fp);

	fopen_failed:
	exitStatus = -1;

	return exitStatus;
}
