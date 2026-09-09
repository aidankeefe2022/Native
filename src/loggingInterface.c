#include <external_headers/logging.h>
#include <internal_headers/logging.h>
#include <external_headers/syscall.h>
#include <stddef.h>

int nat_logErrorI(const char* fmt, ...) {
    struct nat_logErrorArg arg = {
        .fmt = fmt,
    };
    va_start(arg.list, fmt);

    // arg lives on this stack, and arg.list points into this frame's vararg
    // area, so both are only valid until this call returns: block until the
    // privileged thread has finished consuming them.
    auto req = nat_pushToSyscallQueue(nat_logError, &arg);
    if (req == NULL) {
        va_end(arg.list);
        return nat_MemError;
    }
    nat_waitSyscallRequest(req);

    // va_end has to happen in the frame that called va_start, not on the
    // privileged side.
    va_end(arg.list);
    return 0;
}


