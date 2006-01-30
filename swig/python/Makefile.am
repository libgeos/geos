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

python_DATA = geos.pth
pkgpython_PYTHON = geos.py
pkgpyexec_LTLIBRARIES = _geos.la

_geos_la_SOURCES = geos_wrap.cxx 
_geos_la_CPPFLAGS = $(SWIG_PYTHON_CPPFLAGS) -I$(top_srcdir)/src
# Specify -module and -avoid-version so we can create a file called geos.dll/so which is what Python wants
_geos_la_LDFLAGS = -no-undefined -module -avoid-version
_geos_la_LIBADD = $(top_builddir)/source/geom/libgeos.la $(PYTHON_LDFLAGS)



#all: geos.py
#
#geos.py: geos_wrap.cxx setup.py
#	$(PYTHON) setup.py build
#
#
#install: geos.py setyp.py
#	$(PYTHON) setup.py install

if ENABLE_SWIG

geos_wrap.cxx: ../geos.i
	$(SWIG) $(SWIG_PYTHON_OPT) -o $@ $<

else # ! ENABLE_SWIG

geos_wrap.cxx: 
	@echo "SWIG is disabled, can't build geos_wrap.cxx"
	@false

endif # ENABLE_SWIG

test: 
	$(PYTHON) tests/runtests.py -v

endif # ENABLE_PYTHON
