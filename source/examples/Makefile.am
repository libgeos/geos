noinst_PROGRAMS = CoordinateListsExample \
                  CustomCoordinateListExample 

LIBS = ../geom/libgeos.la
# -lmpatrol -lbfd -lintl -liberty -limagehlp

CustomCoordinateListExample_SOURCES = CustomPointCoordinateList.cpp CustomCoordinateListExample.cpp CPCLException.cpp CustomCoordinateListExample.h
CustomCoordinateListExample_LDADD = $(LIBS)

CoordinateListsExample_SOURCES = CoordinateListsExample.cpp 
CoordinateListsExample_LDADD = $(LIBS)

INCLUDES = -I../headers
INCLUDES += -I../io/markup
