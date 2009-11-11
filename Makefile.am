#
# NOTE: Make sure that 'source' appears first.
# 'capi' and 'doc' surely rely on availability of libgeos
# which is built under 'source' and it seems that automake
# is not able to detect required build order but blindly
# relies on the order items in SUBDIRS appear.
#
SUBDIRS = 	\
	build	\
	source	\
	capi	\
	doc	\
	macros	\
	swig	\
	tests	\
	tools

EXTRA_DIST = acsite.m4 makefile.vc nmake.opt

ACLOCAL_AMFLAGS = -I macros

cl:
	cd $(srcdir) && svn2cl --authors=authors.svn -i -o ChangeLog
