#!/bin/sh

OSTYPE=`uname -s`

#AMFLAGS="--add-missing --copy --force-missing"
AMFLAGS="--add-missing --copy"
if test "$OSTYPE" = "IRIX" -o "$OSTYPE" = "IRIX64"; then
   AMFLAGS=$AMFLAGS" --include-deps";
fi

echo "Running autoheader"
autoheader
echo "Running aclocal -I macros"
aclocal -I macros
echo "Running libtoolize"
libtoolize --force --copy
echo "Running automake"
automake $AMFLAGS
echo "Running autoconf"
autoconf

echo "======================================"
echo "Now you are ready to run './configure'"
echo "======================================"
