# $Id$
#
# tests/thread builds threading test programs
#
INCLUDES = -I$(top_srcdir)/source/headers -I$(top_srcdir)/tests/unit/tut -I$(top_builddir)/capi

# TODO: Enable if sample input WKT file is provided
#TESTS = threadtest badthreadtest

check_PROGRAMS = threadtest badthreadtest


# The -lstdc++ is needed for --disable-shared to work
threadtest_SOURCES = threadtest.c
threadtest_LDADD = $(top_builddir)/capi/libgeos_c.la -lpthread -lstdc++

# The -lstdc++ is needed for --disable-shared to work
badthreadtest_SOURCES = badthreadtest.c
badthreadtest_LDADD = $(top_builddir)/capi/libgeos_c.la -lpthread -lstdc++
