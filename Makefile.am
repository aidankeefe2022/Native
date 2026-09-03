SUBDIRS = privileged_src tests

# Public headers live in native/ and are included as <native/native.h>.
# nobase_ keeps the native/ prefix under $(includedir).
nobase_include_HEADERS = internal_headers/native.h \
						 internal_headers/common.h

EXTRA_DIST = autogen.sh README.md

dist-hook:
	@echo "Packaged $(PACKAGE_STRING)"
