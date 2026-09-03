#include <threads.h>
#include <stdlib.h>
#include <native/common.h>
#include <seccomp.h>
#include <external_headers/protectedThread.h>


struct nat_SafeThread {
    thrd_t thread;
};

struct CallbackWrapper_CTX {
    nat_SafeThread_Callback callback;
    void* arg;
};

static i32 CallbackWrapper(void* ctx) {
    struct CallbackWrapper_CTX* wrapperCtx = ctx;

    // 1. Lock down this child process entirely
    scmp_filter_ctx scmp_ctx = seccomp_init(SCMP_ACT_KILL);
    seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(exit_group), 0);
    seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(brk), 0);
    seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(mmap), 0);
    seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(munmap), 0);
    seccomp_load(scmp_ctx);
    seccomp_release(scmp_ctx);

    return wrapperCtx->callback(wrapperCtx->arg);
}


int32_t native_threadCreate(nat_SafeThread* thread, nat_SafeThread_Callback callback,
        void* arg) {
    thread = malloc(sizeof(*thread));

    struct CallbackWrapper_CTX ctx = {.callback = callback, .arg = arg};

    thrd_create(&thread->thread, CallbackWrapper, &ctx);

    return 0;
}
int32_t native_threadJoin(nat_SafeThread* thread, int* ret) {
    if (thread == NULL) {
        return nat_MemError;
    }

    thrd_join(thread->thread, ret);

    return 0;

}
