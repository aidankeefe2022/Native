AM_CPPFLAGS = -I$(top_srcdir) -I$(top_builddir)
AM_CFLAGS = -Wall -Wextra -fPIC $(MACRO_PREFIX_MAP_CFLAGS)

# testProg.so is a guest plugin: the daemon loads it out of a memfd and calls
# its run() symbol.  It deliberately links against nothing -- the client API
# (nat_logErrorI, nat_SafeThread_*) is left undefined here and resolved against
# the daemon's exported symbols at dlopen() time.  Automake has no plain-.so
# target without libtool, so this is built by hand.
check-local: testProg.so

testProg.so: $(srcdir)/testProg.c
	$(AM_V_CC)$(CC) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS) \
		-shared -o $@ $(srcdir)/testProg.c

clean-local:
	-rm -f testProg.so

EXTRA_DIST = testProg.c main.go go.mod
