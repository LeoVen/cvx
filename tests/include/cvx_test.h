#ifndef CVX_TEST_H
#define CVX_TEST_H

#include <stdio.h>

/* ---- state ---- */

struct cvxtest
{
    int total;
    int passed;
    int failed;
    int empty;
    int total_checks;
    int checks_failed;
    const char *current;
};

struct cvxresult
{
    int passed;
    int failed;
    int empty;
};

/* ---- macros ---- */

/* Record a failed check without aborting. */
#define CVXCHECK(t, cond) \
    do \
    { \
        (t)->total_checks++; \
        if (!(cond)) \
        { \
            fprintf(stderr, "    FAIL  %s:%d  (%s)\n", __FILE__, __LINE__, #cond); \
            (t)->checks_failed++; \
        } \
    } while (0)

/* Run a test function, print PASS / FAIL, update counters. */
#define CVXRUN(t, fn) \
    do \
    { \
        (t)->current = #fn; \
        (t)->total_checks = 0; \
        (t)->checks_failed = 0; \
        (t)->total++; \
        fn(t); \
        if ((t)->total_checks == 0) \
        { \
            printf("    EMPTY %s\n", #fn); \
            (t)->empty++; \
        } \
        else if ((t)->checks_failed == 0) \
        { \
            printf("    PASS  %s\n", #fn); \
            (t)->passed++; \
        } \
        else \
        { \
            printf("    FAIL  %s  (%d check(s) failed)\n", #fn, (t)->checks_failed); \
            (t)->failed++; \
        } \
    } while (0)

/* Print a summary line and return a cvxresult with passed and failed counts. */
#define CVXSUMMARY(t) \
    (printf("  %d/%d passed", (t)->passed, (t)->total), \
     (t)->empty ? printf(", %d empty", (t)->empty) : 0, \
     (t)->failed ? printf(", %d failed", (t)->failed) : 0, printf("\n"), \
     (struct cvxresult){ .passed = (t)->passed, .failed = (t)->failed, .empty = (t)->empty })

#define CVX_EVAL(...) CVX_EVAL4(CVX_EVAL4(CVX_EVAL4(CVX_EVAL4(__VA_ARGS__))))
#define CVX_EVAL4(...) __VA_ARGS__
#define CVX_PARENS ()

#define CVX_FOR_EACH(t, ...) __VA_OPT__(CVX_EVAL(CVX_FOR_EACH_HELPER(t, __VA_ARGS__)))

#define CVX_FOR_EACH_HELPER(t, current, ...) \
    CVXRUN(&(t), current); \
    __VA_OPT__(CVX_FOR_EACH_AGAIN CVX_PARENS(t, __VA_ARGS__))

#define CVX_FOR_EACH_AGAIN() CVX_FOR_EACH_HELPER

#define CVXSUITE(name, ...) \
    struct cvxresult name(void) \
    { \
        struct cvxtest t = { 0 }; \
        printf("  %s:\n", #name); \
        CVX_FOR_EACH(t, __VA_ARGS__) \
        return CVXSUMMARY(&t); \
    }

#define CVXMAIN(name, ...) \
    int main(void) \
    { \
        printf("\nTests for %s\n", #name); \
        struct cvxresult (*tests[])(void) = { \
            __VA_ARGS__, \
        }; \
        size_t count = sizeof(tests) / sizeof(tests[0]); \
        int total_passed = 0; \
        int total_failed = 0; \
        int total_empty = 0; \
        for (size_t i = 0; i < count; i++) \
        { \
            struct cvxresult r = tests[i](); \
            total_passed += r.passed; \
            total_failed += r.failed; \
            total_empty += r.empty; \
        } \
        printf("Total: %d passed, %d empty, %d failed\n", total_passed, total_empty, \
               total_failed); \
        return total_failed != 0; \
    }

#endif /* CVX_TEST_H */
