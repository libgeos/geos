noinst_PROGRAMS = CoordinateListsExample \
                  CustomCoordinateListExample 

LIBS = ../geom/libgeos.la

CustomCoordinateListExample_SOURCES = CustomPointCoordinateList.cpp CustomCoordinateListExample.cpp CPCLException.cpp
CustomCoordinateListExample_LDADD = $(LIBS)

CoordinateListsExample_SOURCES = CoordinateListsExample.cpp 
CoordinateListsExample_LDADD = $(LIBS)

INCLUDES = -I../headers
INCLUDES += -I../io/markup







