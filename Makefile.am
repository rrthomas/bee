# Top-level Makefile.am
#
# (c) Reuben Thomas 2011-2018
#
# The package is distributed under the GNU General Public License version 3,
# or, at your option, any later version.
#
# THIS PROGRAM IS PROVIDED AS IS, WITH NO WARRANTY. USE IS AT THE USER’S
# RISK.

SUBDIRS = lib src tests

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = m4/gnulib-cache.m4 $(top_srcdir)/.version

BUILT_SOURCES = $(top_srcdir)/.version

$(top_srcdir)/.version:
	echo $(VERSION) > $@-t && mv $@-t $@

dist-hook:
	echo $(VERSION) > $(distdir)/.tarball-version

release: distcheck
	git diff --exit-code && \
	git tag -a -m "Release tag" "v$(VERSION)" && \
	git push && git push --tags && \
	woger github \
		package=$(PACKAGE) \
		version=$(VERSION) \
		dist_type=tar.gz
