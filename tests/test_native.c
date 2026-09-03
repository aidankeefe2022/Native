#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>

#include <internal_headers/native.h>

#include "testing.c"

int main (void)
{
    int tracker = 0;
    test('Check If 1 is 1', tracker,
        check(1 == 1, "1 is not 1");
    )

    test('Check Fail', tracker,
        checkAndQuit(1 == 1, "Fail");
    )

    if (tracker) {
        fprintf(stderr, "%d tests Failed\n", tracker);
        return 1;
    }
    return 0;
}
