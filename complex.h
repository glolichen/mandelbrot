#ifndef COMPLEX_H
#define COMPLEX_H

#include <gmp.h>

typedef struct {
	mpf_t real, imag;
} Complex;

void complex_init(Complex *num);

void complex_set_d(Complex *num, double real, double imag);
void complex_init_set_d(Complex *num, double real, double imag);

void complex_set_mpf(Complex *num, const mpf_t real, const mpf_t imag);
void complex_init_set_mpf(Complex *num, const mpf_t real, const mpf_t imag);

void complex_set(Complex *num, const Complex *src);
void complex_init_set(Complex *num, const Complex *src);

void complex_square(Complex *num);
void complex_add(Complex *num, const Complex *addend);
void complex_norm(mpf_t out, const Complex *num);

void complex_clear(Complex *num);

void complex_print(const Complex *num);

#endif