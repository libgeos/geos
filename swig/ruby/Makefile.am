SUBDIRS = \
	test

EXTRA_DIST = \
	ruby.i \
	renames.i \
	example.rb \
	README.txt 

CLEANFILES = geos_wrap.cxx

if ENABLE_RUBY

BUILT_SOURCES = geos_wrap.cxx

RUBY_LIB_DIR := $(shell ruby -rrbconfig -e "puts Config::CONFIG['libdir']")
RUBY_INCLUDE_DIR := $(shell ruby -rrbconfig -e "puts Config::CONFIG['archdir']")
RUBY_SO_NAME := $(shell ruby -rrbconfig -e "puts Config::CONFIG['RUBY_SO_NAME']")
RUBY_EXTENSIONS_DIR := $(shell ruby -rrbconfig -e "puts Config::CONFIG['sitearchdir']")

INCLUDES = -I$(RUBY_INCLUDE_DIR)



# ============ Start MinGW handling =============
# Stupid hack for LIBTOOL running under MinGW, which absolutely insists that libraries
# start with lib, which is not the case for the ruby library msvcrt-ruby18.lib.
# SO copy it to msvcrt-ruby18.lib

# Note we use make conditional - so add a space in front so automake doesn't 
# think it is a automake conditional.  UGH!

 ifeq "$(build_os)" "mingw32"

$(RUBY_SO_NAME):

lib$(RUBY_SO_NAME): $(RUBY_SO_NAME)
	cp $(RUBY_LIB_DIR)/msvcrt-ruby18.lib $(RUBY_LIB_DIR)/libmsvcrt-ruby18.lib

geos_la_DEPENDENCIES = lib$(RUBY_SO_NAME)

 endif

# ============= End MingW handling =============

# Don't know why there needs to be dirdir but it makes automake happy
rubyexecdirdir = $(RUBY_EXTENSIONS_DIR)

# Build Ruby module as shared library
rubyexecdir_LTLIBRARIES = geos.la
geos_la_SOURCES = geos_wrap.cxx
geos_la_LIBADD =  $(top_builddir)/source/geom/libgeos.la -L$(RUBY_LIB_DIR) -lmsvcrt-ruby18

# Specify -module and -avoid-version so we can create a file called geos.dll/so which is what Ruby wants
geos_la_LDFLAGS = -no-undefined  -module -avoid-version

geos_wrap.cxx: ../geos.i
	$(SWIG) -c++ -ruby -o $@ $<

endif # ENABLE_RUBY
