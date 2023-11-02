#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>

#include "complex.h"
#include "mandelbrot.h"
#include "bstree.h"

// ./mandelbrot [greatest dimension] [bottom left real] [bottom left imag] [top right real] [top right imag]
int main(int argc, char *argv[]) {
	// mpf_t range;
	// mpf_init_set_d(range, 0.000000000001);

	// GenericBSTree bst;
	// make_bstree(&bst, range);
	
	// mpf_t key;
	// mpf_init(key);
	// GenericBSTreeValue value;
	// value.t = INT;

	// mpf_set_d(key, 5828.12246);
	// value.data.integer = 1203;
	// bstree_add(&bst, key, &value);
	
	// mpf_set_d(key, 28.3939393939);
	// value.data.integer = 1567;
	// bstree_add(&bst, key, &value);
	
	// mpf_set_d(key, 6000.00000);
	// value.data.integer = 2;
	// bstree_add(&bst, key, &value);
	
	// mpf_set_d(key, 28.3939393939);
	// value.data.integer = 2;
	// bstree_add(&bst, key, &value);
	
	// mpf_set_d(key, 28.3939393939);
	// printf("%d\n", bstree_get(&bst, key).data.integer);
	
	// mpf_set_d(key, 6000.00000);
	// printf("%d\n", bstree_get(&bst, key).data.integer);
	
	// mpf_set_d(key, 5828.12246);
	// printf("%d\n", bstree_get(&bst, key).data.integer);

	// clear_bstree(&bst);

	// return 0;

	assert(argc == 6);

	int userProvided = atoi(argv[1]);

	mpf_t bottomLeftReal, bottomLeftImag, topRightReal, topRightImag;
	mpf_init_set_str(bottomLeftReal, argv[2], 10);
	mpf_init_set_str(bottomLeftImag, argv[3], 10);
	mpf_init_set_str(topRightReal, argv[4], 10);
	mpf_init_set_str(topRightImag, argv[5], 10);

	assert(mpf_cmp(bottomLeftReal, topRightReal) < 0);
	assert(mpf_cmp(bottomLeftImag, topRightImag) < 0);

	double wthRatio = (0.0 + atof(argv[4]) - atof(argv[2])) / (0.0 + atof(argv[5]) - atof(argv[3]));
	int width, height;

	if (wthRatio >= 1) {
		width = userProvided;
		height = userProvided / wthRatio;
	}
	else {
		height = userProvided;
		width = userProvided * wthRatio;
	}

	int res = write_png("mandelbrot.png", width, height, bottomLeftReal, bottomLeftImag, topRightReal, topRightImag);

	mpf_clear(bottomLeftReal);
	mpf_clear(bottomLeftImag);
	mpf_clear(topRightReal);
	mpf_clear(topRightImag);

	return res;
}