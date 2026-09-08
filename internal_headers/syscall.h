#ifndef INTERNAL_NATIVE_SYSCALL_H
#define INTERNAL_NATIVE_SYSCALL_H
#include <internal_headers/common.h>
#include <internal_headers/native.h>
#include <internal_headers/safeThread.h>
#include <external_headers/protectedThread.h>
#include <external_headers/logging.h>
#include <internal_headers/logging.h>
#include <external_headers/syscall.h>
#include <stdatomic.h>
#include <stdarg.h>
#include <threads.h>

typedef struct nat_Syscall_Request nat_Syscall_Request;

extern mtx_t nat_SyscallMutex;

struct nat_Syscall_Request {
    enum nat_Syscall_Type func_type;
    atomic_bool isReady;
    atomic_int returnVal;
    union {
        /* threadJoin Syscall */
        struct {
            typeof(nat_threadCreate)* threadCreate;
            struct nat_SafeThread_CreateArg* arg;
        }threadCreate;
        /* logError Syscall */
        struct {
            typeof(nat_logError)* logError;
            struct nat_logErrorArg* arg;
        }logError;
    };
};

struct nat_SyscallQueue {
    u64 len;
    u64 cap;
    u64 nextFree;
    u64 nextUp;
    nat_Syscall_Request* requestQueue[];
};

extern struct nat_SyscallQueue* nat_SyscallQueue;
i32 nat_runSyscallRequest(nat_Syscall_Request* request);
nat_Syscall_Request* nat_getNextSyscall();
#endif
