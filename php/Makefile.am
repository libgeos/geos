#######################################################################
# 
#    GEOS - Geometry Engine Open Source
#    http://trac.osgeo.org/geos
#
#    Copyright (C) 2010 Sandro Santilli <strk@keybit.net>
#
#    This library is free software; you can redistribute it and/or
#    modify it under the terms of the GNU Lesser General Public
#    License as published by the Free Software Foundation; either
#    version 2.1 of the License, or (at your option) any later version.
#
#    This library is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#    Lesser General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software
#    Foundation, Inc., 51 Franklin St, Fifth Floor,
#    Boston, MA  02110-1301  USA
#
#######################################################################


if ENABLE_PHP

SUBDIRS = test

EXTRA_DIST = README 

pkglib_LTLIBRARIES = geos.la
geos_la_SOURCES = geos.c php_geos.h
geos_la_LIBADD =  $(top_builddir)/capi/libgeos_c.la 
# TODO: check if -no-undefined should be added
geos_la_LDFLAGS = -module -avoid-version
# TODO: drop conditionals from geos.c and let it bound to current code
geos_la_CFLAGS = $(AM_CFLAGS) `$(PHP_CONFIG) --includes` \
	-DCOMPILE_DL_GEOS \
	-std=c99

install: all
	$(INSTALL) -d "$(DESTDIR)"/`$(PHP_CONFIG) --extension-dir`
	$(INSTALL) -m 755 .libs/geos.so "$(DESTDIR)"/`php-config --extension-dir`

endif
