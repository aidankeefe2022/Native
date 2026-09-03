
enum nat_Syscall_Type {
    nat_threadCreate_e,
    nat_threadJoin_e,
    nat_logError_e,
}func_type;

typedef struct nat_Syscall_Request nat_Syscall_Request;
nat_Syscall_Request* nat_pushToSyscallQueue(enum nat_Syscall_Type type, void* arg);
int nat_checkSyscallRequest(nat_Syscall_Request* request);
