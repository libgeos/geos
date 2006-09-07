#!/bin/sh

giveup()
{
        echo
        echo "  Something went wrong, giving up!"
        echo
        exit 1
}

OSTYPE=`uname -s`

#AMFLAGS="--add-missing --copy --force-missing"
AMFLAGS="--add-missing --copy"
if test "$OSTYPE" = "IRIX" -o "$OSTYPE" = "IRIX64"; then
   AMFLAGS=$AMFLAGS" --include-deps";
fi

echo "Running autoheader"
autoheader || giveup
echo "Running aclocal -I macros"
aclocal -I macros || giveup
echo "Running libtoolize"
libtoolize --force --copy || giveup
echo "Running automake"
automake $AMFLAGS # || giveup
echo "Running autoconf"
autoconf || giveup

echo "======================================"
echo "Now you are ready to run './configure'"
echo "======================================"
