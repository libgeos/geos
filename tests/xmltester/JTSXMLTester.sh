#!/bin/bash

######################################################################
#
# GEOS - Geometry Engine Open Source
# http://geos.osgeo.org
#
# Copyright (C) 2010-2017 Sandro Santilli <strk@kbt.io>
# Copyright (C) 2010-2012 Martin Davis <mtnclimb@gmail.com>
#
# This is free software; you can redistribute and/or modify it under
# the terms of the GNU Lesser General Public Licence as published
# by the Free Software Foundation.
# See the COPYING file for more information.
#
######################################################################

if test "x$JTS_LIB_DIR" = "x"; then
  JTS_LIB_DIR="$HOME/.m2/repository/org/locationtech/jts"
fi

#---------------------------------#
# dynamically build the classpath #
#---------------------------------#
THE_CLASSPATH=
for i in `find ${JTS_LIB_DIR} -name '*.jar'`
do
  THE_CLASSPATH=${THE_CLASSPATH}:${i}
done

#---------------------------#
# run the program           #
#---------------------------#
java -cp ".:${THE_CLASSPATH}" \
org.locationtech.jtstest.testrunner.JTSTestRunnerCmd $@
