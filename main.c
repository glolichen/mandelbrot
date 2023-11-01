#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>

#include "complex.h"
#include "mandelbrot.h"
#include "bstree.h"

// ./mandelbrot [greatest dimension] [bottom left real] [bottom left imag] [top right real] [top right imag]
int main(int argc, char *argv[]) {
	BSTree bst;
	make_bstree(&bst);
	
	mpf_t num;
	mpf_init_set_d(num, 5828.12246);

	bstree_add(&bst, num, 1203);

	printf("%d\n", bstree_get(&bst, num));

	// Complex num;
	// complex_init_set_d(&num, -0.3900, -0.6630);
	// printf("%d\n", get_iterations(&num));
	// complex_clear(&num);

	// complex_init_set_d(&num, -0.33, 0.6);
	// printf("%d\n", get_iterations(&num));
	// complex_clear(&num);

	return 0;

	assert(argc == 6);

	int userProvided = atoi(argv[1]);

	double bottomLeftReal = atof(argv[2]), bottomLeftImag = atof(argv[3]);
	double topRightReal = atof(argv[4]), topRightImag = atof(argv[5]);

	Complex bottomLeft, topRight;
	complex_init_set_d(&bottomLeft, bottomLeftReal, bottomLeftImag);
	complex_init_set_d(&topRight, topRightReal, topRightImag);

	assert(mpf_cmp(bottomLeft.real, topRight.real) < 0);
	assert(mpf_cmp(bottomLeft.imag, topRight.imag) < 0);

	double wthRatio = (0.0 + topRightReal - bottomLeftReal) / (0.0 + topRightImag - bottomLeftImag);
	int width, height;

	if (wthRatio >= 1) {
		width = userProvided;
		height = userProvided / wthRatio;
	}
	else {
		height = userProvided;
		width = userProvided * wthRatio;
	}

	int res = write_png("mandelbrot.png", width, height, &bottomLeft, &topRight);

	complex_clear(&bottomLeft);
	complex_clear(&topRight);

	return res;
}