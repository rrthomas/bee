# Tests Makefile.am
#
# (c) Reuben Thomas 2011-2020
#
# The package is distributed under the GNU Public License version 3, or,
# at your option, any later version.
#
# THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
# RISK.

AM_CPPFLAGS = -I$(top_srcdir)/src -I$(top_builddir)/src -I$(top_builddir)/lib -I$(top_srcdir)/lib -I$(top_srcdir)/src/include $(WARN_CFLAGS)

LOG_COMPILER = env $(LOG_ENV) $(srcdir)/run-test

LDADD = $(top_builddir)/src/libbee.la

check_PROGRAMS = $(TESTS) load_object

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

do_load_object: load_object
	$(TESTS_ENVIRONMENT) $(LOG_COMPILER) $(builddir)/load_object $(srcdir)

check-local: do_load_object

EXTRA_DIST = \
	run-test \
	tests.h \
	testobj1-le \
	testobj2-le \
	testobj1-be \
	testobj2-be \
	hello.S \
	hello.correct

DISTCLEANFILES = hello.obj hello.output
