#include <internal_headers/syscall.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

/* The single definitions behind the extern declarations in
 * <internal_headers/syscall.h>.  Both the client side (this file) and the
 * privileged side (runSyscall.c) operate on this one queue. */
mtx_t nat_SyscallMutex;
struct nat_SyscallQueue* nat_SyscallQueue;

nat_Syscall_Request* nat_pushToSyscallQueue(void* funcPtr,
        void* arg)
{
    nat_Syscall_Request* request = malloc(sizeof(*request));
    if (request == NULL) {
        return NULL;
    }
    request->isReady   = 0;
    request->returnVal = 0;
    request->arg = arg;
    request->funcPtr = funcPtr;
    mtx_lock(&nat_SyscallMutex);
    // TODO: broken will not work after nextFree wraps
    if (nat_SyscallQueue->len == nat_SyscallQueue->cap) {
        mtx_unlock(&nat_SyscallMutex);
        free(request);
        return NULL;
        /*
        struct nat_SyscallQueue* tmp = realloc(&nat_SyscallQueue,
                (nat_SyscallQueue->cap * 2) *
                sizeof(*nat_SyscallQueue->requestQueue) +
                sizeof(*nat_SyscallQueue));
        if (tmp == NULL) {
            free(request);
            return NULL;
        }
        else {
            nat_SyscallQueue = tmp;
            nat_SyscallQueue->cap *= 2;
        }
        */
    }
    nat_SyscallQueue->requestQueue[nat_SyscallQueue->nextFree++] = request;
    nat_SyscallQueue->len++;
    if (nat_SyscallQueue->nextFree >= nat_SyscallQueue->cap) {
        nat_SyscallQueue->nextFree = 0;
    }
    mtx_unlock(&nat_SyscallMutex);
    return request;
}

i32 nat_checkSyscallRequest(nat_Syscall_Request* request)
{
    return request->isReady == 1;
}

void nat_waitSyscallRequest(nat_Syscall_Request* request)
{
    while (!nat_checkSyscallRequest(request)) {
        __asm__ __volatile__("pause");
    }
}
