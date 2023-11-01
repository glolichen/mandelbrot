#include <gmp.h>
#include "complex.h"

void complex_init(Complex *num) {
	mpf_init(num->real);
	mpf_init(num->imag);
}

void complex_set_d(Complex *num, double real, double imag) {
	mpf_set_d(num->real, real);
	mpf_set_d(num->imag, imag);
}
void complex_init_set_d(Complex *num, double real, double imag) {
	complex_init(num);
	complex_set_d(num, real, imag);
}

void complex_set_mpf(Complex *num, const mpf_t real, const mpf_t imag) {
	mpf_set(num->real, real);
	mpf_set(num->imag, imag);
}
void complex_init_set_mpf(Complex *num, const mpf_t real, const mpf_t imag) {
	complex_init(num);
	complex_set_mpf(num, real, imag);
}

void complex_set(Complex *num, const Complex *src) {
	mpf_set(num->real, num->real);
	mpf_set(num->imag, num->imag);
}
void complex_init_set(Complex *num, const Complex *src) {
	complex_init(num);
	complex_set(num, src);
}

void complex_square(Complex *num) {
	// (a+bi)^2 = (a^2 - b^2) + 2abi

	mpf_t realSquare; // a^2
	mpf_init(realSquare);
	mpf_set(realSquare, num->real);
	mpf_mul(realSquare, realSquare, realSquare);

	mpf_t imagSquare; // b^2
	mpf_init(imagSquare);
	mpf_set(imagSquare, num->imag);
	mpf_mul(imagSquare, imagSquare, imagSquare);
	
	mpf_t twoRealImag; // ab
	mpf_init(twoRealImag);
	mpf_mul(twoRealImag, num->real, num->imag);
	mpf_mul_ui(twoRealImag, twoRealImag, 2);

	// real = a^2 - b^2
	mpf_sub(num->real, realSquare, imagSquare);

	// imag = 2ab
	mpf_set(num->imag, twoRealImag);

	mpf_clear(realSquare);
	mpf_clear(imagSquare);
	mpf_clear(twoRealImag);
}
void complex_add(Complex *num, const Complex *addend) {
	mpf_add(num->real, num->real, (*addend).real);
	mpf_add(num->imag, num->imag, (*addend).imag);
}
void complex_norm(mpf_t out, const Complex *num) {
	// magnitude = sqrt(a^2 + b^2)
	// square magnitude = a^2 + b^2

	mpf_t realSquare; // a^2
	mpf_init(realSquare);
	mpf_set(realSquare, num->real);
	mpf_mul(realSquare, realSquare, realSquare);

	mpf_t imagSquare; // b^2
	mpf_init(imagSquare);
	mpf_set(imagSquare, num->imag);
	mpf_mul(imagSquare, imagSquare, imagSquare);

	// a^2 + b^2
	mpf_add(out, realSquare, imagSquare);

	mpf_clear(realSquare);
	mpf_clear(imagSquare);
}

void complex_clear(Complex *num) {
	mpf_clear(num->real);
	mpf_clear(num->imag);
}

void complex_print(const Complex *num) {
	gmp_printf("(%.Ff, %.Ff)\n", num->real, num->imag);
}