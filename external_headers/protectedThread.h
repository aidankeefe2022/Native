
#ifndef NATIVE_PROTECTED_THREADS
#define NATIVE_PROTECTED_THREADS
#include <stdint.h>

typedef struct nat_SafeThread nat_SafeThread;

typedef int(*nat_SafeThread_Callback)(void* ctx);
/* The thread handle is allocated by the call, so it is returned through
   *safeThread rather than taken by value. */
int nat_SafeThread_CreateI(nat_SafeThread** safeThread,
        nat_SafeThread_Callback cb, void* ctx);

int nat_SafeThread_JoinI(nat_SafeThread* safeThread, int* ret);
#endif

