# GNU Makefile, using geos-config

PROGRAM = test_c
OBJECTS = test_c.o

override CFLAGS += -g -Wall -Werror $(shell geos-config --cflags)
override LDFLAGS += $(shell geos-config --clibs)

all: $(PROGRAM)

$(PROGRAM): $(OBJECTS)
	$(CC) -o $@ $< $(LDFLAGS)

clean:
	$(RM) $(PROGRAM) $(OBJECTS)

.PHONY: clean
