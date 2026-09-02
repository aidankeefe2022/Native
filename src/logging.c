#include <native/logging.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

typedef struct Native_Log {
    Native_Buffer msg;
    struct Native_Log* next;
}Native_Log;

struct Native_LogQueue {
    Native_Log* head;
    Native_Log* tail;
};

static Native_LogQueue native_LogQueue = {0};


i32 native_addLog(const char* fmt, ...) {
    va_list list;
    va_start(list, fmt);
    u8* msg = malloc(500);
    int len = vsnprintf((char*)msg, 500, fmt, list);
    Native_Log* log = malloc(sizeof(*log));
    log->msg.buf = msg;
    log->msg.cap = 500;
    log->msg.len = len;
    log->next = NULL;



    if (native_LogQueue.head == NULL && native_LogQueue.tail == NULL) {
        native_LogQueue.head = log;
        native_LogQueue.tail = log;
    }
    else {
        assert(native_LogQueue.head);
        assert(native_LogQueue.tail);
        native_LogQueue.tail->next = log;
    }

    return EXIT_SUCCESS;
}
