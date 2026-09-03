SUBDIRS = src tests

# Public headers live in native/ and are included as <native/native.h>.
# nobase_ keeps the native/ prefix under $(includedir).
nobase_include_HEADERS = native/native.h \
						 native/common.h \
						 external_headers/logging.h

EXTRA_DIST = autogen.sh README.md

dist-hook:
	@echo "Packaged $(PACKAGE_STRING)"
