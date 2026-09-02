#include <stdio.h>
#include <stdlib.h>

#define Defer(cleanup, logic) \
for (int DeferIntNumber = 0; DeferIntNumber < 1; cleanup, DeferIntNumber++) { \
    do { logic } while (0); \
}\

#define IfFail() (ret == 1)

#define check(expr, msg) \
if (!IfFail()) { \
    if (expr) { \
        fprintf(stderr, msg"\n"); \
        ret = 1; \
    } \
} \


#define checkAndQuit(expr, msg) \
if (!IfFail()) { \
    if (expr) { \
        fprintf(stderr, "Check at in %s, line %d Failed\n", __FILE__, __LINE__); \
        fprintf(stderr, msg"\n"); \
    } \
    else { \
        ret = 1; \
        break; \
    } \
} \

#define test(name, tracker, inner) do { \
    int line = __LINE__ - 1; \
    int ret = 0; \
    fprintf(stderr, "Test %s line %d: \n", #name, line); \
    do {inner} while (0); \
    if (ret == 1) { \
        fprintf(stderr, "Failed\n"); \
        tracker++; \
    } else {\
        fprintf(stderr, "Passed\n"); \
    } \
}while (0);
