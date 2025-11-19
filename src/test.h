#ifndef TEST_H
#define TEST_H

#include <complex.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>

#define expect_true(cond) check_true(cond, false)
#define assert_true(cond) check_true(cond, true)

#define expect_false(cond) check_false(cond, false)
#define assert_false(cond) check_false(cond, true)

#define expect_null(p) check_null(p, false)
#define assert_null(p) check_null(p, true)

#define expect_non_null(p) check_non_null(p, false)
#define assert_non_null(p) check_non_null(p, true)

#define expect_eq(x, y) check_eq(x, y, false)
#define assert_eq(x, y) check_eq(x, y, true)

#define expect_neq(x, y) check_neq(x, y, false)
#define assert_neq(x, y) check_neq(x, y, true)

#define expect_lt(x, y) check_lt(x, y, false)
#define assert_lt(x, y) check_lt(x, y, true)

#define expect_leq(x, y) check_leq(x, y, false)
#define assert_leq(x, y) check_leq(x, y, true)

#define expect_gt(x, y) check_gt(x, y, false)
#define assert_gt(x, y) check_gt(x, y, true)

#define expect_geq(x, y) check_geq(x, y, false)
#define assert_geq(x, y) check_geq(x, y, true)

#define expect_between(x, y, z) check_between(x, y, z, false, false)
#define assert_between(x, y, z) check_between(x, y, z, false, true)

#define expect_between_strict(x, y, z) check_between(x, y, z, true, false)
#define assert_between_strict(x, y, z) check_between(x, y, z, true, true)

#define expect_eq_bool(x, y) check_eq_bool(x, y, false)
#define assert_eq_bool(x, y) check_eq_bool(x, y, true)

#define expect_neq_bool(x, y) check_neq_bool(x, y, false)
#define assert_neq_bool(x, y) check_neq_bool(x, y, true)

#define expect_eq_char(x, y) check_eq_char(x, y, false)
#define assert_eq_char(x, y) check_eq_char(x, y, true)

#define expect_neq_char(x, y) check_neq_char(x, y, false)
#define assert_neq_char(x, y) check_neq_char(x, y, true)

#define expect_eq_mem(x, y, size) check_eq_mem(x, y, size, false)
#define assert_eq_mem(x, y, size) check_eq_mem(x, y, size, true)

#define expect_neq_mem(x, y, size) check_neq_mem(x, y, size, false)
#define assert_neq_mem(x, y, size) check_neq_mem(x, y, size, true)

#define expect_lt_mem(x, y, size) check_lt_mem(x, y, size, false)
#define assert_lt_mem(x, y, size) check_lt_mem(x, y, size, true)

#define expect_leq_mem(x, y, size) check_leq_mem(x, y, size, false)
#define assert_leq_mem(x, y, size) check_leq_mem(x, y, size, true)

#define expect_gt_mem(x, y, size) check_gt_mem(x, y, size, false)
#define assert_gt_mem(x, y, size) check_gt_mem(x, y, size, true)

#define expect_geq_mem(x, y, size) check_geq_mem(x, y, size, false)
#define assert_geq_mem(x, y, size) check_geq_mem(x, y, size, true)

#define expect_between_mem(x, y, z, size) check_between_mem(x, y, z, size, false, false)
#define assert_between_mem(x, y, z, size) check_between_mem(x, y, z, size, false, true)

#define expect_between_strict_mem(x, y, z, size) check_between_mem(x, y, z, size, true, false)
#define assert_between_strict_mem(x, y, z, size) check_between_mem(x, y, z, size, true, true)

#define check_eq(x, y, abort) \
    _Generic((x), \
        const char *: _Generic((y), \
                               const char *: check_eq_str, \
                                     char *: check_eq_str, \
                                    default: dummy), \
              char *: _Generic((y), \
                               const char *: check_eq_str, \
                                     char *: check_eq_str, \
                                    default: dummy), \
                bool: _Generic((y), \
                                                bool: check_eq_bool,    \
                                       unsigned char: check_eq_uint,    \
                                      unsigned short: check_eq_uint,    \
                                        unsigned int: check_eq_uint,    \
                                       unsigned long: check_eq_uint,    \
                                  unsigned long long: check_eq_uint,    \
                                                char: check_eq_int,     \
                                         signed char: check_eq_int,     \
                                        signed short: check_eq_int,     \
                                          signed int: check_eq_int,     \
                                         signed long: check_eq_int,     \
                                    signed long long: check_eq_int,     \
                                               float: check_eq_float,   \
                                              double: check_eq_float,   \
                                         long double: check_eq_float,   \
                                       float complex: check_eq_complex, \
                                      double complex: check_eq_complex, \
                                 long double complex: check_eq_complex, \
                                             default: dummy), \
                char: _Generic((y), \
                                                bool: check_eq_int,     \
                                       unsigned char: check_eq_int,     \
                                      unsigned short: check_eq_int,     \
                                        unsigned int: check_eq_uint,    \
                                       unsigned long: check_eq_uint,    \
                                  unsigned long long: check_eq_uint,    \
                                                char: check_eq_char,    \
                                         signed char: check_eq_int,     \
                                        signed short: check_eq_int,     \
                                          signed int: check_eq_int,     \
                                         signed long: check_eq_int,     \
                                    signed long long: check_eq_int,     \
                                               float: check_eq_float,   \
                                              double: check_eq_float,   \
                                         long double: check_eq_float,   \
                                       float complex: check_eq_complex, \
                                      double complex: check_eq_complex, \
                                 long double complex: check_eq_complex, \
                                             default: dummy), \
             default: _Generic((x) - (y), \
                                        unsigned int: check_eq_uint,    \
                                       unsigned long: check_eq_uint,    \
                                  unsigned long long: check_eq_uint,    \
                                          signed int: check_eq_int,     \
                                         signed long: check_eq_int,     \
                                    signed long long: check_eq_int,     \
                                               float: check_eq_float,   \
                                              double: check_eq_float,   \
                                         long double: check_eq_float,   \
                                       float complex: check_eq_complex, \
                                      double complex: check_eq_complex, \
                                 long double complex: check_eq_complex, \
                                             default: dummy))(x, #x, y, #y, abort, __FILE__, __LINE__, __func__)

#define check_neq(x, y, abort) \
    _Generic((x), \
              char *: _Generic((y), \
                                char *: check_neq_str, \
                               default: dummy), \
                bool: _Generic((y), \
                                                bool: check_neq_bool,    \
                                       unsigned char: check_neq_uint,    \
                                      unsigned short: check_neq_uint,    \
                                        unsigned int: check_neq_uint,    \
                                       unsigned long: check_neq_uint,    \
                                  unsigned long long: check_neq_uint,    \
                                                char: check_neq_int,     \
                                         signed char: check_neq_int,     \
                                        signed short: check_neq_int,     \
                                          signed int: check_neq_int,     \
                                         signed long: check_neq_int,     \
                                    signed long long: check_neq_int,     \
                                               float: check_neq_float,   \
                                              double: check_neq_float,   \
                                         long double: check_neq_float,   \
                                       float complex: check_neq_complex, \
                                      double complex: check_neq_complex, \
                                 long double complex: check_neq_complex, \
                                             default: dummy), \
                char: _Generic((y), \
                                                bool: check_neq_int,     \
                                       unsigned char: check_neq_int,     \
                                      unsigned short: check_neq_int,     \
                                        unsigned int: check_neq_uint,    \
                                       unsigned long: check_neq_uint,    \
                                  unsigned long long: check_neq_uint,    \
                                                char: check_neq_char,    \
                                         signed char: check_neq_int,     \
                                        signed short: check_neq_int,     \
                                          signed int: check_neq_int,     \
                                         signed long: check_neq_int,     \
                                    signed long long: check_neq_int,     \
                                               float: check_neq_float,   \
                                              double: check_neq_float,   \
                                         long double: check_neq_float,   \
                                       float complex: check_neq_complex, \
                                      double complex: check_neq_complex, \
                                 long double complex: check_neq_complex, \
                                             default: dummy), \
             default: _Generic((x) - (y), \
                                        unsigned int: check_neq_uint,    \
                                       unsigned long: check_neq_uint,    \
                                  unsigned long long: check_neq_uint,    \
                                          signed int: check_neq_int,     \
                                         signed long: check_neq_int,     \
                                    signed long long: check_neq_int,     \
                                               float: check_neq_float,   \
                                              double: check_neq_float,   \
                                         long double: check_neq_float,   \
                                       float complex: check_neq_complex, \
                                      double complex: check_neq_complex, \
                                 long double complex: check_neq_complex, \
                                             default: dummy))(x, #x, y, #y, abort, __FILE__, __LINE__, __func__)

#define check_lt(x, y, abort) \
    _Generic((x), \
              char *: _Generic((y), \
                                char *: check_lt_str, \
                               default: dummy), \
             default: _Generic((x) - (y), \
                                     unsigned int: check_lt_uint,  \
                                    unsigned long: check_lt_uint,  \
                               unsigned long long: check_lt_uint,  \
                                       signed int: check_lt_int,   \
                                      signed long: check_lt_int,   \
                                 signed long long: check_lt_int,   \
                                            float: check_lt_float, \
                                           double: check_lt_float, \
                                      long double: check_lt_float, \
                                          default: dummy))(x, #x, y, #y, abort, __FILE__, __LINE__, __func__)

#define check_leq(x, y, abort) \
    _Generic((x), \
              char *: _Generic((y), \
                                char *: check_leq_str, \
                               default: dummy), \
             default: _Generic((x) - (y), \
                                        unsigned int: check_leq_uint,  \
                                       unsigned long: check_leq_uint,  \
                                  unsigned long long: check_leq_uint,  \
                                          signed int: check_leq_int,   \
                                         signed long: check_leq_int,   \
                                    signed long long: check_leq_int,   \
                                               float: check_leq_float, \
                                              double: check_leq_float, \
                                         long double: check_leq_float, \
                                             default: dummy))(x, #x, y, #y, abort, __FILE__, __LINE__, __func__)

#define check_gt(x, y, abort) \
    _Generic((x), \
              char *: _Generic((y), \
                                char *: check_gt_str, \
                               default: dummy), \
             default: _Generic((x) - (y), \
                                        unsigned int: check_gt_uint,  \
                                       unsigned long: check_gt_uint,  \
                                  unsigned long long: check_gt_uint,  \
                                          signed int: check_gt_int,   \
                                         signed long: check_gt_int,   \
                                    signed long long: check_gt_int,   \
                                               float: check_gt_float, \
                                              double: check_gt_float, \
                                         long double: check_gt_float, \
                                             default: dummy))(x, #x, y, #y, abort, __FILE__, __LINE__, __func__)

#define check_geq(x, y, abort) \
    _Generic((x), \
              char *: _Generic((y), \
                                char *: check_geq_str, \
                               default: dummy), \
             default: _Generic((x) - (y), \
                                        unsigned int: check_geq_uint,  \
                                       unsigned long: check_geq_uint,  \
                                  unsigned long long: check_geq_uint,  \
                                          signed int: check_geq_int,   \
                                         signed long: check_geq_int,   \
                                    signed long long: check_geq_int,   \
                                               float: check_geq_float, \
                                              double: check_geq_float, \
                                         long double: check_geq_float, \
                                             default: dummy))(x, #x, y, #y, abort, __FILE__, __LINE__, __func__)

#define check_between(x, y, z, strict, abort) \
    _Generic((x), \
              char *: _Generic((y), \
                                char *: _Generic((z), \
                                                  char *: check_between_str, \
                                                 default: dummy), \
                               default: dummy), \
             default: _Generic(((x) - (y)) + (z), \
                                        unsigned int: check_between_uint,  \
                                       unsigned long: check_between_uint,  \
                                  unsigned long long: check_between_uint,  \
                                          signed int: check_between_int,   \
                                         signed long: check_between_int,   \
                                    signed long long: check_between_int,   \
                                               float: check_between_float, \
                                              double: check_between_float, \
                                         long double: check_between_float, \
                                             default: dummy))(x, #x, y, #y, z, #z, strict, abort, __FILE__, __LINE__, __func__)

void dummy(void);

#define check_true( cond, abort)  check_true( cond, #cond, abort, __FILE__, __LINE__, __func__)
#define check_false(cond, abort)  check_false(cond, #cond, abort, __FILE__, __LINE__, __func__)

void (check_true) (bool cond, const char *cond_str, bool abort, const char *file, int line, const char *func);
void (check_false)(bool cond, const char *cond_str, bool abort, const char *file, int line, const char *func);


#define check_null(   p, abort)  check_null(   p, #p, abort, __FILE__, __LINE__, __func__)
#define check_non_null(p, abort)  check_non_null(p, #p, abort, __FILE__, __LINE__, __func__)

void (check_null)   (const void *p, const char *p_str, bool abort, const char *file, int line, const char *func);
void (check_non_null)(const void *p, const char *p_str, bool abort, const char *file, int line, const char *func);


#define check_eq_bool( x, y, abort)  check_eq_bool( x, #x, y, #y, abort, __FILE__, __LINE__, __func__)
#define check_neq_bool(x, y, abort)  check_neq_bool(x, #x, y, #y, abort, __FILE__, __LINE__, __func__)

void (check_eq_bool) (bool x, const char *x_str, bool y, const char *y_str, bool abort, const char *file, int line, const char *func);
void (check_neq_bool)(bool x, const char *x_str, bool y, const char *y_str, bool abort, const char *file, int line, const char *func);


#define check_eq_char( x, y, abort)  check_eq_char( x, #x, y, #y, abort, __FILE__, __LINE__, __func__)
#define check_neq_char(x, y, abort)  check_neq_char(x, #x, y, #y, abort, __FILE__, __LINE__, __func__)

void (check_eq_char) (char x, const char *x_str, char y, const char *y_str, bool abort, const char *file, int line, const char *func);
void (check_neq_char)(char x, const char *x_str, char y, const char *y_str, bool abort, const char *file, int line, const char *func);


#define check_eq_uint( x, y, abort)  check_eq_uint( x, #x, y, #y, abort, __FILE__, __LINE__, __func__)
#define check_neq_uint(x, y, abort)  check_neq_uint(x, #x, y, #y, abort, __FILE__, __LINE__, __func__)
#define check_lt_uint( x, y, abort)  check_lt_uint( x, #x, y, #y, abort, __FILE__, __LINE__, __func__)
#define check_leq_uint(x, y, abort)  check_leq_uint(x, #x, y, #y, abort, __FILE__, __LINE__, __func__)
#define check_gt_uint( x, y, abort)  check_gt_uint( x, #x, y, #y, abort, __FILE__, __LINE__, __func__)
#define check_geq_uint(x, y, abort)  check_geq_uint(x, #x, y, #y, abort, __FILE__, __LINE__, __func__)

void (check_eq_uint) (uintmax_t x, const char *x_str, uintmax_t y, const char *y_str, bool abort, const char *file, int line, const char *func);
void (check_neq_uint)(uintmax_t x, const char *x_str, uintmax_t y, const char *y_str, bool abort, const char *file, int line, const char *func);
void (check_lt_uint) (uintmax_t x, const char *x_str, uintmax_t y, const char *y_str, bool abort, const char *file, int line, const char *func);
void (check_leq_uint)(uintmax_t x, const char *x_str, uintmax_t y, const char *y_str, bool abort, const char *file, int line, const char *func);
void (check_gt_uint) (uintmax_t x, const char *x_str, uintmax_t y, const char *y_str, bool abort, const char *file, int line, const char *func);
void (check_geq_uint)(uintmax_t x, const char *x_str, uintmax_t y, const char *y_str, bool abort, const char *file, int line, const char *func);


#define check_eq_int( x, y, abort)  check_eq_int( x, #x, y, #y, abort, __FILE__, __LINE__, __func__)
#define check_neq_int(x, y, abort)  check_neq_int(x, #x, y, #y, abort, __FILE__, __LINE__, __func__)
#define check_lt_int( x, y, abort)  check_lt_int( x, #x, y, #y, abort, __FILE__, __LINE__, __func__)
#define check_leq_int(x, y, abort)  check_leq_int(x, #x, y, #y, abort, __FILE__, __LINE__, __func__)
#define check_gt_int( x, y, abort)  check_gt_int( x, #x, y, #y, abort, __FILE__, __LINE__, __func__)
#define check_geq_int(x, y, abort)  check_geq_int(x, #x, y, #y, abort, __FILE__, __LINE__, __func__)

void (check_eq_int) (intmax_t x, const char *x_str, intmax_t y, const char *y_str, bool abort, const char *file, int line, const char *func);
void (check_neq_int)(intmax_t x, const char *x_str, intmax_t y, const char *y_str, bool abort, const char *file, int line, const char *func);
void (check_lt_int) (intmax_t x, const char *x_str, intmax_t y, const char *y_str, bool abort, const char *file, int line, const char *func);
void (check_leq_int)(intmax_t x, const char *x_str, intmax_t y, const char *y_str, bool abort, const char *file, int line, const char *func);
void (check_gt_int) (intmax_t x, const char *x_str, intmax_t y, const char *y_str, bool abort, const char *file, int line, const char *func);
void (check_geq_int)(intmax_t x, const char *x_str, intmax_t y, const char *y_str, bool abort, const char *file, int line, const char *func);


#define check_eq_float( x, y, abort)  check_eq_float( x, #x, y, #y, abort, __FILE__, __LINE__, __func__)
#define check_neq_float(x, y, abort)  check_neq_float(x, #x, y, #y, abort, __FILE__, __LINE__, __func__)
#define check_lt_float( x, y, abort)  check_lt_float( x, #x, y, #y, abort, __FILE__, __LINE__, __func__)
#define check_leq_float(x, y, abort)  check_leq_float(x, #x, y, #y, abort, __FILE__, __LINE__, __func__)
#define check_gt_float( x, y, abort)  check_gt_float( x, #x, y, #y, abort, __FILE__, __LINE__, __func__)
#define check_geq_float(x, y, abort)  check_geq_float(x, #x, y, #y, abort, __FILE__, __LINE__, __func__)

void (check_eq_float) (long double x, const char *x_str, long double y, const char *y_str, bool abort, const char *file, int line, const char *func);
void (check_neq_float)(long double x, const char *x_str, long double y, const char *y_str, bool abort, const char *file, int line, const char *func);
void (check_lt_float) (long double x, const char *x_str, long double y, const char *y_str, bool abort, const char *file, int line, const char *func);
void (check_leq_float)(long double x, const char *x_str, long double y, const char *y_str, bool abort, const char *file, int line, const char *func);
void (check_gt_float) (long double x, const char *x_str, long double y, const char *y_str, bool abort, const char *file, int line, const char *func);
void (check_geq_float)(long double x, const char *x_str, long double y, const char *y_str, bool abort, const char *file, int line, const char *func);


#define check_eq_complex( x, y, abort)  check_eq_complex( x, #x, y, #y, abort, __FILE__, __LINE__, __func__)
#define check_neq_complex(x, y, abort)  check_neq_complex(x, #x, y, #y, abort, __FILE__, __LINE__, __func__)

void (check_eq_complex) (long double complex x, const char *x_str, long double complex y, const char *y_str, bool abort, const char *file, int line, const char *func);
void (check_neq_complex)(long double complex x, const char *x_str, long double complex y, const char *y_str, bool abort, const char *file, int line, const char *func);


#define check_eq_str( x, y, abort)  check_eq_str( x, #x, y, #y, abort, __FILE__, __LINE__, __func__)
#define check_neq_str(x, y, abort)  check_neq_str(x, #x, y, #y, abort, __FILE__, __LINE__, __func__)
#define check_lt_str( x, y, abort)  check_lt_str( x, #x, y, #y, abort, __FILE__, __LINE__, __func__)
#define check_leq_str(x, y, abort)  check_leq_str(x, #x, y, #y, abort, __FILE__, __LINE__, __func__)
#define check_gt_str( x, y, abort)  check_gt_str( x, #x, y, #y, abort, __FILE__, __LINE__, __func__)
#define check_geq_str(x, y, abort)  check_geq_str(x, #x, y, #y, abort, __FILE__, __LINE__, __func__)

void (check_eq_str) (const char *x, const char *x_str, const char *y, const char *y_str, bool abort, const char *file, int line, const char *func);
void (check_neq_str)(const char *x, const char *x_str, const char *y, const char *y_str, bool abort, const char *file, int line, const char *func);
void (check_lt_str) (const char *x, const char *x_str, const char *y, const char *y_str, bool abort, const char *file, int line, const char *func);
void (check_leq_str)(const char *x, const char *x_str, const char *y, const char *y_str, bool abort, const char *file, int line, const char *func);
void (check_gt_str) (const char *x, const char *x_str, const char *y, const char *y_str, bool abort, const char *file, int line, const char *func);
void (check_geq_str)(const char *x, const char *x_str, const char *y, const char *y_str, bool abort, const char *file, int line, const char *func);


#define check_eq_mem( x, y, size, abort)  check_eq_mem( x, #x, y, #y, size, abort, __FILE__, __LINE__, __func__)
#define check_neq_mem(x, y, size, abort)  check_neq_mem(x, #x, y, #y, size, abort, __FILE__, __LINE__, __func__)
#define check_lt_mem( x, y, size, abort)  check_lt_mem( x, #x, y, #y, size, abort, __FILE__, __LINE__, __func__)
#define check_leq_mem(x, y, size, abort)  check_leq_mem(x, #x, y, #y, size, abort, __FILE__, __LINE__, __func__)
#define check_gt_mem( x, y, size, abort)  check_gt_mem( x, #x, y, #y, size, abort, __FILE__, __LINE__, __func__)
#define check_geq_mem(x, y, size, abort)  check_geq_mem(x, #x, y, #y, size, abort, __FILE__, __LINE__, __func__)

void (check_eq_mem) (const void *x, const char *x_str, const void *y, const char *y_str, ssize_t size, bool abort, const char *file, int line, const char *func);
void (check_neq_mem)(const void *x, const char *x_str, const void *y, const char *y_str, ssize_t size, bool abort, const char *file, int line, const char *func);
void (check_lt_mem) (const void *x, const char *x_str, const void *y, const char *y_str, ssize_t size, bool abort, const char *file, int line, const char *func);
void (check_leq_mem)(const void *x, const char *x_str, const void *y, const char *y_str, ssize_t size, bool abort, const char *file, int line, const char *func);
void (check_gt_mem) (const void *x, const char *x_str, const void *y, const char *y_str, ssize_t size, bool abort, const char *file, int line, const char *func);
void (check_geq_mem)(const void *x, const char *x_str, const void *y, const char *y_str, ssize_t size, bool abort, const char *file, int line, const char *func);


#define check_between_uint( x, y, z,       strict, abort)  check_between_uint( x, #x, y, #y, z, #z,       strict, abort, __FILE__, __LINE__, __func__)
#define check_between_int(  x, y, z,       strict, abort)  check_between_int(  x, #x, y, #y, z, #z,       strict, abort, __FILE__, __LINE__, __func__)
#define check_between_float(x, y, z,       strict, abort)  check_between_float(x, #x, y, #y, z, #z,       strict, abort, __FILE__, __LINE__, __func__)
#define check_between_str(  x, y, z,       strict, abort)  check_between_str(  x, #x, y, #y, z, #z,       strict, abort, __FILE__, __LINE__, __func__)
#define check_between_mem(  x, y, z, size, strict, abort)  check_between_mem(  x, #x, y, #y, z, #z, size, strict, abort, __FILE__, __LINE__, __func__)

void (check_between_uint) (uintmax_t   x, const char *x_str, uintmax_t   y, const char *y_str, uintmax_t   z, const char *z_str,               bool strict, bool abort, const char *file, int line, const char *func);
void (check_between_int)  (intmax_t    x, const char *x_str, intmax_t    y, const char *y_str, intmax_t    z, const char *z_str,               bool strict, bool abort, const char *file, int line, const char *func);
void (check_between_float)(long double x, const char *x_str, long double y, const char *y_str, long double z, const char *z_str,               bool strict, bool abort, const char *file, int line, const char *func);
void (check_between_str)  (const char *x, const char *x_str, const char *y, const char *y_str, const char *z, const char *z_str,               bool strict, bool abort, const char *file, int line, const char *func);
void (check_between_mem)  (const void *x, const char *x_str, const void *y, const char *y_str, const void *z, const char *z_str, ssize_t size, bool strict, bool abort, const char *file, int line, const char *func);

void summarize_tests(void);

#endif // TEST_H
