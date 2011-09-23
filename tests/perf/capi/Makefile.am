#
#
# This file is part of project GEOS (http://trac.osgeo.org/geos/) 
#
prefix=@prefix@
top_srcdir=@top_srcdir@
top_builddir=@top_builddir@

check_PROGRAMS = memleak_mp_prep

LIBS = $(top_builddir)/capi/libgeos_c.la
INCLUDES = -I$(top_builddir)/capi -I$(top_srcdir)/include

memleak_mp_prep_SOURCES = memleak_mp_prep.c
memleak_mp_prep_LDADD = $(LIBS)

