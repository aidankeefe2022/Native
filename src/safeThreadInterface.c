#include <internal_headers/native.h>
#include <internal_headers/safeThread.h>
#include <internal_headers/syscall.h>
#include <external_headers/protectedThread.h>
#include <stdlib.h>


i32 nat_SafeThread_CreateI(nat_SafeThread** safeThread,
        nat_SafeThread_Callback cb, void* ctx)
{
    struct nat_SafeThread_CreateArg arg = {
        .thread = safeThread,
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
    return 0;
}

i32 nat_SafeThread_JoinI(nat_SafeThread* safeThread, int* ret)
{
    struct nat_SafeThread_JoinArg arg = {
        .thread = safeThread,
        .ret = ret,
    };
    // The privileged side reports nat_WouldBlock rather than parking the
    // dispatch loop in a blocking join, so poll until the thread has exited.
    // Each request is freed here: the servicer is done with it once isReady
    // is set, and this loop would otherwise leak one per attempt.
    i32 ret_code;
    nat_Syscall_Request* req = nat_pushToSyscallQueue(nat_threadJoin_e, &arg);
    if (req == NULL) {
        return nat_MemError;
    }
    nat_waitSyscallRequest(req);
    ret_code = atomic_load(&req->returnVal);
    free(req);

    return ret_code;
}
