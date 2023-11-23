#include <assert.h>
#include <math.h>
#include <stdio.h>

#include "base.h"

int DIRECTION[4][2] = {
	{ 0, 1 },
	{ 1, 0 },
	{ 0, -1 },
	{ -1, 0 },
};

// a % b is not a modulo operator. It is a remainder operator
// so -2 % 5 is -2, not 3
// this is bad for the hsv calculation formula, where negatives must wrap so that it is between 0 and 360
int mod(int a, int b) {
	int r = a % b;
    return r < 0 ? r + b : r;
}

// https://stackoverflow.com/a/6930407
int hsv2rgb(double h, double s, double v) {
	assert(h >= 0 && h <= 360);
	assert(s >= 0.0 && s <= 1.0);
	assert(v >= 0.0 && v <= 1.0);

	double hh, p, q, t, ff;
	long i;
	int out = 0;

	assert(s > 0.0);

	hh = h;
	if (hh >= 360.0)
		hh = 0.0;
	hh /= 60.0;
	i = (long) hh;
	ff = hh - i;
	p = v * (1.0 - s);
	q = v * (1.0 - (s * ff));
	t = v * (1.0 - (s * (1.0 - ff)));

	switch (i) {
		case 0:
			SET_RGB(out, (int) (255 * v), (int) (255 * t), (int) (255 * p));
			break;
		case 1:
			SET_RGB(out, (int) (255 * q), (int) (255 * v), (int) (255 * p));
			break;
		case 2:
			SET_RGB(out, (int) (255 * p), (int) (255 * v), (int) (255 * t));
			break;
		case 3:
			SET_RGB(out, (int) (255 * p), (int) (255 * q), (int) (255 * v));
			break;
		case 4:
			SET_RGB(out, (int) (255 * t), (int) (255 * p), (int) (255 * v));
			break;
		case 5:
		default:
			SET_RGB(out, (int) (255 * v), (int) (255 * p), (int) (255 * q));
			break;
	}

	return out;     
}

int get_color(double norm, int iters, int max_iters) {
	// floor value of log2 = the location of the largest bit set to 1, counting from the right
	// the location of the largest bit from the left = __builtin_clz(int) - count leading zeros
	// length of an int is 32 - to find value from right do 32 - x + 1 = 31 - x
	int ret = 0;
	if (iters == 0 || iters == max_iters)
		SET_RGB(ret, 0, 0, 0);
	else {
		double nsmooth = iters + 1 - log(log(norm) / 2) / log(2);
		// ret = hsv2rgb(((int) (240 - nsmooth - pow(1.02, nsmooth))) % 360, 0.6, 0.8);
		ret = hsv2rgb(mod((int) 240 - nsmooth, 360), 0.6, 0.8);

		// double hue = ((double) iters) + 1.0 - log(log(sqrt(norm))) / log(2.0);  // 2 is escape radius
		// printf("%f\n", hue);
		// // hue = 0.4 + 10.0 * hue; // adjust to make it prettier
		// // the hsv function expects values from 0 to 360
		// while (hue > 360.0)
		// 	hue -= 360.0;
		// while (hue < 0.0)
		// 	hue += 360.0;
		// ret = hsv2rgb(hue, 0.6, 1.0);

		// int r, g;
		// r = g = min(255, (int) log(iters + 2 - log(log(sqrt(norm)))/log(2)) * 75);
		// SET_RGB(ret, (int) r, (int) g, 255);
		
		// int value = iters < max_iters ? max(0, 255 - 20 * (31 - __builtin_clz(iters))) : 0;
		// SET_RGB(ret, value, value, value);

		// double quotient = (double) iters / 100;
		// double color = quotient > 1.0 ? 1.0 : (quotient < 0.0 ? 0.0 : quotient);
		// // printf("%f\n", color);
		// if (quotient > 0.5)
		// 	ret = rgb(color, 1.0, color);
		// else
		// 	ret = rgb(0, color, 0);


		// double smoothed_value = iters - ln(log(norm) / ln(2)) / ln(2);
		// // more colors to reduce banding
		// double scaled_value = smoothed_value * 20.0;
		// double color = palette.eval_rational(scaled_value, scaled_max_iterations);
	}
	return ret;
}

int min(int a, int b) {
	return a <= b ? a : b;
}
int max(int a, int b) {
	return a >= b ? a : b;
}
