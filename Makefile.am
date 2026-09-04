SUBDIRS = privileged_src tests testBench

# Public headers.  They are included as <external_headers/native.h>, so
# nobase_ is used to keep the external_headers/ prefix under $(includedir).
nobase_include_HEADERS = external_headers/native.h \
						 external_headers/logging.h \
						 external_headers/protectedThread.h \
						 external_headers/syscall.h

# Private headers: distributed, never installed.
noinst_HEADERS = internal_headers/buffer.h \
				 internal_headers/common.h \
				 internal_headers/logging.h \
				 internal_headers/native.h \
				 internal_headers/safeThread.h \
				 internal_headers/syscall.h

EXTRA_DIST = autogen.sh README.md certs

dist-hook:
	@echo "Packaged $(PACKAGE_STRING)"
