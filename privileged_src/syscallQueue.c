#include <internal_headers/syscall.h>
#include <stdlib.h>

nat_Syscall_Request* nat_pushToSyscallQueue(enum nat_Syscall_Type type, void* arg)
{
    nat_Syscall_Request* request = malloc(sizeof(*request));
    if (request == NULL) {
        return NULL;
    }
    request->func_type = type;
    switch (type) {
        case nat_logError_e:
            request->logError.logError = nat_logError;
            request->logError.arg = *(typeof(request->logError.arg)*)arg;
            break;

        case nat_threadCreate_e:
            request->threadCreate.threadCreate = native_threadCreate;
            request->threadCreate.arg = *(typeof(request->threadCreate.arg)*)arg;
            break;

        case nat_threadJoin_e:
            request->threadJoin.threadJoin = native_threadJoin;
            request->threadJoin.arg = *(typeof(request->threadJoin.arg)*)arg;
            break;

        default:
            free(request);
            return NULL;
    }
    mtx_lock(&nat_SyscallMutex);
    if (nat_SyscallQueue->len == nat_SyscallQueue->cap) {
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
    }
    nat_SyscallQueue->requestQueue[nat_SyscallQueue->currentIndex++] = request;
    nat_SyscallQueue->len++;
    if (nat_SyscallQueue->currentIndex >= nat_SyscallQueue->cap) {
        nat_SyscallQueue->currentIndex = 0;
    }
    mtx_unlock(&nat_SyscallMutex);
    return request;
}


struct nat_Syscall_Return {

};

i32 nat_checkSyscallRequest(nat_Syscall_Request* request)
{

}
