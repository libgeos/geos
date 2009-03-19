#!/bin/sh
#
# $Id$
#
# GEOS Bootstrapping Script
#
giveup()
{
        echo
        echo "  Something went wrong, giving up!"
        echo
        exit 1
}

OSTYPE=`uname -s`

for aclocal in aclocal aclocal-1.10 aclocal-1.9; do
    ACLOCAL=`which $aclocal 2>/dev/null`
    if test -x "${ACLOCAL}"; then
        break;
    fi
done
if [ ! $ACLOCAL ]; then
    echo "Missing aclocal!"
    exit
fi

for automake in automake automake-1.10 automake-1.9; do
    AUTOMAKE=`which $automake 2>/dev/null`
    if test -x "${AUTOMAKE}"; then
        break;
    fi
done
if [ ! $AUTOMAKE ]; then
    echo "Missing automake!"
    exit
fi


for libtoolize in glibtoolize libtoolize; do
    LIBTOOLIZE=`which $libtoolize 2>/dev/null`
    if test -x "${LIBTOOLIZE}"; then
        break;
    fi
done
if [ ! $LIBTOOLIZE ]; then
    echo "Missing libtoolize!"
    exit
fi

#AMFLAGS="--add-missing --copy --force-missing"
AMFLAGS="--add-missing --copy"
if test "$OSTYPE" = "IRIX" -o "$OSTYPE" = "IRIX64"; then
   AMFLAGS=$AMFLAGS" --include-deps";
fi

echo "Running aclocal -I macros"
$ACLOCAL -I macros || giveup
echo "Running autoheader"
autoheader || giveup
echo "Running libtoolize"
$LIBTOOLIZE --force --copy || giveup
echo "Running automake"
$AUTOMAKE $AMFLAGS # || giveup
echo "Running autoconf"
autoconf || giveup

echo "======================================"
echo "Now you are ready to run './configure'"
echo "======================================"
