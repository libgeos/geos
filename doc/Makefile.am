noinst_PROGRAMS = example 

LIBS = ../source/geom/libgeos.la

example_SOURCES = example.cpp 
example_LDADD = $(LIBS)

INCLUDES = -I../source/headers

DOXYGEN = doxygen

doxygen-html: Doxyfile
	$(DOXYGEN) $<

clean-generic:
	rm -Rf doxygen_docs

EXTRA_DIST = Doxyfile

