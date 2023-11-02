#include <math.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <gmp.h>
#include <png.h>

#include "mandelbrot.h"
#include "bstree.h"

#define ERROR 1
#define MAX_ITERATION 1000

GenericBSTree past;

int get_iterations(const mpf_t real, const mpf_t imag) {
	// algorithm from https://en.wikipedia.org/wiki/Plotting_algorithms_for_the_Mandelbrot_set#Continuous_(smooth)_coloring
	
	mpf_t x, y, x2, y2;
	mpf_init_set_d(x, 0);
	mpf_init_set_d(y, 0);
	mpf_init(x2);
	mpf_init(y2);

	int iters = 0;

	mpf_t norm, temp;
	mpf_init(norm);
	mpf_init(temp);

	for (; iters < MAX_ITERATION; iters++) {
		GenericBSTreeValue pastValue = bstree_get(&past, x);
		if (pastValue.t == IMAGNODE) {
			printf("found match for real ");
			ImagBSTreeNode *imagTree = pastValue.data.imagNode;
			printf("%p\n", imagTree);
		}

		mpf_mul(x2, x, x);
		mpf_mul(y2, y, y);

		mpf_sub(temp, x2, y2);
		mpf_add(temp, temp, real);

		mpf_mul(y, x, y);
		mpf_mul_ui(y, y, 2);
		mpf_add(y, y, imag);

		mpf_set(x, temp);

		mpf_add(norm, x2, y2);
		if (mpf_cmp_d(norm, 4.0) >= 0)
			break;
	}

	mpf_clear(x);
	mpf_clear(y);

	mpf_clear(x2);
	mpf_clear(y2);

	mpf_clear(norm);
	mpf_clear(temp);

	return iters;
}

typedef struct {
	int red, green, blue;
} color_t;

void get_color(color_t *color, int iter_count) {
	// floor value of log2 = the location of the largest bit set to 1, counting from the right
	// the location of the largest bit from the left = __builtin_clz(int) - count leading zeros
	// length of an int is 32 - to find value from right do 32 - x + 1 = 31 - x
	int value = iter_count < 1000 ? 255 - 20 * (31 - __builtin_clz(iter_count)) : 0;
	color->red = value, color->green = value, color->blue = value;
}

int write_png(const char *file_name, int width, int height, 
			  const mpf_t bottom_left_real, const mpf_t bottom_left_imag,
			  const mpf_t top_right_real, const mpf_t top_right_imag) {
	mpf_t realChange, imagChange, change;
	mpf_init(realChange);
	mpf_init(imagChange);
	mpf_sub(realChange, top_right_real, bottom_left_real);
	mpf_sub(imagChange, top_right_imag, bottom_left_imag);
	mpf_div_ui(realChange, realChange, width);
	mpf_div_ui(imagChange, imagChange, height);

	mpf_init(change);
	mpf_add(change, realChange, imagChange);
	mpf_div_ui(change, change, 2);

	mpf_clear(realChange);
	mpf_clear(imagChange);

	FILE *fp;

	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	png_byte **row_pointers = NULL;
	int pixel_size = 3;
	int depth = 8;
	
	fp = fopen(file_name, "wb");
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
	
	row_pointers = png_malloc(png_ptr, height * sizeof(png_byte *));
	mpf_t real, imag;
	mpf_init(real);
	mpf_init(imag);

	color_t color;

	make_bstree(&past, change);

	for (size_t y = 0; y < height; y++) {
		png_byte *row = png_malloc(png_ptr, sizeof(uint8_t) * width * pixel_size);
		row_pointers[y] = row;
		for (size_t x = 0; x < width; x++) {
			// bottom left real + change * y
			mpf_mul_ui(real, change, x);
			mpf_add(real, bottom_left_real, real);

			mpf_mul_ui(imag, change, y);
			// top right imag - change * y
			mpf_sub(imag, top_right_imag, imag);
			// the mandelbrot set is symmetric over the x axis
			mpf_abs(imag, imag);

			int iterations = get_iterations(real, imag);

			if (iterations == 1000) {
				GenericBSTreeValue realValue = { .t = IMAGNODE, .data.imagNode = NULL };
				GenericBSTreeNode *realNode = bstree_add(&past, real, &realValue);
				GenericBSTreeValue imagValue = { .t = INT, .data.integer = iterations };
				GenericBSTreeNode imagNode = { .t = IMAG, .data.imagNode = realNode->data.realNode->value };
				bstree_node_add(&imagNode, imag, &imagValue);
			}
			// printf("%p\n", realNode->data.realNode->value);

			// GenericBSTreeNode = { .t = IMAG, .data.imagNode =  }
			// GenericBSTreeValue = { .t = IMAGNODE, .data.imagNode =  }	

			get_color(&color, iterations);

			*(row++) = color.red;
			*(row++) = color.green;
			*(row++) = color.blue;
		}
	}

	mpf_t test;
	mpf_init_set_d(test, 0.356584119496855345912);
	GenericBSTreeValue pastValue = bstree_get(&past, test);
	if (pastValue.t != NOTHING) {
		printf("found match\n");
	}
	mpf_clear(test);

	clear_bstree(&past);
	
	/* Write the image data to "fp". */

	png_init_io(png_ptr, fp);
	png_set_rows(png_ptr, info_ptr, row_pointers);
	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

	/* The routine has successfully written the file, so we set
	"status" to a value which indicates success. */
	
	for (size_t y = 0; y < height; y++)
		png_free(png_ptr, row_pointers[y]);
	png_free(png_ptr, row_pointers);

	int status = 0;
	png_failure:
	png_create_info_struct_failed:
	png_destroy_write_struct(&png_ptr, &info_ptr);

	png_create_write_struct_failed:
	fclose(fp);

	fopen_failed:
	status = -1;

	mpf_clear(change);
	return status;
}