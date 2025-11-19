#include <complex.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

static ssize_t g_num_tests = 0;
static ssize_t g_num_passed_tests = 0;
static ssize_t g_num_failed_tests = 0;

static const char *bool_to_str(bool x) {
    return x ? "true" : "false";
}

static const char *const char_str[] = {
    "\\x00", "\\x01", "\\x02", "\\x03", "\\x04", "\\x05", "\\x06", "\\a", "\\b", "\\t", "\\n", "\\v", "\\f", "\\r", "\\x0e", "\\x0f",
    "\\x10", "\\x11", "\\x12", "\\x13", "\\x14", "\\x15", "\\x16", "\\x17", "\\x18", "\\x19", "\\x1a", "\\e", "\\x1c", "\\x1d", "\\x1e", "\\x1f",
    " ", "!", "\"", "#", "$", "%", "&", "'", "(", ")", "*", "+", ",", "-", ".", "/",
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", ":", ";", "<", "=", ">", "?",
    "@", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O",
    "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "[", "\\", "]", "^", "_",
    "`", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o",
    "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "{", "|", "}", "~", "\\x7f",
    "\\x80", "\\x81", "\\x82", "\\x83", "\\x84", "\\x85", "\\x86", "\\x87", "\\x88", "\\x89", "\\x8a", "\\x8b", "\\x8c", "\\x8d", "\\x8e", "\\x8f",
    "\\x90", "\\x91", "\\x92", "\\x93", "\\x94", "\\x95", "\\x96", "\\x97", "\\x98", "\\x99", "\\x9a", "\\x9b", "\\x9c", "\\x9d", "\\x9e", "\\x9f",
    "\\xa0", "\\xa1", "\\xa2", "\\xa3", "\\xa4", "\\xa5", "\\xa6", "\\xa7", "\\xa8", "\\xa9", "\\xaa", "\\xab", "\\xac", "\\xad", "\\xae", "\\xaf",
    "\\xb0", "\\xb1", "\\xb2", "\\xb3", "\\xb4", "\\xb5", "\\xb6", "\\xb7", "\\xb8", "\\xb9", "\\xba", "\\xbb", "\\xbc", "\\xbd", "\\xbe", "\\xbf",
    "\\xc0", "\\xc1", "\\xc2", "\\xc3", "\\xc4", "\\xc5", "\\xc6", "\\xc7", "\\xc8", "\\xc9", "\\xca", "\\xcb", "\\xcc", "\\xcd", "\\xce", "\\xcf",
    "\\xd0", "\\xd1", "\\xd2", "\\xd3", "\\xd4", "\\xd5", "\\xd6", "\\xd7", "\\xd8", "\\xd9", "\\xda", "\\xdb", "\\xdc", "\\xdd", "\\xde", "\\xdf",
    "\\xe0", "\\xe1", "\\xe2", "\\xe3", "\\xe4", "\\xe5", "\\xe6", "\\xe7", "\\xe8", "\\xe9", "\\xea", "\\xeb", "\\xec", "\\xed", "\\xee", "\\xef",
    "\\xf0", "\\xf1", "\\xf2", "\\xf3", "\\xf4", "\\xf5", "\\xf6", "\\xf7", "\\xf8", "\\xf9", "\\xfa", "\\xfb", "\\xfc", "\\xfd", "\\xfe", "\\xff"
};

static const char *char_to_str(char c) {
    return char_str[(unsigned char) c];
}

static int strcmp_index(const char *s1, const char *s2, ssize_t *index) {
    ssize_t i;
    for (i = 0; s1[i] != '\0' && s1[i] == s2[i]; i++);
    *index = i;
    return (unsigned char) s1[i] - (unsigned char) s2[i];
}

static int memcmp_index(const void *p1, const void *p2, ssize_t size, ssize_t *index) {
    const unsigned char *a1 = p1;
    const unsigned char *a2 = p2;

    ssize_t i = 0;
    while (i < size) {
        unsigned char x1 = a1[i];
        unsigned char x2 = a2[i];
        if (x1 != x2) {
            *index = i;
            return x1 - x2;
        }
        i++;
    }

    *index = size - 1;
    return 0;
}

static unsigned char get_byte(const void *p, ssize_t index) {
    return ((const unsigned char *) p)[index];
}

static int str_between_index(const char *s1, const char *s2, const char *s3, bool strict, ssize_t *index) {
    ssize_t index2, index3;

    int cmp2 = strcmp_index(s1, s2, &index2);
    int cmp3 = strcmp_index(s1, s3, &index3);

    bool cond2 = strict ? (cmp2 <= 0) : (cmp2 < 0);
    bool cond3 = strict ? (cmp3 >= 0) : (cmp3 > 0);

    if (cond2) {
        if (cond3) {
            if (index2 <= index3) {
                *index = index2;
                return -1;
            } else {
                *index = index3;
                return 1;
            }
        } else {
            *index = index2;
            return -1;
        }
    } else if (cond3) {
        *index = index3;
        return 1;
    }
    
    *index = -1;
    return 0;
}

static int mem_between_index(const void *s1, const void *s2, const void *s3, ssize_t size, bool strict, ssize_t *index) {
    ssize_t index2, index3;

    int cmp2 = memcmp_index(s1, s2, size, &index2);
    int cmp3 = memcmp_index(s1, s3, size, &index3);

    bool cond2 = strict ? (cmp2 <= 0) : (cmp2 < 0);
    bool cond3 = strict ? (cmp3 >= 0) : (cmp3 > 0);

    if (cond2) {
        if (cond3) {
            if (index2 <= index3) {
                *index = index2;
                return -1;
            } else {
                *index = index3;
                return 1;
            }
        } else {
            *index = index2;
            return -1;
        }
    } else if (cond3) {
        *index = index3;
        return 1;
    }
    
    *index = -1;
    return 0;
}

#define define_check_arg(func_name, arg_type, arg_name, cond, expected) \
    void (func_name)(arg_type arg_name, const char *arg_name ## _str, bool abort, const char *file, int line, const char *func) { \
        g_num_tests++; \
        if (cond) { \
            g_num_passed_tests++; \
        } else { \
            g_num_failed_tests++; \
            fprintf(stderr, "[%s L%d %s()] Failed test %zd: expected %s to be " expected "\n", file, line, func, g_num_tests, arg_name ## _str); \
            if (abort) { \
                exit(EXIT_FAILURE); \
            } \
        } \
    }

define_check_arg(check_true,  bool, cond, cond,  "true")
define_check_arg(check_false, bool, cond, !cond, "false")

define_check_arg(check_null,     const void *, p, p == NULL, "NULL")
define_check_arg(check_non_null, const void *, p, p != NULL, "not NULL")

#define define_check_cmp(func_name, arg_type, cmp, arg_spec, arg_func) \
    void (func_name) (arg_type x, const char *x_str, arg_type y, const char *y_str, bool abort, const char *file, int line, const char *func) { \
        g_num_tests++; \
        if (x cmp y) { \
            g_num_passed_tests++; \
        } else { \
            g_num_failed_tests++; \
            fprintf(stderr, "[%s L%d %s()] Failed test %zd: expected %s " #cmp " %s; got " arg_spec ", " arg_spec "\n", file, line, func, g_num_tests, x_str, y_str, arg_func(x), arg_func(y)); \
            if (abort) { \
                exit(EXIT_FAILURE); \
            } \
        } \
    }

define_check_cmp(check_eq_bool,  bool, ==, "%s", bool_to_str)
define_check_cmp(check_neq_bool, bool, !=, "%s", bool_to_str)

define_check_cmp(check_eq_char,  char, ==, "%s", char_to_str)
define_check_cmp(check_neq_char, char, !=, "%s", char_to_str)

define_check_cmp(check_eq_uint,  uintmax_t, ==, "%ju", )
define_check_cmp(check_neq_uint, uintmax_t, !=, "%ju", )
define_check_cmp(check_lt_uint,  uintmax_t, <,  "%ju", )
define_check_cmp(check_leq_uint, uintmax_t, <=, "%ju", )
define_check_cmp(check_gt_uint,  uintmax_t, >,  "%ju", )
define_check_cmp(check_geq_uint, uintmax_t, >=, "%ju", )

define_check_cmp(check_eq_int,  intmax_t, ==, "%jd", )
define_check_cmp(check_neq_int, intmax_t, !=, "%jd", )
define_check_cmp(check_lt_int,  intmax_t, <,  "%jd", )
define_check_cmp(check_leq_int, intmax_t, <=, "%jd", )
define_check_cmp(check_gt_int,  intmax_t, >,  "%jd", )
define_check_cmp(check_geq_int, intmax_t, >=, "%jd", )

define_check_cmp(check_eq_float,  long double, ==, "%Lg", )
define_check_cmp(check_neq_float, long double, !=, "%Lg", )
define_check_cmp(check_lt_float,  long double, <,  "%Lg", )
define_check_cmp(check_leq_float, long double, <=, "%Lg", )
define_check_cmp(check_gt_float,  long double, >,  "%Lg", )
define_check_cmp(check_geq_float, long double, >=, "%Lg", )

void (check_eq_complex)(long double complex x, const char *x_str, long double complex y, const char *y_str, bool abort, const char *file, int line, const char *func) {
    g_num_tests++;
    if (x == y) {
        g_num_passed_tests++;
    } else {
        g_num_failed_tests++;

        long double x_imag = cimagl(x);
        long double y_imag = cimagl(y);
        char x_sign = x_imag > 0 ? '+' : '-';
        char y_sign = y_imag > 0 ? '+' : '-';
        if (x_imag < 0) x_imag = -x_imag;
        if (y_imag < 0) y_imag = -y_imag;

        fprintf(stderr, "[%s L%d %s()] Failed test %zd: expected %s == %s; got %Lg %c %Lgi, %Lg %c %Lgi\n", file, line, func, g_num_tests, x_str, y_str, creall(x), x_sign, x_imag, creall(y), y_sign, y_imag);
        if (abort) {
            exit(EXIT_FAILURE);
        }
    }
}

void (check_neq_complex)(long double complex x, const char *x_str, long double complex y, const char *y_str, bool abort, const char *file, int line, const char *func) {
    g_num_tests++;
    if (x != y) {
        g_num_passed_tests++;
    } else {
        g_num_failed_tests++;

        long double real = creall(x);
        long double imag = cimagl(x);
        char sign = imag > 0 ? '+' : '-';
        if (imag < 0) imag = -imag;

        fprintf(stderr, "[%s L%d %s()] Failed test %zd: expected %s != %s; got %Lg %c %Lgi, %Lg %c %Lgi\n", file, line, func, g_num_tests, x_str, y_str, real, sign, imag, real, sign, imag);
        if (abort) {
            exit(EXIT_FAILURE);
        }
    }
}

void (check_eq_str)(const char *x, const char *x_str, const char *y, const char *y_str, bool abort, const char *file, int line, const char *func) {
    g_num_tests++;

    ssize_t index;
    if (strcmp_index(x, y, &index) == 0) {
        g_num_passed_tests++;
    } else {
        g_num_failed_tests++;

        fprintf(stderr, "[%s L%d %s()] Failed test %zd: expected %s == %s; got \"%s\", \"%s\" (index %zu)\n", file, line, func, g_num_tests, x_str, y_str, x, y, index);
        if (abort) {
            exit(EXIT_FAILURE);
        }
    }
}

void (check_neq_str)(const char *x, const char *x_str, const char *y, const char *y_str, bool abort, const char *file, int line, const char *func) {
    g_num_tests++;

    if (strcmp(x, y) != 0) {
        g_num_passed_tests++;
    } else {
        g_num_failed_tests++;

        fprintf(stderr, "[%s L%d %s()] Failed test %zd: expected %s != %s; got \"%s\" (equal)\n", file, line, func, g_num_tests, x_str, y_str, x);
        if (abort) {
            exit(EXIT_FAILURE);
        }
    }
}

void (check_lt_str)(const char *x, const char *x_str, const char *y, const char *y_str, bool abort, const char *file, int line, const char *func) {
    g_num_tests++;

    ssize_t index;
    int cmp = strcmp_index(x, y, &index);
    if (cmp < 0) {
        g_num_passed_tests++;
    } else {
        g_num_failed_tests++;

        if (cmp == 0) {
            fprintf(stderr, "[%s L%d %s()] Failed test %zd: expected %s < %s; got \"%s\" (equal)\n", file, line, func, g_num_tests, x_str, y_str, x);
        } else {
            fprintf(stderr, "[%s L%d %s()] Failed test %zd: expected %s < %s; got \"%s\", \"%s\" (index %zu)\n", file, line, func, g_num_tests, x_str, y_str, x, y, index);
        }
        if (abort) {
            exit(EXIT_FAILURE);
        }
    }
}

void (check_leq_str)(const char *x, const char *x_str, const char *y, const char *y_str, bool abort, const char *file, int line, const char *func) {
    g_num_tests++;

    ssize_t index;
    if (strcmp_index(x, y, &index) <= 0) {
        g_num_passed_tests++;
    } else {
        g_num_failed_tests++;

        fprintf(stderr, "[%s L%d %s()] Failed test %zd: expected %s <= %s; got \"%s\", \"%s\" (index %zu)\n", file, line, func, g_num_tests, x_str, y_str, x, y, index);
        if (abort) {
            exit(EXIT_FAILURE);
        }
    }
}

void (check_gt_str)(const char *x, const char *x_str, const char *y, const char *y_str, bool abort, const char *file, int line, const char *func) {
    g_num_tests++;

    ssize_t index;
    int cmp = strcmp_index(x, y, &index);
    if (cmp > 0) {
        g_num_passed_tests++;
    } else {
        g_num_failed_tests++;

        if (cmp == 0) {
            fprintf(stderr, "[%s L%d %s()] Failed test %zd: expected %s > %s; got \"%s\" (equal)\n", file, line, func, g_num_tests, x_str, y_str, x);
        } else {
            fprintf(stderr, "[%s L%d %s()] Failed test %zd: expected %s > %s; got \"%s\", \"%s\" (index %zu)\n", file, line, func, g_num_tests, x_str, y_str, x, y, index);
        }
        if (abort) {
            exit(EXIT_FAILURE);
        }
    }
}

void (check_geq_str)(const char *x, const char *x_str, const char *y, const char *y_str, bool abort, const char *file, int line, const char *func) {
    g_num_tests++;

    ssize_t index;
    if (strcmp_index(x, y, &index) >= 0) {
        g_num_passed_tests++;
    } else {
        g_num_failed_tests++;

        fprintf(stderr, "[%s L%d %s()] Failed test %zd: expected %s >= %s; got \"%s\", \"%s\" (index %zu)\n", file, line, func, g_num_tests, x_str, y_str, x, y, index);
        if (abort) {
            exit(EXIT_FAILURE);
        }
    }
}

void (check_eq_mem)(const void *x, const char *x_str, const void *y, const char *y_str, ssize_t size, bool abort, const char *file, int line, const char *func) {
    g_num_tests++;

    ssize_t index;
    if (memcmp_index(x, y, size, &index) == 0) {
        g_num_passed_tests++;
    } else {
        g_num_failed_tests++;

        fprintf(stderr, "[%s L%d %s()] Failed test %zd: expected %s == %s; got \\x%02hhx, \\x%02hhx at index %zu\n", file, line, func, g_num_tests, x_str, y_str, get_byte(x, index), get_byte(y, index), index);
        if (abort) {
            exit(EXIT_FAILURE);
        }
    }
}

void (check_neq_mem)(const void *x, const char *x_str, const void *y, const char *y_str, ssize_t size, bool abort, const char *file, int line, const char *func) {
    g_num_tests++;

    if (memcmp(x, y, size) != 0) {
        g_num_passed_tests++;
    } else {
        g_num_failed_tests++;

        fprintf(stderr, "[%s L%d %s()] Failed test %zd: expected %s != %s; got ", file, line, func, g_num_tests, x_str, y_str);

        ssize_t preview = 8;
        if (size <= 2 * preview) {
            for (ssize_t i = 0; i < size; i++) {
                fprintf(stderr, "\\x%02hhx%c", get_byte(x, i), (i == size - 1) ? '\n' : ' ');
            }
        } else {
            for (ssize_t i = 0; i < preview; i++) {
                fprintf(stderr, "\\x%02hhx ", get_byte(x, i));
            }
            fputs("... ", stderr);
            for (ssize_t i = size - preview; i < size; i++) {
                fprintf(stderr, "\\x%02hhx ", get_byte(x, i));
            }
            fputs("(equal)\n", stderr);
        }

        if (abort) {
            exit(EXIT_FAILURE);
        }
    }
}

void (check_lt_mem)(const void *x, const char *x_str, const void *y, const char *y_str, ssize_t size, bool abort, const char *file, int line, const char *func) {
    g_num_tests++;

    ssize_t index;
    int cmp = memcmp_index(x, y, size, &index);
    if (cmp < 0) {
        g_num_passed_tests++;
    } else {
        g_num_failed_tests++;

        if (cmp == 0) {
            fprintf(stderr, "[%s L%d %s()] Failed test %zd: expected %s < %s; got ", file, line, func, g_num_tests, x_str, y_str);

            ssize_t preview = 8;
            if (size < 2 * preview) {
                for (ssize_t i = 0; i < size; i++) {
                    fprintf(stderr, "\\x%02hhx%c", get_byte(x, i), (i == size - 1) ? '\n' : ' ');
                }
            } else {
                for (ssize_t i = 0; i < preview; i++) {
                    fprintf(stderr, "\\x%02hhx ", get_byte(x, i));
                }
                fputs("... ", stderr);
                for (ssize_t i = size - preview; i < size; i++) {
                    fprintf(stderr, "\\x%02hhx ", get_byte(x, i));
                }
                fputs("(equal)\n", stderr);
            }
        } else {
            fprintf(stderr, "[%s L%d %s()] Failed test %zd: expected %s < %s; got \\x%02hhx, \\x%02hhx at index %zu\n", file, line, func, g_num_tests, x_str, y_str, get_byte(x, index), get_byte(y, index), index);
        }

        fprintf(stderr, "[%s L%d %s()] Failed test %zd: expected %s < %s; got \\x%02hhx, \\x%02hhx at index %zu\n", file, line, func, g_num_tests, x_str, y_str, get_byte(x, index), get_byte(y, index), index);
        if (abort) {
            exit(EXIT_FAILURE);
        }
    }
}

void (check_leq_mem)(const void *x, const char *x_str, const void *y, const char *y_str, ssize_t size, bool abort, const char *file, int line, const char *func) {
    g_num_tests++;

    ssize_t index;
    if (memcmp_index(x, y, size, &index) <= 0) {
        g_num_passed_tests++;
    } else {
        g_num_failed_tests++;

        fprintf(stderr, "[%s L%d %s()] Failed test %zd: expected %s <= %s; got \\x%02hhx, \\x%02hhx at index %zu\n", file, line, func, g_num_tests, x_str, y_str, get_byte(x, index), get_byte(y, index), index);
        if (abort) {
            exit(EXIT_FAILURE);
        }
    }
}

void (check_gt_mem)(const void *x, const char *x_str, const void *y, const char *y_str, ssize_t size, bool abort, const char *file, int line, const char *func) {
    g_num_tests++;

    ssize_t index;
    int cmp = memcmp_index(x, y, size, &index);
    if (cmp > 0) {
        g_num_passed_tests++;
    } else {
        g_num_failed_tests++;

        if (cmp == 0) {
            fprintf(stderr, "[%s L%d %s()] Failed test %zd: expected %s > %s; got ", file, line, func, g_num_tests, x_str, y_str);

            ssize_t preview = 8;
            if (size < 2 * preview) {
                for (ssize_t i = 0; i < size; i++) {
                    fprintf(stderr, "\\x%02hhx%c", get_byte(x, i), (i == size - 1) ? '\n' : ' ');
                }
            } else {
                for (ssize_t i = 0; i < preview; i++) {
                    fprintf(stderr, "\\x%02hhx ", get_byte(x, i));
                }
                fputs("... ", stderr);
                for (ssize_t i = size - preview; i < size; i++) {
                    fprintf(stderr, "\\x%02hhx ", get_byte(x, i));
                }
                fputs("(equal)\n", stderr);
            }
        } else {
            fprintf(stderr, "[%s L%d %s()] Failed test %zd: expected %s > %s; got \\x%02hhx, \\x%02hhx at index %zu\n", file, line, func, g_num_tests, x_str, y_str, get_byte(x, index), get_byte(y, index), index);
        }

        if (abort) {
            exit(EXIT_FAILURE);
        }
    }
}

void (check_geq_mem)(const void *x, const char *x_str, const void *y, const char *y_str, ssize_t size, bool abort, const char *file, int line, const char *func) {
    g_num_tests++;

    ssize_t index;
    if (memcmp_index(x, y, size, &index) >= 0) {
        g_num_passed_tests++;
    } else {
        g_num_failed_tests++;

        fprintf(stderr, "[%s L%d %s()] Failed test %zd: expected %s >= %s; got \\x%02hhx, \\x%02hhx at index %zu\n", file, line, func, g_num_tests, x_str, y_str, get_byte(x, index), get_byte(y, index), index);
        if (abort) {
            exit(EXIT_FAILURE);
        }
    }
}

void (check_between_uint)(uintmax_t x, const char *x_str, uintmax_t y, const char *y_str, uintmax_t z, const char *z_str, bool strict, bool abort, const char *file, int line, const char *func) {
    g_num_tests++;

    bool cond = strict ? (x > y && x < z) : (x >= y && x <= z);
    const char *cmp_str = strict ? "<" : "<=";

    if (cond) {
        g_num_passed_tests++;
    } else {
        g_num_failed_tests++;
        fprintf(stderr, "[%s L%d %s()] Failed test %zd: expected %s %s %s %s %s; got %ju, %ju, %ju\n", file, line, func, g_num_tests, y_str, cmp_str, x_str, cmp_str, z_str, y, x, z);
        if (abort) {
            exit(EXIT_FAILURE);
        }
    }
}

void (check_between_int)(intmax_t x, const char *x_str, intmax_t y, const char *y_str, intmax_t z, const char *z_str, bool strict, bool abort, const char *file, int line, const char *func) {
    g_num_tests++;

    bool cond = strict ? (x > y && x < z) : (x >= y && x <= z);
    const char *cmp_str = strict ? "<" : "<=";

    if (cond) {
        g_num_passed_tests++;
    } else {
        g_num_failed_tests++;
        fprintf(stderr, "[%s L%d %s()] Failed test %zd: expected %s %s %s %s %s; got %jd, %jd, %jd\n", file, line, func, g_num_tests, y_str, cmp_str, x_str, cmp_str, z_str, y, x, z);
        if (abort) {
            exit(EXIT_FAILURE);
        }
    }
}

void (check_between_float)(long double x, const char *x_str, long double y, const char *y_str, long double z, const char *z_str, bool strict, bool abort, const char *file, int line, const char *func) {
    g_num_tests++;

    bool cond = strict ? (x > y && x < z) : (x >= y && x <= z);
    const char *cmp_str = strict ? "<" : "<=";

    if (cond) {
        g_num_passed_tests++;
    } else {
        g_num_failed_tests++;
        fprintf(stderr, "[%s L%d %s()] Failed test %zd: expected %s %s %s %s %s; got %Lg, %Lg, %Lg\n", file, line, func, g_num_tests, y_str, cmp_str, x_str, cmp_str, z_str, y, x, z);
        if (abort) {
            exit(EXIT_FAILURE);
        }
    }
}

void (check_between_str)(const char *x, const char *x_str, const char *y, const char *y_str, const char *z, const char *z_str, bool strict, bool abort, const char *file, int line, const char *func) {
    g_num_tests++;
    
    ssize_t index;
    int cmp = str_between_index(x, y, z, strict, &index);
    const char *cmp_str = strict ? "<" : "<=";
    const char *desc;
    if (cmp == 0) {
        desc = "between";
    } else if (cmp < 0) {
        desc = strict ? "leq" : "less";
    } else {
        desc = strict ? "geq" : "greater";
    }

    if (cmp == 0) {
        g_num_passed_tests++;
    } else {
        g_num_failed_tests++;
        fprintf(stderr, "[%s L%d %s()] Failed test %zd: expected %s %s %s %s %s; got \"%s\", \"%s\", \"%s\" (%s)\n", file, line, func, g_num_tests, y_str, cmp_str, x_str, cmp_str, z_str, y, x, z, desc);
        if (abort) {
            exit(EXIT_FAILURE);
        }
    }
}

void (check_between_mem)(const void *x, const char *x_str, const void *y, const char *y_str, const void *z, const char *z_str, ssize_t size, bool strict, bool abort, const char *file, int line, const char *func) {
    g_num_tests++;
    
    ssize_t index;
    int cmp = mem_between_index(x, y, z, size, strict, &index);
    const char *cmp_str = strict ? "<" : "<=";
    const char *desc;
    if (cmp == 0) {
        desc = "between";
    } else if (cmp < 0) {
        desc = strict ? "leq" : "less";
    } else {
        desc = strict ? "geq" : "greater";
    }

    if (cmp == 0) {
        g_num_passed_tests++;
    } else {
        g_num_failed_tests++;
        fprintf(stderr, "[%s L%d %s()] Failed test %zd: expected %s %s %s %s %s; got \\x%02hhx, \\x%02hhx, \\x%02hhx at index %zd (%s)\n", file, line, func, g_num_tests, y_str, cmp_str, x_str, cmp_str, z_str, get_byte(y, index), get_byte(x, index), get_byte(z, index), index, desc);
        if (abort) {
            exit(EXIT_FAILURE);
        }
    }
}

ssize_t get_num_tests(void) {
    return g_num_tests;
}

ssize_t get_num_passed_tests(void) {
    return g_num_passed_tests;
}

ssize_t get_num_failed_tests(void) {
    return g_num_failed_tests;
}

void summarize_tests(void) {
    printf("Passed %zd/%zd tests\n", g_num_passed_tests, g_num_tests);
}
