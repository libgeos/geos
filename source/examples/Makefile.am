noinst_PROGRAMS = CoordinateSequencesExample \
                  CustomCoordinateSequenceExample 

LIBS = ../geom/libgeos.la
# -lmpatrol -lbfd -lintl -liberty -limagehlp

CustomCoordinateSequenceExample_SOURCES = CustomPointCoordinateSequence.cpp CustomCoordinateSequenceExample.cpp CPCLException.cpp CustomCoordinateSequenceExample.h
CustomCoordinateSequenceExample_LDADD = $(LIBS)

CoordinateSequencesExample_SOURCES = CoordinateSequencesExample.cpp 
CoordinateSequencesExample_LDADD = $(LIBS)

INCLUDES = -I../headers
INCLUDES += -I../io/markup
