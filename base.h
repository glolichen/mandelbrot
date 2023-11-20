#ifndef BASE_H
#define BASE_H

#define GET_BLUE(num) ((num) & 255)
#define SET_BLUE(num, val) (num) |= (val)

#define GET_GREEN(num) ((num) >> 8 & 255)
#define SET_GREEN(num, val) (num) |= (val << 8)

#define GET_RED(num) ((num) >> 16 & 255)
#define SET_RED(num, val) (num) |= (val << 16)

#define SET_RGB(num, r, g, b) (SET_RED(num, r), SET_GREEN(num, g), SET_BLUE(num, b))

extern int DIRECTION[4][2];

int min(int a, int b);
int max(int a, int b);

int get_color(double norm, int iters, int max_iters);

#endif

