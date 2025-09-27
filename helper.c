#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "helper.h"

void *malloc_msg(ssize_t size, const char *file, int line, const char *func) {
    void *p = (malloc)(size);
    if (p == NULL) {
        fprintf(stderr, "[%s | %d | %s()] malloc(%zd): %s\n", file, line, func, size, strerror(errno));
    }
    return p;
}

void *calloc_msg(ssize_t num, ssize_t size, const char *file, int line, const char *func) {
    void *p = (calloc)(num, size);
    if (p == NULL) {
        fprintf(stderr, "[%s | %d | %s()] calloc(%zd, %zd): %s\n", file, line, func, num, size, strerror(errno));
    }
    return p;
}

void *realloc_msg(void *ptr, ssize_t size, const char *file, int line, const char *func) {
    void *p = (realloc)(ptr, size);
    if (p == NULL) {
        fprintf(stderr, "[%s | %d | %s()] realloc(%p, %zd): %s\n", file, line, func, ptr, size, strerror(errno));
    }
    return p;
}

int serialize_16(FILE *f, uint16_t x) {
    uint8_t bytes[] = {x, x >> 8};
    if (fwrite(bytes, sizeof(bytes), 1, f) == 0) {
        return -1;
    }
    return 0;
}

int serialize_32(FILE *f, uint32_t x) {
    uint8_t bytes[] = {x, x >> 8, x >> 16, x >> 24};
    if (fwrite(bytes, sizeof(bytes), 1, f) == 0) {
        return -1;
    }
    return 0;
}

int serialize_64(FILE *f, uint64_t x) {
    uint8_t bytes[] = {x, x >> 8, x >> 16, x >> 24, x >> 32, x >> 40, x >> 48, x >> 56};
    if (fwrite(bytes, sizeof(bytes), 1, f) == 0) {
        return -1;
    }
    return 0;
}

int serialize_float(FILE *f, float x) {
    uint32_t value = ((union { uint32_t i; float f; }){ .f = x }).i;
    return serialize_32(f, value);
}

int serialize_double(FILE *f, double x) {
    uint64_t value = ((union { uint64_t i; double d; }){ .d = x }).i;
    return serialize_64(f, value);
}

int deserialize_16(FILE *f, uint16_t *x) {
    uint8_t bytes[2];
    if (fread(bytes, sizeof(bytes), 1, f) == 0) {
        return -1;
    }

    *x = ((uint16_t) bytes[0]) | (((uint16_t) bytes[1]) << 8);
    return 0;
}

int deserialize_32(FILE *f, uint32_t *x) {
    uint8_t bytes[4];
    if (fread(bytes, sizeof(bytes), 1, f) == 0) {
        return -1;
    }

    *x = ((uint32_t) bytes[0]) | (((uint32_t) bytes[1]) << 8) | (((uint32_t) bytes[2]) << 16) | (((uint32_t) bytes[3]) << 24);
    return 0;
}

int deserialize_64(FILE *f, uint64_t *x) {
    uint8_t bytes[8];
    if (fread(bytes, sizeof(bytes), 1, f) == 0) {
        return -1;
    }

    *x = ((uint64_t) bytes[0]) | (((uint64_t) bytes[1]) << 8) | (((uint64_t) bytes[2]) << 16) | (((uint64_t) bytes[3]) << 24) \
      | (((uint64_t) bytes[4]) << 32) | (((uint64_t) bytes[5]) << 40) | (((uint64_t) bytes[6]) << 48) | (((uint64_t) bytes[7]) << 56);
    return 0;
}

int deserialize_float(FILE *f, float *x) {
    uint32_t value;
    if (deserialize_32(f, &value) == -1) {
        return -1;
    }
    *x = ((union { uint32_t i; float f; }){ .i = value }).f;
    return 0;
}

int deserialize_double(FILE *f, double *x) {
    uint64_t value;
    if (deserialize_64(f, &value) == -1) {
        return -1;
    }
    *x = ((union { uint64_t i; double d; }){ .i = value }).d;
    return 0;
}

uint64_t make_power_of_2(uint64_t x) {
    return (x == 1) ? 1 : 1ull << (64 - __builtin_clzll(x - 1));
}
