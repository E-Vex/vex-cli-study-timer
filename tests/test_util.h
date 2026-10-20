#ifndef TEST_UTIL_H
#define TEST_UTIL_H

#include <stdio.h>
#include <string.h>

extern int test_failures;

#define CHECK(condition)                                                         \
    do {                                                                         \
        if (!(condition))                                                        \
        {                                                                        \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
            test_failures++;                                                     \
        }                                                                        \
    } while (0)

#define CHECK_STRING(actual, expected)                                           \
    do {                                                                         \
        const char *check_actual = (actual);                                     \
        const char *check_expected = (expected);                                 \
        if (check_actual == NULL || check_expected == NULL ||                    \
            strcmp(check_actual, check_expected) != 0)                           \
        {                                                                        \
            fprintf(stderr, "FAIL %s:%d: \"%s\" != \"%s\"\n", __FILE__, __LINE__, \
                    check_actual != NULL ? check_actual : "(null)",              \
                    check_expected != NULL ? check_expected : "(null)");         \
            test_failures++;                                                     \
        }                                                                        \
    } while (0)

#endif
