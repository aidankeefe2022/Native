#define _GNU_SOURCE  // pthread_tryjoin_np
#include <threads.h>
#include <pthread.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <internal_headers/common.h>
#include <seccomp.h>
#include <sys/mman.h>
#include <linux/futex.h>
#include <external_headers/protectedThread.h>
#include <internal_headers/safeThread.h>


struct nat_SafeThread {
    thrd_t thread;
};

struct CallbackWrapper_CTX {
    nat_SafeThread_Callback callback;
    void* arg;
};

static i32 CallbackWrapper(void* ctx) {
    struct CallbackWrapper_CTX* wrapperCtx = ctx;

    // nat_threadCreate's frame is already gone by the time this runs, so the
    // context is heap allocated: unpack it and hand the memory back here.
    nat_SafeThread_Callback callback = wrapperCtx->callback;
    void* arg = wrapperCtx->arg;
    free(wrapperCtx);

    scmp_filter_ctx scmp_ctx = seccomp_init(SCMP_ACT_KILL);
    seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(exit_group), 0);
    seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(brk), 0);
    seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(mmap), 0);
    seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(munmap), 0);
    // Allow basic private wait (Argument 1 is the 'op' field)
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(futex), 1,
                     SCMP_CMP(1, SCMP_CMP_MASKED_EQ, FUTEX_CMD_MASK, FUTEX_WAIT_PRIVATE));

    // Allow basic private wake
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(futex), 1,
                     SCMP_CMP(1, SCMP_CMP_MASKED_EQ, FUTEX_CMD_MASK, FUTEX_WAKE_PRIVATE));

    // Repeat the strict filtering for modern 64-bit time variations if necessary
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(futex_time64), 1,
                     SCMP_CMP(1, SCMP_CMP_MASKED_EQ, FUTEX_CMD_MASK, FUTEX_WAIT_PRIVATE));
    seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(mprotect), 1,
                 SCMP_A2(SCMP_CMP_MASKED_EQ, PROT_EXEC, 0));

    seccomp_load(scmp_ctx);
    seccomp_release(scmp_ctx);

    return callback(arg);
}


int32_t nat_threadCreate(struct nat_SafeThread_CreateArg *arg) {
    if (arg->thread == NULL) {
        return nat_MemError;
    }
    *(arg->thread) = NULL;

    // Allocated here and handed back to the caller through arg->thread.
    nat_SafeThread* thread = malloc(sizeof(*thread));
    if (thread == NULL) {
        return nat_MemError;
    }

    struct CallbackWrapper_CTX* ctx = malloc(sizeof(*ctx));
    if (ctx == NULL) {
        free(thread);
        return nat_MemError;
    }
    *ctx = (struct CallbackWrapper_CTX){.callback = arg->callback, .arg = arg->ctx};

    if (thrd_create(&(thread->thread), CallbackWrapper, ctx) != thrd_success) {
        free(ctx);
        free(thread);
        return nat_MemError;
    }

    *(arg->thread) = thread;
    return 0;
}

int32_t nat_threadJoin(struct nat_SafeThread_JoinArg* arg) {
    if (arg->thread == NULL) {
        return nat_MemError;
    }

    // A blocking join here would park the single dispatch thread, so a thread
    // that issues a syscall on its way out could never be serviced.  Report
    // nat_WouldBlock instead and let the client re-issue the request.
    void* threadRet;
    int rc = pthread_tryjoin_np(arg->thread->thread, &threadRet);
    if (rc == EBUSY) {
        return nat_WouldBlock;
    }
    if (rc != 0) {
        return nat_MemError;
    }

    // Mirrors how glibc's thrd_join narrows a pthread return value.
    if (arg->ret != NULL) {
        *(arg->ret) = (int)(uintptr_t)threadRet;
    }

    return 0;
}
