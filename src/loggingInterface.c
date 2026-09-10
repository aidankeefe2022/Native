#include <external_headers/logging.h>
#include <internal_headers/logging.h>
#include <external_headers/syscall.h>
#include <stddef.h>
#include <stdlib.h>

nat_Syscall_arg_cleanup cleanup;

int cleanup(nat_Syscall_arg* arg) {
    struct nat_logErrorArg* logErrorArg = arg->arg;
    va_end(logErrorArg->list);
    free(logErrorArg);
    free(arg);
    return 0;
}

int nat_logErrorI(const char* fmt, ...) {
    struct nat_logErrorArg *innerArg = malloc(sizeof(*innerArg));
    *innerArg = (struct nat_logErrorArg){
        .fmt = fmt,
    };
    va_start(innerArg->list, fmt);
    nat_Syscall_arg* arg = malloc(sizeof(*arg));
    *arg = (nat_Syscall_arg) {
        .arg = innerArg,
        .cleanup = cleanup,
    };

    // arg lives on this stack, and arg.list points into this frame's vararg
    // area, so both are only valid until this call returns: block until the
    // privileged thread has finished consuming them.
    auto req = nat_pushToSyscallQueue(nat_logError, arg);
    if (req == NULL) {
        return nat_MemError;
    }
    return 0;
}


