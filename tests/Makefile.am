# Tests Makefile.am
#
# (c) Reuben Thomas 2011-2022
#
# The package is distributed under the GNU General Public License version 3,
# or, at your option, any later version.
#
# THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
# RISK.

AM_CPPFLAGS = -I$(top_srcdir)/src -I$(top_builddir)/src -I$(top_builddir)/lib -I$(top_srcdir)/lib -I$(top_srcdir)/src/include -I$(top_builddir)/src/include $(WARN_CFLAGS)

LOG_COMPILER = $(srcdir)/run-test

LDADD = $(top_builddir)/src/libbee$(PACKAGE_SUFFIX).la $(top_builddir)/lib/libgnu.la

check_PROGRAMS = $(TESTS)

TESTS = arithmetic catch comparison constants init jump logic memory \
	registers stack single_step run errors traps
TESTS_ENVIRONMENT = \
	export LIBTOOL=$(top_builddir)/libtool;

# Test binutils support. Assumes binutils for Bee configured with
# --program-prefix=bee- is installed on PATH.
test-binutils:
	bee-as -R -o hello.o $(srcdir)/hello.S && \
	bee-objcopy -O binary hello.o hello.bin && \
	( $(TESTS_ENVIRONMENT) $(LOG_COMPILER) $(top_builddir)/src/bee$(EXEEXT) ./hello.bin > hello.output ) && \
	diff hello.output $(srcdir)/hello.correct

EXTRA_DIST = \
	run-test \
	tests.h \
	hello.S \
	hello.correct

DISTCLEANFILES = hello.obj hello.output
