# GNU Makefile, using geos-config

PROGRAM = test_cpp
OBJECTS = test_cpp.o

override CXXFLAGS += -std=c++11 -g -Wall -Werror $(shell geos-config --cflags)
override LDFLAGS += $(shell geos-config --cclibs)

all: $(PROGRAM)

$(PROGRAM): $(OBJECTS)
	$(CXX) -o $@ $< $(LDFLAGS)

clean:
	$(RM) $(PROGRAM) $(OBJECTS)

.PHONY: clean
