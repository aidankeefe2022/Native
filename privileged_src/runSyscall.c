#include <internal_headers/syscall.h>
#include <internal_headers/common.h>
#include <stdatomic.h>

nat_Syscall_Request* nat_getNextSyscall() {
    auto ret = nat_SyscallQueue->requestQueue[nat_SyscallQueue->nextUp];
    nat_SyscallQueue->len--;
    nat_SyscallQueue->nextUp++;
    if (nat_SyscallQueue->nextUp >= nat_SyscallQueue->cap) {
        nat_SyscallQueue->nextUp = 0;
    }
    return ret;
}

enum nat_syscallTypes {
    threadCreate,
    logError,
};

static const struct {
    void* funcPtr;
    enum nat_syscallTypes type;
} requestTypes[] = {
    {nat_threadCreate, threadCreate},
    {nat_logError, logError},
};

i32 nat_runSyscallRequest(nat_Syscall_Request* request) {
    for (u32 i = 0; i < sizeof(requestTypes) / sizeof(*requestTypes); i++) {
        if (request->funcPtr == requestTypes[i].funcPtr) {
            switch (requestTypes[i].type) {
                case threadCreate: {
                    nat_threadCreate_t* func_ptr = request->funcPtr;
                    atomic_store(&request->returnVal, func_ptr(request->arg));
                    request->isReady = 1;
                    return 0;
                }
                case logError: {
                    nat_logError_t* func_ptr = request->funcPtr;
                    atomic_store(&request->returnVal, func_ptr(request->arg));
                    request->isReady = 1;
                    return 0;
                }
                default:
                    /* should never get here */
                    return nat_BadFuncError;
            };
        }
    }

    return nat_BadFuncError;
}
