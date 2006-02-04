
EXTRA_DIST = \
	ruby.i \
	README.txt 

CLEANFILES = geos_wrap.cxx

if ENABLE_RUBY

SUBDIRS = \
	test

BUILT_SOURCES = geos_wrap.cxx


# ============ Start MinGW handling =============
# Stupid hack for LIBTOOL running under MinGW, which absolutely insists that libraries
# start with lib, which is not the case for the ruby library msvcrt-ruby18.lib.
# So copy it to msvcrt-ruby18.lib

# Note we use make conditional - so add a space in front so automake doesn't 
# think it is a automake conditional.  UGH!

 ifeq ($(findstring msvcrt,$(RUBY_SHARED_LIB)),msvcrt)
  
$(RUBY_LIB_DIR)/lib$(RUBY_SHARED_LIB): $(RUBY_LIB_DIR)/$(RUBY_SHARED_LIB)
	cp $(RUBY_LIB_DIR)/$(RUBY_SHARED_LIB) $(RUBY_LIB_DIR)/lib$(RUBY_SHARED_LIB)

-l$(RUBY_SO_NAME): $(RUBY_LIB_DIR)/lib$(RUBY_SHARED_LIB)

 else

-l$(RUBY_SO_NAME): $(RUBY_LIB_DIR)/$(RUBY_SHARED_LIB)
 
 endif
 
geos_la_DEPENDENCIES = -l$(RUBY_SO_NAME)

# ============= End MingW handling =============

# Don't know why there needs to be dirdir but it makes automake happy
rubyextensiondirdir = $(RUBY_EXTENSION_DIR)

# Setup includes
INCLUDES = -I$(RUBY_INCLUDE_DIR)

# Build Ruby module as shared library
rubyextensiondir_LTLIBRARIES = geos.la
geos_la_SOURCES = geos_wrap.cxx
geos_la_LIBADD =  $(top_builddir)/source/geom/libgeos.la -L$(RUBY_LIB_DIR) -l$(RUBY_SO_NAME)

# Specify -module and -avoid-version so we can create a file called geos.dll/so which is what Ruby wants
geos_la_LDFLAGS = -no-undefined  -module -avoid-version

if ENABLE_SWIG

geos_wrap.cxx: ../geos.i ruby.i
	$(SWIG) -c++ -ruby -autorename -o $@ $<

else # !ENABLE_SWIG

geos_wrap.cxx: 
	@echo "SWIG is disabled, can't build geos_wrap.cxx"
	false

endif # ENABLE_SWIG

endif # ENABLE_RUBY
