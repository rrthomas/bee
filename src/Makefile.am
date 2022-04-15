# Source Makefile.am
#
# (c) Reuben Thomas 2011-2022
#
# The package is distributed under the GNU General Public License version 3,
# or, at your option, any later version.
#
# THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
# RISK.

AM_CPPFLAGS = -I$(top_builddir)/lib -I$(top_srcdir)/lib -I$(srcdir)/include -I$(builddir)/include $(WARN_CFLAGS)

lib_LTLIBRARIES = libbee@PACKAGE_SUFFIX@.la
libbee@PACKAGE_SUFFIX@_la_SOURCES = vm.c traps.h traps.c trap_libc.c
nodist_libbee@PACKAGE_SUFFIX@_la_SOURCES = private.h
libbee@PACKAGE_SUFFIX@_la_LIBADD = $(top_builddir)/lib/libgnu.la
if HAVE_MIJIT
libbee@PACKAGE_SUFFIX@_la_LIBADD += $(top_srcdir)/mijit-bee/target/release/libmijit_bee.la
endif

bin_PROGRAMS = bee@PACKAGE_SUFFIX@
man_MANS = bee@PACKAGE_SUFFIX@.1
bee@PACKAGE_SUFFIX@_LDADD = libbee@PACKAGE_SUFFIX@.la $(top_builddir)/lib/libgnu.la
bee@PACKAGE_SUFFIX@_SOURCES = main.c gdb-stub.c gdb-stub.h cmdline.h $(include_HEADERS)
pkginclude_HEADERS = include/bee/bee.h include/bee/opcodes.h include/bee/registers.h

if HAVE_MIJIT
# Have a phony target to force cargo to be run always
.PHONY: mijit-bee
$(top_srcdir)/mijit-bee/target/release/libmijit_bee.la: mijit-bee
	cargo build --release --manifest-path $(top_srcdir)/mijit-bee/Cargo.toml
endif

# The man page depends on the binary because it is built by running the binary.
bee@PACKAGE_SUFFIX@.1: bee@PACKAGE_SUFFIX@$(EXEEXT)
## Exit gracefully if bee.1 is not writeable, such as during distcheck!
	$(AM_V_GEN)if ( touch $@.w && rm -f $@.w; ) >/dev/null 2>&1; then \
	  $(top_srcdir)/build-aux/missing --run $(HELP2MAN) --no-info \
		--name="Simple virtual machine" \
		--output=$@ ./bee@PACKAGE_SUFFIX@$(EXEEXT); \
	fi

CLOC = cloc --force-lang="C",h

loc:
	$(CLOC) $(libbee@PACKAGE_SUFFIX@_la_SOURCES) $(bee_SOURCES)

DISTCLEANFILES = bee@PACKAGE_SUFFIX@.1 $(include_HEADERS)
