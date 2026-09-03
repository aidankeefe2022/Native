#ifndef INTERNAL_NATIVE_SYSCALL_H
#define INTERNAL_NATIVE_SYSCALL_H
#include <internal_headers/common.h>
#include <internal_headers/native.h>
#include <external_headers/protectedThread.h>
#include <external_headers/logging.h>
#include <external_headers/syscall.h>
#include <stdio.h>
#include <stdarg.h>
#include <threads.h>

typedef struct nat_Syscall_Request nat_Syscall_Request;

enum nat_Syscall_Type {
    nat_threadCreate_e,
    nat_threadJoin_e,
    nat_logError_e,
};

mtx_t nat_SyscallMutex;

struct nat_Syscall_Request {
    enum nat_Syscall_Type func_type;
    union {
        /* threadJoin Syscall */
        struct {
            typeof(native_threadCreate)* threadCreate;
            typeof(native_threadCreate(0, NULL, NULL)) threadCreateRet;
            struct {
                nat_SafeThread* thread;
                nat_SafeThread_Callback cb;
                void* ctx;
            } arg;
        }threadCreate;
        /* threadJoin Syscall */
        struct {
            typeof(native_threadJoin)* threadJoin;
            typeof(native_threadJoin(0, NULL)) threadJoinRet;
            struct {
                nat_SafeThread* thread;
                int* ret;
            } arg;
        }threadJoin;
        /* logError Syscall */
        struct {
            typeof(nat_logError)* logError;
            typeof(nat_logError(NULL, NULL, NULL)) logErrorRet;
            struct {
                FILE* stream;
                const char* fmt;
                va_list list;
            } arg;
        }logError;
    };
};

struct nat_SyscallQueue {
    u64 len;
    u64 cap;
    u64 currentIndex;
    nat_Syscall_Request* requestQueue[];
};

static struct nat_SyscallQueue* nat_SyscallQueue;

nat_Syscall_Request* nat_pushToSyscallQueue(enum nat_Syscall_Type type, void* arg);
int nat_checkSyscallRequest(nat_Syscall_Request* request);


#endif
