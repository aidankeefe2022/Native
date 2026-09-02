# native

A C project built with GNU Autotools.

## Building from a checkout

```sh
./autogen.sh          # regenerate configure and Makefile.in files
./configure
make
make check            # run the test suite
sudo make install
```

## Common configure options

- `--prefix=DIR` — installation prefix (default `/usr/local`)
- `--enable-debug` — build with `-g3 -O0 -DDEBUG`

## Other targets

- `make distcheck` — build a release tarball and verify it builds/tests standalone
- `make clean` / `make distclean`

## Layout

```
configure.ac      package definition and feature checks
Makefile.am       top-level, recurses into src/ and tests/
native/           public headers, installed as <native/native.h>
src/              libnative.a (core logic) + the native binary
tests/            test programs run by `make check`
m4/               local autoconf macros
build-aux/        auxiliary scripts installed by autoreconf
```
