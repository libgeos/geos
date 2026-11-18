# GNU Makefile used by test_pkg-config.sh

PROGRAM = cpp_app
OBJECTS = $(addsuffix .o,$(PROGRAM))
TESTS = \
  test_length.sh \
  test_libpath.sh \
  test_version.sh

override CXXFLAGS += -std=c++17 -fvisibility=hidden -g -Wall -Werror $(shell pkg-config geos --cflags) -DUSE_UNSTABLE_GEOS_CPP_API

ifeq ($(library_type),static)
  _uname_s := $(shell uname -s)
  _ldflags := $(shell pkg-config geos --libs --static)
  # remove unnecessary
  _ldflags := $(shell echo $(_ldflags) | sed 's/-lgeos_c//')
  ifeq ($(_uname_s),Linux)
    # force static linking to libgeos
    _ldflags := $(shell echo $(_ldflags) | sed 's/-lgeos/-Wl,-Bstatic -lgeos -Wl,-Bdynamic/')
  endif
  override LDFLAGS += $(_ldflags)
else  # default is shared
  override LDFLAGS += $(shell pkg-config geos --libs-only-L) -lgeos
endif

all: $(PROGRAM)

$(PROGRAM): $(OBJECTS)
	$(CXX) -o $@ $< $(LDFLAGS)

test: $(PROGRAM)
	set -e; for t in $(TESTS); do ./$$t; done

clean:
	$(RM) $(PROGRAM) $(OBJECTS)

.PHONY: test clean
