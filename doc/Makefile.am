noinst_PROGRAMS = example

LIBS = ../source/geom/libgeos.la

example_SOURCES = example.cpp 
example_LDADD = $(LIBS)

INCLUDES = -I../headers
INCLUDES += -I../io/markup

doxygen:
	doxygen geosDoxygen.conf

clean-generic:
	rm -Rf doxygen_docs
