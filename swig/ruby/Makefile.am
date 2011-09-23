# $Id: Makefile.am 2844 2010-01-13 03:03:38Z mloskot $
#
# This file is part of project GEOS (http://trac.osgeo.org/geos/) 
#
EXTRA_DIST = \
	ruby.i 

MAINTAINERCLEANFILES = geos_wrap.cxx

if ENABLE_RUBY

SUBDIRS = \
	test

BUILT_SOURCES = geos_wrap.cxx

# Don't know why there needs to be dirdir but it makes automake happy
rubyextensiondirdir = $(RUBY_EXTENSION_DIR)

# Setup includes
INCLUDES = -I$(RUBY_INCLUDE_DIR)

# Build Ruby module as shared library
rubyextensiondir_LTLIBRARIES = geos.la
geos_la_SOURCES = geos_wrap.cxx
geos_la_LIBADD =  $(top_builddir)/capi/libgeos_c.la -l$(RUBY_SO_NAME)

# Only need to grab the capi header files
geos_la_CPPFLAGS = -I$(top_builddir)/capi

# Specify -module and -avoid-version so we can create a file called geos.dll/so which is what Ruby wants
geos_la_LDFLAGS = -no-undefined  -module -avoid-version -L$(RUBY_LIB_DIR)

if ENABLE_SWIG

geos_wrap.cxx: ../geos.i ruby.i
	$(SWIG) -c++ -ruby -autorename -o $@ $<

else # !ENABLE_SWIG

geos_wrap.cxx: ../geos.i ruby.i
	@echo "SWIG is disabled, can't build geos_wrap.cxx"
	false

endif # ENABLE_SWIG

endif # ENABLE_RUBY
