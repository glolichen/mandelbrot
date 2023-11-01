#include <math.h>
#include <stdint.h>
#include <gmp.h>
#include <png.h>

#include "mandelbrot.h"

#define ERROR 1
#define MAX_ITERATION 1000

int get_iterations(const Complex *num) {
	int iters = 0;

	Complex current;
	complex_init(&current);
	complex_init_set_d(&current, 0, 0);

	mpf_t squareMagnitude;
	mpf_init(squareMagnitude);

	for (; iters < MAX_ITERATION; iters++) {
		complex_square(&current);
		complex_add(&current, num);
		complex_norm(squareMagnitude, &current);
		if (mpf_cmp_d(squareMagnitude, 4) >= 0)
			break;
	}
	
	complex_clear(&current);
	mpf_clear(squareMagnitude);

	return iters;
}

typedef struct {
	int red, green, blue;
} color_t;

void get_color(color_t *color, int iter_count) {
	// floor value of log2 = the location of the largest bit set to 1, counting from the right
	// the location of the largest bit from the left = __builtin_clz(int) - count leading zeros
	// length of an int is 32 - to find value from right do 32 - x + 1 = 31 - x
	int value = iter_count == 1000 ? 0 : 255 - 20 * (31 - __builtin_clz(iter_count));
	color->red = value, color->green = value, color->blue = value;
}

int write_png(const char *file_name, int width, int height, const Complex *bottom_left, const Complex *top_right) {
	mpf_t realChange;
	mpf_t imagChange;
	mpf_init(realChange);
	mpf_init(imagChange);
	mpf_sub(realChange, top_right->real, bottom_left->real);
	mpf_sub(imagChange, top_right->imag, bottom_left->imag);
	mpf_div_ui(realChange, realChange, width);
	mpf_div_ui(imagChange, imagChange, height);

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
	Complex point;
	complex_init(&point);

	color_t color;

	for (size_t y = 0; y < height; y++) {
		png_byte *row = png_malloc(png_ptr, sizeof(uint8_t) * width * pixel_size);
		row_pointers[y] = row;
		for (size_t x = 0; x < width; x++) {
			// bottom left real + realChange * y
			mpf_mul_ui(real, realChange, x);
			mpf_add(real, bottom_left->real, real);

			mpf_mul_ui(imag, imagChange, y);
			// top right imag - imagChange * y
			mpf_sub(imag, top_right->imag, imag);
			// the mandelbrot set is symmetric over the x axis
			mpf_abs(imag, imag);

			complex_set_mpf(&point, real, imag);

			int iterations = get_iterations(&point);
			get_color(&color, iterations);

			*(row++) = color.red;
			*(row++) = color.green;
			*(row++) = color.blue;
		}
	}
	
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
	if (0) {
		png_failure:
		png_create_info_struct_failed:
		png_destroy_write_struct(&png_ptr, &info_ptr);

		png_create_write_struct_failed:
		fclose(fp);

		fopen_failed:
		status = -1;
	}

	mpf_clear(realChange);
	mpf_clear(imagChange);
	return status;
}