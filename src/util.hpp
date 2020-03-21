#pragma once

#include <cstring>

#define streq(str0, str1) (strcmp(str0, str1) == 0)

/**
 * Determine whether an argument is in argv
 */
bool arg_check(int argc, char** argv, const char* arg);

/**
 * Get the argument after the target arg
 */
char* arg_get(int argc, char** argv, const char* arg);

/**
 * Random stuff
 */
int rand_range(int min, int max);
float rand_uniform();

/**
 * https://en.wikipedia.org/wiki/Fast_inverse_square_root
 */
float fast_sqrtf(float number);

double fast_sin(double x);

double fast_cos(double x);

char* file_read(const char* fname);
