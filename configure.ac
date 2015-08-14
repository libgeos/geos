dnl
dnl configure.in - autoconf input template to produce ./configure script
dnl
dnl version 2.52 is required for Cygwin libtool support
AC_PREREQ([2.52])

dnl local vars to hold user's preferences --------------------------------
AC_INIT([include/geos.h])
AC_CANONICAL_SYSTEM

AC_CONFIG_MACRO_DIR([macros])

dnl -- JTS_PORT: the version of JTS this release is bound to
JTS_PORT=1.13.0

dnl -- Version info for the CAPI
CAPI_INTERFACE_CURRENT=10
CAPI_INTERFACE_REVISION=0
CAPI_INTERFACE_AGE=9

dnl
dnl -- Release versions / C++ library SONAME will use these
dnl -- encoding ABI break at every release
dnl
VERSION_MAJOR=3
VERSION_MINOR=5
VERSION_PATCH=0
VERSION="$VERSION_MAJOR.$VERSION_MINOR.$VERSION_PATCH"

dnl CAPI_VERSION_MAJOR=$(($CAPI_INTERFACE_CURRENT-$CAPI_INTERFACE_AGE))
dnl the following should be more portable
CAPI_VERSION_MAJOR=`expr $CAPI_INTERFACE_CURRENT - $CAPI_INTERFACE_AGE`
CAPI_VERSION_MINOR=$CAPI_INTERFACE_AGE
CAPI_VERSION_PATCH=$CAPI_INTERFACE_REVISION
CAPI_VERSION="$CAPI_VERSION_MAJOR.$CAPI_VERSION_MINOR.$CAPI_VERSION_PATCH"

AM_INIT_AUTOMAKE([geos], [$VERSION], [no-define])
AM_MAINTAINER_MODE
AC_CONFIG_HEADERS([include/config.h] [include/geos/platform.h])
AC_PROG_CC

dnl use libtool ----------------------------------------------------------
AC_LIBTOOL_DLOPEN
AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL

dnl check for programs ----------------------------------------------------
AC_PROG_CXX
AC_ISC_POSIX
AC_PROG_INSTALL
AC_PROG_MAKE_SET

dnl function checks ------------------------------------------------------
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_MEMCMP
AC_FUNC_STRFTIME
AC_FUNC_VPRINTF
AC_FUNC_ALLOCA
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([memory.h])
AC_CHECK_HEADERS([unistd.h])
AC_CHECK_HEADERS([ieeefp.h])
AC_CHECK_HEADERS([sys/file.h])
AC_CHECK_HEADERS([sys/time.h])
AC_CHECK_FUNCS([strchr memcpy gettimeofday])
AC_HEADER_STAT
AC_STRUCT_TM
AC_TYPE_SIZE_T
AC_C_CONST

dnl --------------------------------------------------------------------
dnl - Check for inline and cassert settings
dnl --------------------------------------------------------------------


AC_ARG_ENABLE([inline], [  --disable-inline    Disable inlining],
	[case "${enableval}" in
		yes) enable_inline=true ;;
		no)  enable_inline=false ;;
		*) AC_MSG_ERROR(bad value ${enableval} for --enable-inline);;
	esac],
	[enable_inline=true]
)

AC_ARG_ENABLE([cassert], [  --disable-cassert   Disable assertion checking],
	[case "${enableval}" in
		yes) enable_cassert=true ;;
		no)  enable_cassert=false ;;
		*) AC_MSG_ERROR(bad value ${enableval} for --enable-cassert);;
	esac],
	[enable_cassert=true]
)

AC_ARG_ENABLE([glibcxx-debug], [  --enable-glibcxx-debug    Enable libstdc++ debug mode],
	[case "${enableval}" in
		yes) enable_glibcxx_debug=true ;;
		no)  enable_glibcxx_debug=false ;;
		*) AC_MSG_ERROR(bad value ${enableval} for --enable-glibcxx-debug);;
	esac],
	[enable_glibcxx_debug=false]
)


AC_MSG_CHECKING([if requested to force inline functions])
INLINE_FLAGS=
AC_SUBST(INLINE_FLAGS)
if test x"$enable_inline" = xtrue; then
	INLINE_FLAGS="-DGEOS_INLINE"
	AM_CXXFLAGS="$AM_CXXFLAGS $INLINE_FLAGS"
    AC_MSG_RESULT([yes])
else
    AC_MSG_RESULT([no])
fi

AC_MSG_CHECKING([if requested to enable assert macros])
if test x"$enable_cassert" = xfalse; then
	AM_CXXFLAGS="$AM_CXXFLAGS -DNDEBUG"
    AC_MSG_RESULT([no])
else
    AC_MSG_RESULT([yes])
fi

AC_MSG_CHECKING([if requested libstdc++ debug mode])
if test x"$enable_glibcxx_debug" = xtrue; then
    AM_CXXFLAGS="$AM_CXXFLAGS -D_GLIBCXX_DEBUG=1 -D_GLIBCXX_DEBUG_PEDANTIC=1"
    AC_MSG_RESULT([yes])
else
    AC_MSG_RESULT([no])
fi

dnl --------------------------------------------------------------------
dnl - Append default C++ and C flags 
dnl --------------------------------------------------------------------

dnl In order for AC_LIBTOOL_COMPILER_OPTION to use
dnl the C compiler we need the hack below.
dnl It is likely a bug in the libtool macro file to
dnl require AC_LIBTOOL_LANG_CXX_CONFIG in *addition*
dnl to AC_LANG(CXX) or AC_LANG_PUSH(CXX)/AC_LANG_POP()
dnl 
AC_LIBTOOL_LANG_CXX_CONFIG

# Set default AM_CXXFLAGS and AM_CFLAGS 
# -pedantic: ISO does not support long long
# we add -Wno-long-long to avoid those messages
WARNFLAGS=""
AC_LIBTOOL_COMPILER_OPTION([if $compiler supports -pedantic], [dummy_cv_pedantic], [-pedantic], [], [WARNFLAGS="$WARNFLAGS -pedantic"], [])
AC_LIBTOOL_COMPILER_OPTION([if $compiler supports -Wall], [dummy_cv_wall], [-Wall], [], [WARNFLAGS="$WARNFLAGS -Wall"], [])
AC_LIBTOOL_COMPILER_OPTION([if $compiler supports -ansi], [dummy_cv_ansi], [-ansi], [], [WARNFLAGS="$WARNFLAGS -ansi"], [])
AC_LIBTOOL_COMPILER_OPTION([if $compiler supports -Wno-long-long], [dummy_cv_no_long_long], [-Wno-long-long], [], [WARNFLAGS="$WARNFLAGS -Wno-long-long"], [])

# To make numerical computation more stable, we use --ffloat-store
NUMERICFLAGS=""
AC_LIBTOOL_COMPILER_OPTION([if $compiler supports -ffloat-store], [dummy_cv_ffloat_store], [-ffloat-store], [], [NUMERICFLAGS="$NUMERICFLAGS -ffloat-store"], [])

DEFAULTFLAGS="${WARNFLAGS} ${NUMERICFLAGS}"

AM_CXXFLAGS="${AM_CXXFLAGS} ${DEFAULTFLAGS}"
AM_CFLAGS="${AM_CFLAGS} ${DEFAULTFLAGS}"
AC_SUBST(AM_CXXFLAGS)
AC_SUBST(AM_CFLAGS)

dnl --------------------------------------------------------------------
dnl - Look for finite and/or isfinite macros/functions
dnl --------------------------------------------------------------------

dnl These two tests need the math library or they won't link
dnl on OpenBSD, even if the functions exist.
save_LIBS=$LIBS
LIBS="$LIBS -lm"
AC_CACHE_CHECK([for finite], ac_cv_finite,
 [AC_TRY_LINK([#include <math.h>],
 [double x; int y; y = finite(x);],
 ac_cv_finite=yes,
 ac_cv_finite=no
)])
if test x"$ac_cv_finite" = x"yes"; then
  AC_DEFINE(HAVE_FINITE, [1], [Has finite])
fi

AC_CACHE_CHECK([for isfinite], ac_cv_isfinite,
 [AC_TRY_LINK([#include <math.h>],
 [double x; int y; y = isfinite(x);],
 ac_cv_isfinite=yes,
 ac_cv_isfinite=no
)])
if test x"$ac_cv_isfinite" = x"yes"; then
  AC_DEFINE(HAVE_ISFINITE, [1], [Has isfinite])
fi

dnl --------------------------------------------------------------------
dnl Test for presence of isnan function when using C++ and <cmath>
dnl This is for a particular bug in OS/X where <cmath> drops the definition
dnl of isnan().

AC_LANG_PUSH([C++])
AC_CACHE_CHECK([for isnan], ac_cv_isnan,
 [AC_TRY_LINK([#include <cmath>],
 [double x; int y; y = isnan(x);],
 ac_cv_isnan=yes,
 ac_cv_isnan=no
)])
if test x"$ac_cv_isnan" = x"yes"; then
  AC_DEFINE(HAVE_ISNAN, [1], [Has isnan])
fi
AC_LANG_POP([C++])

dnl --------------------------------------------------------------------

LIBS=$save_LIBS

dnl --------------------------------------------------------------------
dnl - Look for a 64bit integer (do after CFLAGS is set)
dnl --------------------------------------------------------------------

dnl Find a working 64bit integer
PGAC_TYPE_64BIT_INT([int64_t])
if test x"$HAVE_INT64_T_64" = x"no" ; then
  PGAC_TYPE_64BIT_INT([long long int])
  if test x"$HAVE_LONG_LONG_INT_64" = x"no" ; then
    PGAC_TYPE_64BIT_INT([long int])
    if test x"$HAVE_LONG_INT_64" = x"no" ; then
      AC_MSG_WARN([Could not find a working 64bit int type, you may experience weird bugs (undefined behaviour)]);
    fi
  fi
fi



dnl --------------------------------------------------------------------
dnl - check whether python is required for the build
dnl --------------------------------------------------------------------

AC_ARG_ENABLE([python], [  --enable-python    Enable build of python module],
	[case "${enableval}" in
		yes) use_python=true ;;
		no)  use_python=false ;;
		*) AC_MSG_ERROR(bad value ${enableval} for --enable-python) ;;
	esac],
	[use_python=false]
)


dnl --------------------------------------------------------------------
dnl - check whether ruby is required for the build
dnl --------------------------------------------------------------------

AC_ARG_ENABLE([ruby], [  --enable-ruby    Enable build of ruby module],
	[case "${enableval}" in
		yes) use_ruby=true ;;
		no)  use_ruby=false ;;
		*) AC_MSG_ERROR(bad value ${enableval} for --enable-ruby) ;;
	esac],
	[use_ruby=false]
)

dnl --------------------------------------------------------------------
dnl - check whether php is required for the build
dnl --------------------------------------------------------------------

AC_ARG_ENABLE([php], [  --enable-php    Enable build of php module],
	[case "${enableval}" in
		yes) use_php=true ;;
		no)  use_php=false ;;
		*) AC_MSG_ERROR(bad value ${enableval} for --enable-php) ;;
	esac],
	[use_php=false]
)


dnl --------------------------------------------------------------------
dnl - check for swig if python, ruby or php are enabled
dnl --------------------------------------------------------------------

use_swig=false
if test x"$use_python" = xtrue ||
   test x"$use_ruby" = xtrue; then
  AC_PROG_SWIG(1.3.28)
  if test x"$SWIG" != "x"; then
    SWIG_ENABLE_CXX	
    AC_SUBST(SWIG)           
    use_swig=true 
  else
    use_python=false
    use_ruby=false
  fi
fi
AM_CONDITIONAL(ENABLE_SWIG, [ test x"$use_swig" = xtrue ])


dnl --------------------------------------------------------------------
dnl - check for python if enabled
dnl --------------------------------------------------------------------

if test x"$use_python" = xtrue; then
	dnl Check for Python 
  AM_PATH_PYTHON	
  SWIG_PYTHON

	if test x"$PYTHON" = "x"; then
		use_python=false
	fi
	
	AC_SUBST(PYTHON)
	AC_SUBST(SWIG_PYTHON_CPPFLAGS)
	AC_SUBST(SWIG_PYTHON_OPT)
fi
AM_CONDITIONAL(ENABLE_PYTHON, [test x"$use_python" = xtrue])


dnl --------------------------------------------------------------------
dnl - check for ruby if enabled
dnl --------------------------------------------------------------------

if test x"$use_ruby" = xtrue; then
	dnl Check for Ruby 
	AC_RUBY_DEVEL

	if test x"$RUBY" = x; then
		use_ruby=false
	fi

fi
AM_CONDITIONAL(ENABLE_RUBY, [ test x"$use_ruby" = xtrue ])

dnl --------------------------------------------------------------------
dnl - check for php if enabled
dnl --------------------------------------------------------------------

if test x"$use_php" = xtrue; then
	dnl Check for PHP 
	AC_PATH_PROG(PHP_CONFIG, php-config)

	if test x"$PHP_CONFIG" = x; then
		AC_MSG_WARN([php-config not found, php support disabled])
		use_php=false
	else

		dnl TODO: check for version, we want PHP5 dev files

		AC_PATH_PROG(PHP, php) dnl for unit testing
		AC_PATH_PROG(PHPUNIT, phpunit) dnl for unit testing
	fi


fi
AM_CONDITIONAL(ENABLE_PHP, [ test x"$use_php" = xtrue ])
AM_CONDITIONAL(HAVE_PHP, [ test x"$PHP" != x ])
AM_CONDITIONAL(HAVE_PHP_UNIT, [ test x"$PHPUNIT" != x ])


dnl --------------------------------------------------------------------
dnl - do operating-system specific things
dnl --------------------------------------------------------------------

AC_MSG_CHECKING([OS-specific settings])

case "${host_os}" in
	*darwin*)
		AC_MSG_RESULT([${host_os}])
		AC_MSG_CHECKING([for OS/X version])
		kernel=`uname -r`

		# "Darwin 9.6.0" is Mac OSX 10.5.6
		# "Darwin 10.x" would presumably be Mac OS X 10.6.x
		case "${kernel}" in
			8.*)
				AC_MSG_RESULT([Mac OS X 10.4 Tiger])
				;;
			9.*)
				AC_MSG_RESULT([Mac OS X 10.5 Leopard])
				;;
			10.*)
dnl				AM_CXXFLAGS="$AM_CXXFLAGS -Wnon-virtual-dtor -Woverloaded-virtual"
				AC_MSG_RESULT([Mac OS X 10.6 Snow Leopard])
				;;
			*)
				AC_MSG_RESULT([Mac OS X (Darwin ${kernel} kernel)])
				;;
		esac
		;;
esac

dnl --------------------------------------------------------------------
dnl - check for boost 
dnl --------------------------------------------------------------------

dnl -- AX_BOOST(1.32)
dnl -- if test "x$ax_cv_boost_unit_test_framework" = "xyes"; then
dnl -- 	use_boost_utf=yes
dnl -- else
dnl -- 	use_boost_utf=no
dnl -- fi
dnl -- AM_CONDITIONAL(ENABLE_BOOST_UTF, [test "x$use_boost_utf" = "xyes"])

dnl things to substitute in output ----------------------------------------
AC_SUBST(VERSION)
AC_SUBST(VERSION_MAJOR)
AC_SUBST(VERSION_MINOR)
AC_SUBST(VERSION_PATCH)
AC_SUBST(INTERFACE_CURRENT)
AC_SUBST(INTERFACE_REVISION)
AC_SUBST(INTERFACE_AGE)
AC_SUBST(JTS_PORT)
AC_SUBST(CAPI_VERSION)
AC_SUBST(CAPI_VERSION_MAJOR)
AC_SUBST(CAPI_VERSION_MINOR)
AC_SUBST(CAPI_VERSION_PATCH)
AC_SUBST(CAPI_INTERFACE_CURRENT)
AC_SUBST(CAPI_INTERFACE_REVISION)
AC_SUBST(CAPI_INTERFACE_AGE)

dnl output stuff ----------------------------------------------------------

AC_OUTPUT([
	Makefile 
	capi/Makefile
	capi/geos_c.h
	doc/Doxyfile
	doc/Makefile
	macros/Makefile
	src/Makefile 
	src/algorithm/Makefile
	src/algorithm/locate/Makefile
	src/algorithm/distance/Makefile
	src/geom/Makefile
	src/geom/prep/Makefile
	src/geom/util/Makefile
	src/geomgraph/Makefile
	src/geomgraph/index/Makefile
	include/Makefile
	include/geos/Makefile
	include/geos/algorithm/Makefile
	include/geos/algorithm/locate/Makefile
	include/geos/algorithm/distance/Makefile
	include/geos/geom/Makefile
	include/geos/geom/prep/Makefile
	include/geos/geom/util/Makefile
	include/geos/geomgraph/Makefile
	include/geos/geomgraph/index/Makefile
	include/geos/index/Makefile
	include/geos/index/bintree/Makefile
	include/geos/index/chain/Makefile
	include/geos/index/intervalrtree/Makefile
	include/geos/index/quadtree/Makefile
	include/geos/index/strtree/Makefile
	include/geos/index/sweepline/Makefile
	include/geos/io/Makefile
	include/geos/linearref/Makefile
	include/geos/noding/Makefile
	include/geos/noding/snapround/Makefile
	include/geos/operation/Makefile
	include/geos/operation/buffer/Makefile
	include/geos/operation/distance/Makefile
	include/geos/operation/intersection/Makefile
	include/geos/operation/linemerge/Makefile
	include/geos/operation/overlay/Makefile
	include/geos/operation/overlay/snap/Makefile
	include/geos/operation/polygonize/Makefile
	include/geos/operation/predicate/Makefile
	include/geos/operation/relate/Makefile
	include/geos/operation/sharedpaths/Makefile
	include/geos/operation/union/Makefile
	include/geos/operation/valid/Makefile
	include/geos/planargraph/Makefile
	include/geos/planargraph/algorithm/Makefile
	include/geos/precision/Makefile
	include/geos/simplify/Makefile
	include/geos/triangulate/Makefile
	include/geos/triangulate/quadedge/Makefile
	include/geos/util/Makefile
	include/geos/version.h
	src/index/Makefile
	src/index/bintree/Makefile
	src/index/chain/Makefile
	src/index/intervalrtree/Makefile
	src/index/quadtree/Makefile
	src/index/strtree/Makefile
	src/index/sweepline/Makefile
	src/io/Makefile
	src/linearref/Makefile
	src/noding/Makefile
	src/noding/snapround/Makefile
	src/operation/Makefile
	src/operation/buffer/Makefile
	src/operation/distance/Makefile
	src/operation/intersection/Makefile
	src/operation/linemerge/Makefile
	src/operation/overlay/Makefile
	src/operation/polygonize/Makefile
	src/operation/predicate/Makefile
	src/operation/relate/Makefile
	src/operation/sharedpaths/Makefile
	src/operation/union/Makefile
	src/operation/valid/Makefile
	src/planargraph/Makefile
	src/precision/Makefile
	src/simplify/Makefile
	src/triangulate/Makefile
	src/triangulate/quadedge/Makefile
	src/util/Makefile
	swig/geos.i
	swig/Makefile
	swig/python/Makefile
	swig/python/tests/Makefile
	swig/ruby/Makefile
	swig/ruby/test/Makefile
	php/Makefile
	php/test/Makefile
	tests/Makefile
	tests/bigtest/Makefile
	tests/unit/Makefile
	tests/perf/Makefile
	tests/perf/operation/Makefile
	tests/perf/operation/buffer/Makefile
	tests/perf/operation/predicate/Makefile
	tests/perf/capi/Makefile
	tests/xmltester/Makefile
	tests/geostest/Makefile
	tests/thread/Makefile
	tools/Makefile
	tools/geos-config
	])

dnl -- echo "---------------------------------------"
dnl -- echo "Boost UTF: $use_boost_utf"
echo "Swig: $use_swig"
echo "Python bindings: $use_python"
echo "Ruby bindings: $use_ruby"

echo "PHP bindings: $use_php"
if test x"$use_php" = xtrue; then
 echo " PHP: $PHP"
 echo " PHPUNIT: $PHPUNIT"
 if test x"$PHP" = x -o x"$PHPUNIT" = x; then
   AC_MSG_WARN([PHP Unit testing disabled (missing PHP or PHPUNIT)])
 fi
fi

dnl -- echo "---------------------------------------"
