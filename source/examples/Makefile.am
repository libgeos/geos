noinst_PROGRAMS = CoordinateListsExample \
                  CustomCoordinateListExample \
                  CustomPointCoordinateListExample

LIBS = ../geom/libgeos.la

CustomCoordinateListExample_SOURCES = CustomCoordinateListExample.cpp
CustomCoordinateListExample_LDADD = $(LIBS)

CustomPointCoordinateListExample_SOURCES = CustomPointCoordinateListExample.cpp
CustomPointCoordinateListExample_LDADD = $(LIBS)

CoordinateListsExample_SOURCES = CoordinateListsExample.cpp
CoordinateListsExample_LDADD = $(LIBS)

INCLUDES = -I../headers
INCLUDES += -I../io/markup







