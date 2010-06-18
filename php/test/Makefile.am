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

if ENABLE_PHP

check:
	php -n -d enable_dl=On -d extension_dir=../.libs test.php

crashtest:
	php -n -d enable_dl=On -d extension_dir=../.libs crashme.php

endif
