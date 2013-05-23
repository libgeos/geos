#
# This file is part of project GEOS (http://trac.osgeo.org/geos/) 
#
# NOTE: Make sure that 'src' appears first.
# 'capi' and 'doc' surely rely on availability of libgeos
# which is built under 'src' and it seems that automake
# is not able to detect required build order but blindly
# relies on the order items in SUBDIRS appear.
#
SUBDIRS =   \
    include \
    src     \
    capi    \
    doc     \
    macros  \
    swig    \
    php     \
    tests   \
    tools

BUILT_SOURCES = geos_svn_revision.h

EXTRA_DIST = acsite.m4 makefile.vc nmake.opt autogen.bat CMakeLists.txt \
  cmake/modules/CheckPrototypeExists.cmake \
  cmake/modules/COPYING-CMAKE-SCRIPTS \
  cmake/cmake_uninstall.cmake.in geos_svn_revision.h

ACLOCAL_AMFLAGS = -I macros

cl:
	cd $(srcdir) && svn update && svn2cl --authors=authors.svn -i -o ChangeLog

apidoc doxygen:
	cd doc && make $@

authors.git: authors.svn
	cd $(srcdir) && sed -e 's/:/ = /' authors.svn > authors.git

svnrebase: authors.git
	cd $(srcdir) && git svn rebase --authors-file authors.git

geos_svn_revision.h:
	top_srcdir=$(srcdir) sh $(srcdir)/tools/svn_repo_revision.sh

.PHONY: geos_svn_revision.h
