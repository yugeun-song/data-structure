#ifndef TEST_H
#define TEST_H

#include <stddef.h>
#include <stdio.h>

static int g_test_cases_total = 0;
static int g_test_cases_failed = 0;
static int g_test_case_failures = 0;

static inline int test_summary(void)
{
    printf("\n%d/%d cases passed\n",
           g_test_cases_total - g_test_cases_failed, g_test_cases_total);
    return g_test_cases_failed == 0 ? 0 : 1;
}

#define TEST_RUN(case_fn)                                                    \
    do {                                                                     \
        g_test_case_failures = 0;                                            \
        case_fn();                                                           \
        g_test_cases_total++;                                                \
        if (g_test_case_failures > 0) {                                      \
            g_test_cases_failed++;                                           \
            printf("[ FAIL ] %s (%d failure(s))\n",                          \
                   #case_fn, g_test_case_failures);                          \
        } else {                                                             \
            printf("[ PASS ] %s\n", #case_fn);                               \
        }                                                                    \
    } while (0)

#define TEST_ASSERT(expr)                                                    \
    do {                                                                     \
        if (!(expr)) {                                                       \
            printf("  ASSERT FAIL: %s:%d: %s\n", __FILE__, __LINE__, #expr); \
            g_test_case_failures++;                                          \
        }                                                                    \
    } while (0)

#define TEST_ASSERT_EQ(actual, expected)                                     \
    do {                                                                     \
        long long _ta = (long long)(actual);                                 \
        long long _te = (long long)(expected);                               \
        if (_ta != _te) {                                                    \
            printf("  ASSERT_EQ FAIL: %s:%d: %s == %s "                      \
                   "(got %lld, expected %lld)\n",                            \
                   __FILE__, __LINE__, #actual, #expected, _ta, _te);        \
            g_test_case_failures++;                                          \
        }                                                                    \
    } while (0)

#define TEST_ASSERT_NULL(ptr)                                                \
    do {                                                                     \
        if ((ptr) != NULL) {                                                 \
            printf("  ASSERT_NULL FAIL: %s:%d: %s != NULL\n",                \
                   __FILE__, __LINE__, #ptr);                                \
            g_test_case_failures++;                                          \
        }                                                                    \
    } while (0)

#define TEST_ASSERT_NOT_NULL(ptr)                                            \
    do {                                                                     \
        if ((ptr) == NULL) {                                                 \
            printf("  ASSERT_NOT_NULL FAIL: %s:%d: %s == NULL\n",            \
                   __FILE__, __LINE__, #ptr);                                \
            g_test_case_failures++;                                          \
        }                                                                    \
    } while (0)

#define TEST_SUMMARY() test_summary()

#endif