#!/bin/sh

#######################################################################
#
# GEOS - Geometry Engine Open Source
# http://geos.osgeo.org
#
# Copyright (C) 2009 Sandro Santilli <strk@keybit.net>
#
# This is free software; you can redistribute and/or modify it under
# the terms of the GNU Lesser General Public Licence as published
# by the Free Software Foundation. 
# See the COPYING file for more information.
#
#######################################################################
#
# This script is an utility to convert SAFE Software testcases to
# the format accepted by XMLTester
#
#######################################################################

if ! test -n "$1"; then
	echo "Usage: $0 <testno>"
	exit 1
fi

SAFETESTNO="$1"

SAFEIN=${SAFETESTNO}_input.txt
SAFEPARAMS=${SAFETESTNO}_params.txt

if ! test -e ${SAFEIN}; then echo "Missing ${SAFEIN}"; exit 1; fi
#if ! test -e ${SAFEOUT_LEFT}; then echo "Missing ${SAFEOUT_LEFT}"; exit 1; fi
#if ! test -e ${SAFEOUT_RIGHT}; then echo "Missing ${SAFEOUT_RIGHT}"; exit 1; fi
if ! test -e ${SAFEOUT_PARAMS}; then echo "Missing ${SAFEOUT_PARAMS}"; exit 1; fi


exec 4< ${SAFEIN}

#exec 5< ${SAFEOUT_LEFT}
#exec 6< ${SAFEOUT_LEFT}

#echo '<run>'
#echo '<precisionModel type="FLOATING" />'

# Parse parameters/tests
TESTS=${SAFEIN}_tests
exec 7< ${SAFEPARAMS}
exec 8> ${TESTS}
arg2=na
style=na
opname=na
while read line <&7; do

	# end of test
	if test -z "${line}"; then
		testline="<op name='buffersinglesided' arg1='a' arg2='${arg2}' arg3='4' arg4='${style}'>|${opname}"
		echo ${testline} >&8
	fi

	lbl=`echo ${line} | cut -d: -f1`
	val=`echo ${line} | cut -d: -f2`

	if test "${lbl}" = "Buffer Amount"; then

		arg2=${val}

	elif test "${lbl}" = "Buffer Style"; then

      # No quotes around ${val} is intentional, to trim blanks
		if test ${val} = "RIGHT_SIDE_ONLY"; then
			style=right
		else
			style=left
		fi

	elif test "${lbl}" = "Interpolation Angle"; then

		continue

	else

		opname=`echo ${lbl} | cut -d' ' -f 2 | tr '[A-Z]' ['a-z']`
		if test "${opname}" = 'left' -o "${opname}" = 'right'; then
			opname="_${opname}"
		fi

	fi
		
done

testline="<op name='buffersinglesided' arg1='a' arg2='${arg2}' arg3='4' arg4='${style}'>|${opname}"
echo ${testline} >&8

echo '<run>'
echo '<precisionModel type="FLOATING" />'

# A case each input
caseno=1
while read inp <&4; do

	echo "<case><desc>SAFE test ${SAFETESTNO} line ${caseno}</desc>"
	echo '<a>'

	# hack until GEOS supports proper OGC WKT: POINT Z (0 0)
	inp=`echo "${inp}" | sed -e 's/ Z //'`
	echo ${inp}
	echo '</a>'


	# A test each param
	exec 9< ${TESTS}
	while read testspec <&9; do

		test=`echo "${testspec}" | cut -d'|' -f1`
		out=`echo "${testspec}" | cut -d'|' -f2`

		echo '<test>'
		echo '  '${test}

		# read line from line ${caseno} of ${SAFEOUT}
		SAFEOUT=${SAFETESTNO}_output${out}.txt
		expout=`head -${caseno} ${SAFEOUT} | tail -1`

		# hack until GEOS supports proper OGC WKT: POINT Z (0 0)
		expout=`echo "${expout}" | sed -e 's/ Z //'`

		echo ${expout}

		echo '  </op>'
		echo '</test>'

	done


	echo '</case>'

	caseno=$((caseno+1))


done

echo '</run>'
