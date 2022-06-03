#
# This file is part of project GEOS (http://trac.osgeo.org/geos/)
#
# NOTE: Make sure that 'src' appears first.
# 'capi' and 'doc' surely rely on availability of libgeos
# which is built under 'src' and it seems that automake
# is not able to detect required build order but blindly
# relies on the order items in SUBDIRS appear.
#

# foreign is needed to avoid enforced ChangeLog in EXTRA_DIST
# (we'll still take care of adding it ourselves)
AUTOMAKE_OPTIONS = foreign

SUBDIRS = \
	include \
	src     \
	capi    \
	doc     \
	macros  \
	tests   \
	benchmarks \
	tools \
	util

BUILT_SOURCES =

EXTRA_DIST = \
	README.md \
	acsite.m4 \
	.editorconfig \
	Version.txt \
	CMakeLists.txt \
	cmake/cmake_uninstall.cmake.in \
	cmake/geos-config.cmake \
	cmake/FindMakeDistCheck.cmake

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = tools/geos.pc

ACLOCAL_AMFLAGS = -I macros

dist-hook: gen-ChangeLog

gen-ChangeLog:
	cd $(distdir) && owd=`pwd` && \
  cd ../ && cd $(srcdir) && \
	if test -d .git; then \
		git log --pretty --numstat --summary | git2cl > $${owd}/ChangeLog; \
	elif test -d .svn; then \
		svn2cl --authors=authors.svn -i -o $${owd}/ChangeLog; \
	elif test -f ChangeLog; then \
		cp ChangeLog  $${owd}/ChangeLog; \
	fi

apidoc doxygen:
	cd doc && make $@

doxygen-checked:
	( set -o pipefail; \
		! make doxygen 2>&1 > /dev/null | \
		grep -v '\(not documented\|ignoring unsupported tag\)'; \
	)

authors.git: authors.svn
	cd $(srcdir) && sed -e 's/:/ = /' authors.svn > authors.git

svnrebase: authors.git
	cd $(srcdir) && git svn rebase --authors-file authors.git

VALGRIND = $(LIBTOOL) --mode=execute valgrind --leak-check=full --error-exitcode=1

valgrindcheck:
	$(VALGRIND) tests/unit/geos_unit

check-local:
	! find $(srcdir) -name '*.cpp' -o -name '*.h' | \
		grep -v tests/xmltester/tinyxml | \
		xargs grep -n '[[:space:]]$$'
