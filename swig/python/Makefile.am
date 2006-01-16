SUBDIRS = \
	tests

EXTRA_DIST = \
	setup.py \
	README.txt 

#MAINTAINERCLEANFILES = geos_wrap.cxx
#MOSTLYCLEANFILES = geos.py
CLEANFILES = geos_wrap.cxx geos.py

if ENABLE_PYTHON

BUILT_SOURCES = geos_wrap.cxx

pkgpython_PYTHON = geos.py
pkgpyexec_LTLIBRARIES = geos.la

geos_la_SOURCES = geos_wrap.cxx 
geos_la_CPPFLAGS = $(SWIG_PYTHON_CPPFLAGS) -I$(top_srcdir)/src
geos_la_LDFLAGS = -no-undefined -module -avoid-version
geos_la_LIBADD = $(top_builddir)/source/geom/libgeos.la 



#all: geos.py
#
#geos.py: geos_wrap.cxx setup.py
#	$(PYTHON) setup.py build
#
#
#install: geos.py setyp.py
#	$(PYTHON) setup.py install

geos_wrap.cxx: ../geos.i
	$(SWIG) $(SWIG_PYTHON_OPT) -o $@ $<

test: 
	$(PYTHON) tests/runtests.py -v

endif # ENABLE_PYTHON
