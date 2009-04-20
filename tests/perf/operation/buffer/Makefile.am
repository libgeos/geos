prefix=@prefix@
top_srcdir=@top_srcdir@
top_builddir=@top_builddir@

noinst_PROGRAMS = IteratedBufferStressTest

LIBS = $(top_builddir)/source/libgeos.la

IteratedBufferStressTest_SOURCES = IteratedBufferStressTest.cpp 
IteratedBufferStressTest_LDADD = $(LIBS)

INCLUDES = -I$(top_srcdir)/source/headers
INCLUDES += -I$(top_srcdir)/source/io/markup
