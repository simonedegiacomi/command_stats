#ifndef MY_ASSERT_H
#define MY_ASSERT_H

#include <assert.h>
#include <stdio.h>

#define my_assert(COND, MESSAGE, ...) if (!(COND)) {fprintf(stderr, "[ASSERTION FAILURE] "); fprintf(stderr, MESSAGE, ##__VA_ARGS__); fprintf(stderr, "\n"); assert(COND);} 

#endif