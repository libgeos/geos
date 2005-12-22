# This makefile generates a Ruby extension for GEOS using GCC
# There are two important things to note:
#
# * It makes multiple calls to Ruby to discover the Ruby version,
#	location of header files, libraries, etc.  Thus Ruby must
#	be on your path.
#
# * By convention Ruby method names are lower case with underscores,
#	thus methods such as "GetFieldName" need to be mapped to
#	"get_field_name."  This is done by running the separate
#	RenamesMakefile.mk script.

GEOS_ROOT = ../..
RUBY = ruby

RUBY_MODULES = geos.so
RUBY_MAJOR_VERSION := $(shell ruby -rrbconfig -e "puts Config::CONFIG['MAJOR']")
RUBY_MINOR_VERSION := $(shell ruby -rrbconfig -e "puts Config::CONFIG['MINOR']")
RUBY_VERSION := $(RUBY_MAJOR_VERSION).$(RUBY_MINOR_VERSION)
RUBY_SO_NAME := $(shell ruby -rrbconfig -e "puts Config::CONFIG['RUBY_SO_NAME']")

RUBY_DIR := $(shell ruby -rrbconfig -e "puts Config::TOPDIR")
RUBY_ARCH := $(shell ruby -rrbconfig -e "puts Config::CONFIG['arch']")
RUBY_ARCH_DIR := $(RUBY_DIR)/lib/ruby/$(RUBY_VERSION)/$(RUBY_ARCH)

SITE_LIB_DIR := $(shell ruby -rrbconfig -e "puts Config::CONFIG['sitelibdir']")
INSTALL_DIR := $(SITE_LIB_DIR)/$(RUBY_ARCH)

GEOS_INCLUDE := -I$(GEOS_ROOT)/source/headers
RUBY_INCLUDE := -I$(RUBY_ARCH_DIR)

GEOS_LIB := -lgeos
RUBY_LIB := -shared -L$(RUBY_DIR)/lib -l$(RUBY_SO_NAME)

build: $(RUBY_MODULES)

clean:
	rm -f *.so
	rm -f *.o
	rm -f *.lo
	
veryclean: clean
	rm -frd $(INSTALL_DIR)

$(INSTALL_DIR):
	mkdir -v $(INSTALL_DIR)

install: $(INSTALL_DIR)
	cp *.so $(INSTALL_DIR)

$(RUBY_MODULES): %.so: %_wrap.o
	$(CXX) $(LDFLAGS) $(LIBS) $(GEOS_LIB) $(RUBY_LIB) $< -o $@

%.o: %.cpp
	$(CXX) $(CFLAGS) $(GEOS_INCLUDE) $(RUBY_INCLUDE)  -c $<

%.o: %.cxx
	$(CXX) $(CFLAGS) $(GEOS_INCLUDE) $(RUBY_INCLUDE) -c $<