prefix=@prefix@
top_srcdir=@top_srcdir@
top_builddir=@top_builddir@

noinst_PROGRAMS = example 

LIBS = $(top_builddir)/source/libgeos.la

example_SOURCES = example.cpp 
example_LDADD = $(LIBS)

INCLUDES = -I$(top_srcdir)/source/headers

DOXYGEN = doxygen

doxygen-html: Doxyfile
	$(DOXYGEN) $<

clean-generic:
	rm -Rf doxygen_docs

EXTRA_DIST = Doxyfile

