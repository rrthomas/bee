# Source Makefile.am
#
# (c) Reuben Thomas 2011-2020
#
# The package is distributed under the GNU Public License version 3, or,
# at your option, any later version.
#
# THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
# RISK.

AM_CPPFLAGS = -I$(top_builddir)/lib -I$(top_srcdir)/lib $(WARN_CFLAGS)

lib_LTLIBRARIES = libbee.la
libbee_la_SOURCES = storage.c run.c extra.c debug.c loadobj.c \
	private.h stringify.h
libbee_la_LIBADD = $(top_builddir)/lib/libgnu.la

bin_PROGRAMS = bee
man_MANS = bee.1
bee_LDADD = libbee.la $(top_builddir)/lib/libgnu.la
bee_SOURCES = main.c cmdline.h tbl_registers.h $(include_HEADERS)
include_HEADERS = bee.h bee_aux.h bee_debug.h bee_opcodes.h

# Depend on bee$(EXEEXT) rather than explicitly make-ing it, as otherwise
# we break parallel builds, as libbee.la can be built twice in parallel,
# which can fail. Set distcleancheck_listfiles below to fix distcheck.
bee.1: bee$(EXEEXT)
## Exit gracefully if bee.1 is not writeable, such as during distcheck!
	$(AM_V_GEN)if ( touch $@.w && rm -f $@.w; ) >/dev/null 2>&1; then \
	  $(top_srcdir)/build-aux/missing --run $(HELP2MAN) --no-info \
		--name="Forth virtual machine" \
		--output=$@ ./bee$(EXEEXT); \
	fi

CLOC = cloc --force-lang="C",h

loc:
	$(CLOC) $(libbee_la_SOURCES) $(bee_SOURCES)

EXTRA_DIST = external_syms.h

DISTCLEANFILES = bee.1 $(include_HEADERS)
# Ignore built files that are part of the distribution (specifically,
# bee.1)
distcleancheck_listfiles = \
       find . -type f -exec sh -c 'test -f $(srcdir)/$$1 || echo $$1' \
	    sh '{}' ';'
