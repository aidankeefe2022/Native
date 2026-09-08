#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include <internal_headers/native.h>

#include "testing.c"

int main (void)
{
    int tracker = 0;
    test('', tracker,
        check(1 == 1, "1 is not 1");
    )

    test('', tracker,
        checkAndQuit(1 == 1, "Fail");
    )

    if (tracker) {
        fprintf(stderr, "%d tests Failed\n", tracker);
        return 1;
    }
    return 0;
}
