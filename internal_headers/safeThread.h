#ifndef INTERNAL_NATIVE_SAFETHREAD_H
#define INTERNAL_NATIVE_SAFETHREAD_H
#include <internal_headers/common.h>
#include <threads.h>
#include <stdatomic.h>

typedef struct nat_SafeThread {
    thrd_t thread;
    atomic_bool isRunning;
}nat_SafeThread;

struct nat_SafeThread_CreateArg {
    nat_SafeThread* thread;  /* out: receives the allocated handle */
    thrd_start_t callback;
    void* ctx;
};
typedef i32(nat_threadCreate_t)(struct nat_SafeThread_CreateArg* arg);
nat_threadCreate_t nat_threadCreate;

struct nat_SafeThread_JoinArg {
    nat_SafeThread thread;
    int* ret;
};
typedef i32(nat_threadJoin_t)(struct nat_SafeThread_JoinArg* arg);
nat_threadJoin_t nat_threadJoin;
#endif /* INTERNAL_NATIVE_SAFETHREAD_H */
