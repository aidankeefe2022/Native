#include <internal_headers/native.h>
#include <internal_headers/safeThread.h>
#include <internal_headers/syscall.h>
#include <external_headers/protectedThread.h>
#include <stdlib.h>
#include <stdio.h>


i32 nat_SafeThread_CreateI(thrd_t* safeThread,
        thrd_start_t cb, void* ctx)
{
    struct nat_SafeThread_CreateArg arg = {
        .thread = &(nat_SafeThread){.thread = *safeThread},
        .callback = cb,
        .ctx = ctx,
    };
    // arg lives on this stack, and the privileged thread dereferences it when
    // it pops the request, so this has to block until the syscall has run.
    auto req = nat_pushToSyscallQueue(nat_threadCreate_e, &arg);
    if (req == NULL) {
        return nat_MemError;
    }
    nat_waitSyscallRequest(req);
    free(req);
    *safeThread = arg.thread->thread;
    return 0;
}
