# $Id$
#
# This file is part of project GEOS (http://trac.osgeo.org/geos/) 
#
noinst_PROGRAMS = \
    CoordinateSequencesExample \
    CustomCoordinateSequenceExample 

LIBS = $(top_srcdir)/src/geom/libgeos.la
# -lmpatrol -lbfd -lintl -liberty -limagehlp

CustomCoordinateSequenceExample_SOURCES = \
    CustomPointCoordinateSequence.cpp \
    CustomCoordinateSequenceExample.cpp \
    CPCLException.cpp \
    CustomCoordinateSequenceExample.h

CustomCoordinateSequenceExample_LDADD = $(LIBS)

CoordinateSequencesExample_SOURCES = CoordinateSequencesExample.cpp 
CoordinateSequencesExample_LDADD = $(LIBS)

INCLUDES = -I$(top_srcdir)/include
INCLUDES += -I$(top_srcdir)/io/markup
