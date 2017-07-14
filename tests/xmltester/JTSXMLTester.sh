#!/bin/sh

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
org.locationtech.jtstest.testrunner.TopologyTestApp -files $@
