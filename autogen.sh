#!/bin/sh
#
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

AUTOCONF=`which autoconf 2>/dev/null`
if [ ! ${AUTOCONF} ]; then
    echo "Missing autoconf!"
    exit
fi
AUTOCONF_VER=`${AUTOCONF} --version | grep -E "^.*[0-9]$" | sed 's/^.* //'`

AUTOHEADER=`which autoheader 2>/dev/null`
if [ ! ${AUTOHEADER} ]; then
    echo "Missing autoheader!"
    exit
fi

for aclocal in aclocal aclocal-1.10 aclocal-1.9; do
    ACLOCAL=`which $aclocal 2>/dev/null`
    if test -x "${ACLOCAL}"; then
        break;
    fi
done
if [ ! ${ACLOCAL} ]; then
    echo "Missing aclocal!"
    exit
fi
ACLOCAL_VER=`${ACLOCAL} --version | grep -E "^.*[0-9]$" | sed 's/^.* //'`

for automake in automake automake-1.10 automake-1.9; do
    AUTOMAKE=`which $automake 2>/dev/null`
    if test -x "${AUTOMAKE}"; then
        break;
    fi
done
if [ ! ${AUTOMAKE} ]; then
    echo "Missing automake!"
    exit
fi
AUTOMAKE_VER=`${AUTOMAKE} --version | grep -E "^.*[0-9]$" | sed 's/^.* //'`


for libtoolize in libtoolize glibtoolize; do
    LIBTOOLIZE=`which $libtoolize 2>/dev/null`
    if test -x "${LIBTOOLIZE}"; then
        break;
    fi
done
if [ ! ${LIBTOOLIZE} ]; then
    echo "Missing libtoolize!"
    exit
fi
LIBTOOLIZE_VER=`${LIBTOOLIZE} --version | grep -E "^.*[0-9]\.[0-9]" | sed 's/^.* //'`

AMOPTS="--add-missing --copy -Woverride"
if test "$OSTYPE" = "IRIX" -o "$OSTYPE" = "IRIX64"; then
   AMOPTS=$AMOPTS" --include-deps";
fi

LTOPTS="--force --copy"
echo "* Running ${LIBTOOLIZE} (${LIBTOOLIZE_VER})"
echo "   OPTIONS = ${LTOPTS}"
${LIBTOOLIZE} ${LTOPTS} || giveup

echo "* Running $ACLOCAL (${ACLOCAL_VER})"
${ACLOCAL} -I macros || giveup

echo "* Running ${AUTOHEADER} (${AUTOCONF_VER})"
${AUTOHEADER} || giveup

echo "* Running ${AUTOMAKE} (${AUTOMAKE_VER})"
echo "   OPTIONS = ${AMOPTS}"
${AUTOMAKE} ${AMOPTS} || giveup

echo "* Running ${AUTOCONF} (${AUTOCONF_VER})"
${AUTOCONF} || giveup

if test -f "${PWD}/configure"; then
    echo "======================================"
    echo "Now you are ready to run './configure'"
    echo "======================================"
else
    echo "  Failed to generate ./configure script!"
    giveup
fi

