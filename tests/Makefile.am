# Tests Makefile.am
#
# (c) Reuben Thomas 2011-2023
#
# The package is distributed under the GNU General Public License version 3,
# or, at your option, any later version.
#
# THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
# RISK.

AM_CPPFLAGS = -I$(top_srcdir)/src -I$(top_builddir)/src -I$(top_builddir)/lib -I$(top_srcdir)/lib -I$(top_srcdir)/src/include $(WARN_CFLAGS)

LOG_COMPILER = $(srcdir)/run-test

LDADD = $(builddir)/libtestutil.a $(top_builddir)/src/libbee@PACKAGE_SUFFIX@.la $(top_builddir)/lib/libgnu.la

noinst_LIBRARIES = libtestutil.a
libtestutil_a_SOURCES = util.c util.h

check_PROGRAMS = $(TESTS)

TESTS = arithmetic catch comparison constants jump logic memory \
	registers stack single_step run errors traps
TESTS_ENVIRONMENT = \
	export LIBTOOL=$(top_builddir)/libtool;

# Test binutils support. Assumes binutils for Bee configured with
# --program-prefix=bee- is installed on PATH.
test-binutils:
	bee-as -R -o hello.o $(srcdir)/hello.s && \
	bee-objcopy -O binary hello.o hello.bin && \
	( $(TESTS_ENVIRONMENT) $(LOG_COMPILER) $(top_builddir)/src/bee$(EXEEXT) ./hello.bin > hello.output ) && \
	diff hello.output $(srcdir)/hello.correct

EXTRA_DIST = \
	run-test \
	tests.h \
	hello.s \
	hello.correct

DISTCLEANFILES = hello.obj hello.output
