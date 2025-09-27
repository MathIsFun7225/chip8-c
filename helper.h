#ifndef HELPER_H
#define HELPER_H

#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>

#define malloc(size) malloc_msg(size, __FILE__, __LINE__, __func__)
#define calloc(num, size) calloc_msg(num, size, __FILE__, __LINE__, __func__)
#define realloc(ptr, size) realloc_msg(ptr, size, __FILE__, __LINE__, __func__)

void *malloc_msg(ssize_t size, const char *file, int line, const char *func);
void *calloc_msg(ssize_t num, ssize_t size, const char *file, int line, const char *func);
void *realloc_msg(void *ptr, ssize_t size, const char *file, int line, const char *func);

int serialize_16(FILE *f, uint16_t x);
int serialize_32(FILE *f, uint32_t x);
int serialize_64(FILE *f, uint64_t x);
int serialize_float(FILE *f, float x);
int serialize_double(FILE *f, double x);

int deserialize_16(FILE *f, uint16_t *x);
int deserialize_32(FILE *f, uint32_t *x);
int deserialize_64(FILE *f, uint64_t *x);
int deserialize_float(FILE *f, float *x);
int deserialize_double(FILE *f, double *x);

uint64_t make_power_of_2(uint64_t x);

#endif // HELPER_H
