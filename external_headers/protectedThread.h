
#ifndef NATIVE_PROTECTED_THREADS
#define NATIVE_PROTECTED_THREADS
#include <stdint.h>
#include <threads.h>

#define thrd_create nat_SafeThread_CreateI

typedef struct nat_SafeThread nat_SafeThread;

typedef thrd_start_t nat_SafeThread_Callback;
/* The thread handle is allocated by the call, so it is returned through
   *safeThread rather than taken by value. */
int nat_SafeThread_CreateI(thrd_t* safeThread,
        nat_SafeThread_Callback cb, void* ctx);

int nat_SafeThread_JoinI(thrd_t safeThread, int* ret);
#endif

