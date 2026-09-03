
#ifndef NATIVE_PROTECTED_THREADS
#define NATIVE_PROTECTED_THREADS
#include <stdint.h>

typedef struct nat_SafeThread nat_SafeThread;

typedef int(*nat_SafeThread_Callback)(void* ctx);

int32_t native_threadCreate(nat_SafeThread* thread,
        nat_SafeThread_Callback callback, void* ctx);
int32_t native_threadJoin(nat_SafeThread* thread, int* ret);

#endif

