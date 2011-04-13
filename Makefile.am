# $Id$
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

EXTRA_DIST = acsite.m4 makefile.vc nmake.opt autogen.bat

ACLOCAL_AMFLAGS = -I macros

cl:
	cd $(srcdir) && svn2cl --authors=authors.svn -i -o ChangeLog

apidoc doxygen:
	cd doc && make $@

authors.git: authors.svn
	cd $(srcdir) && sed -e 's/:/ = /' authors.svn > authors.git

svnrebase: authors.git
	cd $(srcdir) && git svn rebase --authors-file authors.git
