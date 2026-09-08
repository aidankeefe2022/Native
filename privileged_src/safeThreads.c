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
#include <internal_headers/safeThread.h>

/* FUTEX_CMD_MASK clears FUTEX_PRIVATE_FLAG (bit 7), so masking an op with it
 * and comparing the result against a *_PRIVATE command is unsatisfiable: the
 * rule can never fire.  Mask off only FUTEX_CLOCK_REALTIME instead, which
 * keeps the private bit in the compared value. */
#define FUTEX_CMD_PRIV_MASK ((uint32_t)~FUTEX_CLOCK_REALTIME)

struct CallbackWrapper_CTX {
    thrd_start_t callback;
    void* arg;
};

static i32 CallbackWrapper(void* ctx) {
    struct CallbackWrapper_CTX* wrapperCtx = ctx;

    // nat_threadCreate's frame is already gone by the time this runs, so the
    // context is heap allocated: unpack it and hand the memory back here.
    thrd_start_t callback = wrapperCtx->callback;
    void* arg = wrapperCtx->arg;
    free(wrapperCtx);

    // Default-deny.  Anything not matched below fails with EPERM rather than
    // being allowed through.
    scmp_filter_ctx scmp_ctx = seccomp_init(SCMP_ACT_ERRNO(EPERM));

    // Keep the audit trail that the permissive SCMP_ACT_LOG default used to
    // provide: log every non-allow action, so a refused syscall still shows up
    // as a type=1326 record instead of failing silently.
    seccomp_attr_set(scmp_ctx, SCMP_FLTATR_CTL_LOG, 1);

    // A thread returning from its entry point exits with exit(2).  exit_group(2)
    // would tear down the whole daemon, so it is refused outright rather than
    // left to the default action.
    seccomp_rule_add(scmp_ctx, SCMP_ACT_ERRNO(EPERM), SCMP_SYS(exit_group), 0);
    seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(exit), 0);

    seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(brk), 0);

    // An unrestricted mmap makes the mprotect rule below unenforceable: a fresh
    // PROT_EXEC anonymous mapping, or MAP_FIXED laid over an existing one, both
    // reintroduce executable memory the mprotect rule is meant to deny.
    // Argument 2 is prot, argument 3 is flags.
    seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(mmap), 2,
                     SCMP_A2(SCMP_CMP_MASKED_EQ, PROT_EXEC, 0),
                     SCMP_A3(SCMP_CMP_MASKED_EQ, MAP_FIXED, 0));
    seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(munmap), 0);

    // Allow basic private wait (Argument 1 is the 'op' field).  Admits ops
    // 128 and 384 (with FUTEX_CLOCK_REALTIME); the requeue and
    // priority-inheritance families stay out.
    seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(futex), 1,
                     SCMP_CMP(1, SCMP_CMP_MASKED_EQ, FUTEX_CMD_PRIV_MASK, FUTEX_WAIT_PRIVATE));

    // Allow basic private wake
    seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(futex), 1,
                     SCMP_CMP(1, SCMP_CMP_MASKED_EQ, FUTEX_CMD_PRIV_MASK, FUTEX_WAKE_PRIVATE));

    // Repeat the strict filtering for modern 64-bit time variations if necessary
    seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(futex_time64), 1,
                     SCMP_CMP(1, SCMP_CMP_MASKED_EQ, FUTEX_CMD_PRIV_MASK, FUTEX_WAIT_PRIVATE));
    seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(futex_time64), 1,
                     SCMP_CMP(1, SCMP_CMP_MASKED_EQ, FUTEX_CMD_PRIV_MASK, FUTEX_WAKE_PRIVATE));

    seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(mprotect), 1,
                 SCMP_A2(SCMP_CMP_MASKED_EQ, PROT_EXEC, 0));

    // pthread_join waits on the joined thread's tid with FUTEX_WAIT_BITSET and
    // FUTEX_CLOCK_REALTIME, and glibc uses the *shared* variant for it, so this
    // one is matched on the command alone (FUTEX_CMD_MASK strips the private
    // and clock bits).  Admits ops 9/137/265/393 -- all waits; the requeue and
    // priority-inheritance families are still excluded.
    seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(futex), 1,
                     SCMP_CMP(1, SCMP_CMP_MASKED_EQ, FUTEX_CMD_MASK, FUTEX_WAIT_BITSET));
    seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(futex_time64), 1,
                     SCMP_CMP(1, SCMP_CMP_MASKED_EQ, FUTEX_CMD_MASK, FUTEX_WAIT_BITSET));

    // glibc's thread teardown returns stack and malloc'd pages with
    // MADV_DONTNEED/MADV_FREE.  Argument 2 is the advice: restricted rather
    // than allowed bare, since MADV_DONTNEED destroys page contents and has a
    // long history as a race primitive (CVE-2016-5195 and successors).
    seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(madvise), 1,
                     SCMP_A2(SCMP_CMP_EQ, MADV_DONTNEED));
    seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(madvise), 1,
                     SCMP_A2(SCMP_CMP_EQ, MADV_FREE));

    // Alters only the calling thread's signal mask; it cannot install handlers
    // (that is rt_sigaction, which is process-wide and stays denied).
    seccomp_rule_add(scmp_ctx, SCMP_ACT_ALLOW, SCMP_SYS(rt_sigprocmask), 0);

    seccomp_load(scmp_ctx);
    seccomp_release(scmp_ctx);

    return callback(arg);
}


int32_t nat_threadCreate(struct nat_SafeThread_CreateArg *arg) {
    if (arg->thread == NULL) {
        return nat_MemError;
    }

    struct CallbackWrapper_CTX* ctx = malloc(sizeof(*ctx));
    if (ctx == NULL) {
        return nat_MemError;
    }
    *ctx = (struct CallbackWrapper_CTX){.callback = arg->callback, .arg = arg->ctx};

    if (thrd_create(&arg->thread->thread, CallbackWrapper, ctx) != thrd_success) {
        free(ctx);
        return nat_MemError;
    }
    arg->thread->isRunning = 1;

    return 0;
}

int32_t nat_threadJoin(struct nat_SafeThread_JoinArg* arg) {
    if (arg == NULL) {
        return nat_MemError;
    }

    thrd_join(arg->thread.thread, arg->ret);

    return 0;
}
