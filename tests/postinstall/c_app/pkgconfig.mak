# GNU Makefile used by test_pkg-config.sh

PROGRAM = c_app
OBJECTS = $(addsuffix .o,$(PROGRAM))
TESTS = \
  test_length.sh \
  test_libpath.sh \
  test_version.sh

override CFLAGS += -g -Wall -Werror $(shell ${PKG_CONFIG} geos --cflags)

ifeq ($(library_type),static)
  _uname_s := $(shell uname -s)
  _ldflags := $(shell ${PKG_CONFIG} geos --libs --static)
  ifeq ($(_uname_s),Linux)
    # force static linking to libgeos
    _ldflags := $(shell echo $(_ldflags) | sed 's/-lgeos_c/-Wl,-Bstatic -lgeos_c -Wl,-Bdynamic/')
  endif
  override LDFLAGS += $(_ldflags)
else  # default is shared
  override LDFLAGS += $(shell ${PKG_CONFIG} geos --libs)
endif

all: $(PROGRAM)

$(PROGRAM): $(OBJECTS)
	$(CC) -o $@ $< $(LDFLAGS)

test: $(PROGRAM)
	set -e; for t in $(TESTS); do ./$$t; done

clean:
	$(RM) $(PROGRAM) $(OBJECTS)

.PHONY: test clean
