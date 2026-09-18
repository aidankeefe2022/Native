#include <internal_headers/native.h>
#include <internal_headers/safeThread.h>
#include <internal_headers/syscall.h>
#include <external_headers/protectedThread.h>
#include <stdlib.h>
#include <stdio.h>

static nat_Syscall_arg* makeArg(thrd_t* thread, thrd_start_t cb, void* ctx) {
    nat_SafeThread* sf = malloc(sizeof(*sf));
    *sf = (nat_SafeThread) {
        .thread = thread,
    };
    struct nat_SafeThread_CreateArg* innerArg = malloc(sizeof(*innerArg));
    *innerArg = (struct nat_SafeThread_CreateArg) {
        .thread = sf,
        .callback = cb,
        .ctx = ctx,
    };
    nat_Syscall_arg* arg = malloc(sizeof(*arg));
    arg->arg = innerArg;
    arg->cleanup = NULL;
    return arg;
}

i32 nat_SafeThread_CreateI(thrd_t* safeThread,
        thrd_start_t cb, void* ctx)
{
    auto arg = makeArg(safeThread, cb, ctx);
    // arg lives on this stack, and the privileged thread dereferences it when
    // it pops the request, so this has to block until the syscall has run.
    auto req = nat_pushToSyscallQueue(&nat_threadCreate, arg);
    if (req == NULL) {
        return nat_MemError;
    }
    nat_waitSyscallRequest(req);
    free(req);
    return 0;
}
