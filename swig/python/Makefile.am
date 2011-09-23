#
# This file is part of project GEOS (http://trac.osgeo.org/geos/) 
#
SUBDIRS = \
	tests

EXTRA_DIST = \
	python.i \
	geos.pth 

MAINTAINERCLEANFILES = geos.py geos_wrap.cxx

if ENABLE_PYTHON

BUILT_SOURCES = geos_wrap.cxx

python_DATA = geos.pth
pkgpython_PYTHON = geos.py
pkgpyexec_LTLIBRARIES = _geos.la

_geos_la_SOURCES = geos_wrap.cxx 

# Only need to grab the capi header files
_geos_la_CPPFLAGS = $(SWIG_PYTHON_CPPFLAGS) -I$(top_builddir)/capi -I$(top_srcdir)/include
# Specify -module and -avoid-version so we can create a file called geos.dll/so which is what Python wants
_geos_la_LDFLAGS = -no-undefined -module -avoid-version
_geos_la_LIBADD = $(top_builddir)/capi/libgeos_c.la $(PYTHON_LDFLAGS)

if ENABLE_SWIG

geos_wrap.cxx: ../geos.i python.i
	$(SWIG) $(SWIG_PYTHON_OPT) -o $@ $<

else # ! ENABLE_SWIG

geos_wrap.cxx: ../geos.i python.i
	@echo "SWIG is disabled, can't build geos_wrap.cxx"
	@false

endif # ENABLE_SWIG

test: 
	$(PYTHON) tests/runtests.py -v

endif # ENABLE_PYTHON
