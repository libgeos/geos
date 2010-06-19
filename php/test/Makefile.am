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

EXTRA_DIST = test.php crashme.php

abs_topbuilddir=`cd $(top_builddir); pwd`
abs_srcdir=`cd $(srcdir); pwd`

if ENABLE_PHP_TESTS

TESTS = phpunit

PHPOPTS = -n -d enable_dl=On -d extension_dir=$(abs_topbuilddir)/php/.libs

phpunit: test.php 
	echo "#!/bin/sh" > $@ 
	echo "$(PHP) $(PHPOPTS) $(PHPUNIT) test $(abs_srcdir)/test.php $$@" >> $@
	chmod +x $@

crashtest:
	$(PHP) $(PHPOPTS) crashme.php

clean-local:
	rm -f phpunit


endif
