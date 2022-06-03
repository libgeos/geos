dnl
dnl configure.in - autoconf input template to produce ./configure script
dnl
dnl version 2.52 is required for Cygwin libtool support
AC_PREREQ([2.63])

dnl local vars to hold user's preferences --------------------------------
AC_INIT([include/geos.h])
AC_CANONICAL_SYSTEM

AC_CONFIG_MACRO_DIR([macros])

dnl
dnl -- Release versions / C++ library SONAME will use these
dnl -- encoding ABI break at every release
dnl
VERTXT=${srcdir}/Version.txt
VERSION_MAJOR=`cat $VERTXT | awk -F= '/^GEOS_VERSION_MAJOR/ {print $2}'`
VERSION_MINOR=`cat $VERTXT | awk -F= '/^GEOS_VERSION_MINOR/ {print $2}'`
VERSION_PATCH=`cat $VERTXT | awk -F= '/^GEOS_VERSION_PATCH/ {print $2}'`
PATCH_WORD=`cat $VERTXT    | awk -F= '/^GEOS_PATCH_WORD/ {print $2}'`

VERSION_PATCH=${VERSION_PATCH}${PATCH_WORD}
VERSION="$VERSION_MAJOR.$VERSION_MINOR.$VERSION_PATCH"
VERSION_RELEASE=`echo "$VERSION" | sed -E 's/^([[0-9]+\.[0-9]+\.[0-9]+]).*$/\1/'`

AC_MSG_CHECKING([for GEOS version])
AC_MSG_RESULT(["$VERSION"])

CAPI_INTERFACE_CURRENT=`cat $VERTXT  | awk -F= '/^CAPI_INTERFACE_CURRENT/ {print $2}'`
CAPI_INTERFACE_REVISION=`cat $VERTXT | awk -F= '/^CAPI_INTERFACE_REVISION/ {print $2}'`
CAPI_INTERFACE_AGE=`cat $VERTXT      | awk -F= '/^CAPI_INTERFACE_AGE/ {print $2}'`

dnl CAPI_VERSION_MAJOR=$(($CAPI_INTERFACE_CURRENT-$CAPI_INTERFACE_AGE))
dnl the following should be more portable
CAPI_VERSION_MAJOR=`expr $CAPI_INTERFACE_CURRENT - $CAPI_INTERFACE_AGE`
CAPI_VERSION_MINOR=$CAPI_INTERFACE_AGE
CAPI_VERSION_PATCH=$CAPI_INTERFACE_REVISION
CAPI_VERSION="$CAPI_VERSION_MAJOR.$CAPI_VERSION_MINOR.$CAPI_VERSION_PATCH"

AC_MSG_CHECKING([for GEOS CAPI version])
AC_MSG_RESULT(["$CAPI_VERSION"])

AM_INIT_AUTOMAKE([geos], [$VERSION], [no-define])
AM_MAINTAINER_MODE
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES])
AC_CONFIG_HEADERS([include/config.h])
AC_PROG_CC

AC_CONFIG_LINKS([tools/astyle.sh:tools/astyle.sh])

dnl -- JTS_PORT: the version of JTS this release is bound to
dnl JTS_PORT=1.13.0
JTS_PORT=`cat $VERTXT | awk -F= '/^JTS_PORT/ {print $2}'`

DOXYGEN_LOGFILE=
AC_SUBST(DOXYGEN_LOGFILE)

dnl Hush warnings
AC_DEFINE(USE_UNSTABLE_GEOS_CPP_API, [1], [We know])

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


AC_ARG_ENABLE([inline],
	[  --disable-inline        Disable inlining],
	[case "${enableval}" in
		yes) enable_inline=true ;;
		no)  enable_inline=false ;;
		*) AC_MSG_ERROR(bad value ${enableval} for --enable-inline);;
	esac],
	[enable_inline=true]
)

AC_ARG_ENABLE([cassert],
	[  --disable-cassert       Disable assertion checking],
	[case "${enableval}" in
		yes) enable_cassert=true ;;
		no)  enable_cassert=false ;;
		*) AC_MSG_ERROR(bad value ${enableval} for --enable-cassert);;
	esac],
	[enable_cassert=true]
)

AC_ARG_ENABLE([glibcxx-debug],
	[  --enable-glibcxx-debug  Enable libstdc++ debug mode],
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
dnl - check whether user has requested overlayng
dnl --------------------------------------------------------------------
dnl
dnl AC_ARG_ENABLE([overlayng],
dnl 	[  --disable-overlayng     Disable use of new overlay],
dnl 	[case "${enableval}" in
dnl 		yes) use_overlayng=true ;;
dnl 		no)  use_overlayng=false ;;
dnl 		*) AC_MSG_ERROR(bad value ${enableval} for --disable-overlayng) ;;
dnl 	esac],
dnl 	[use_overlayng=true]
dnl )
dnl
dnl if test x"$use_overlayng" = xtrue; then
dnl OVERLAYNG_FLAGS=""
dnl else
dnl 	OVERLAYNG_FLAGS="-DDISABLE_OVERLAYNG"
dnl fi

dnl --------------------------------------------------------------------
dnl - Append default C++ and C flags
dnl --------------------------------------------------------------------
AC_PROG_CXX
AX_CXX_COMPILE_STDCXX(11, noext, mandatory)

WARNFLAGS=""
AC_LANG_PUSH([C++])
dnl Available in GCC 5.1
AX_CHECK_COMPILE_FLAG([-Wsuggest-override -Werror], [WARNFLAGS="$WARNFLAGS -Wsuggest-override"])
dnl Available in clang 3.5
AX_CHECK_COMPILE_FLAG([-Wmissing-override -Werror], [WARNFLAGS="$WARNFLAGS -Wmissing-override"])
AC_LANG_POP([C++])

dnl In order for AC_LIBTOOL_COMPILER_OPTION to use
dnl the C compiler we need the hack below.
dnl It is likely a bug in the libtool macro file to
dnl require AC_LIBTOOL_LANG_CXX_CONFIG in *addition*
dnl to AC_LANG(CXX) or AC_LANG_PUSH(CXX)/AC_LANG_POP()
dnl
AC_LIBTOOL_LANG_CXX_CONFIG

dnl Set default AM_CXXFLAGS and AM_CFLAGS
dnl -pedantic: ISO does not support long long
dnl we add -Wno-long-long to avoid those messages
AC_LIBTOOL_COMPILER_OPTION([if $compiler supports -pedantic], [dummy_cv_pedantic], [-pedantic], [], [WARNFLAGS="$WARNFLAGS -pedantic"], [])
AC_LIBTOOL_COMPILER_OPTION([if $compiler supports -Wall], [dummy_cv_wall], [-Wall], [], [WARNFLAGS="$WARNFLAGS -Wall"], [])
AC_LIBTOOL_COMPILER_OPTION([if $compiler supports -Wno-long-long], [dummy_cv_wno_long_long], [-Wno-long-long], [], [WARNFLAGS="$WARNFLAGS -Wno-long-long"], [])

dnl -----------------------------------------------------------------------------
dnl To make numerical computation more stable, we use --ffloat-store
dnl on 32-bit platforms
NUMERICFLAGS=""
dnl 32-bit platforms have a 4-byte pointer
AC_CHECK_SIZEOF([void *])
if test $ac_cv_sizeof_void_p -eq 4 ; then
  AC_LIBTOOL_COMPILER_OPTION([if $compiler supports -ffloat-store], [dummy_cv_ffloat_store], [-ffloat-store], [], [NUMERICFLAGS="$NUMERICFLAGS -ffloat-store"], [])
fi

dnl -----------------------------------------------------------------------------
dnl Add flags to prevent 'fused multiply-add' operations on targets (ARM64)
dnl that allow it, as it breaks calculations in math/DD.cpp.
dnl TODO: Replace DD calculations with 'long float' where target supports
dnl true long float, and remove other cases where FMA causes regression
dnl failures.
dnl     https://gcc.gnu.org/bugzilla/show_bug.cgi?id=98207
NUMERICFLAGS="$NUMERICFLAGS -ffp-contract=off"

dnl -----------------------------------------------------------------------------
HUSHWARNING="-DUSE_UNSTABLE_GEOS_CPP_API"
DEFAULTFLAGS="${WARNFLAGS} ${NUMERICFLAGS} ${HUSHWARNING}"

AM_CXXFLAGS="${AM_CXXFLAGS} ${DEFAULTFLAGS}"
AM_CFLAGS="${AM_CFLAGS} ${DEFAULTFLAGS}"
AC_SUBST(AM_CXXFLAGS)
AC_SUBST(AM_CFLAGS)

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


dnl things to substitute in output ----------------------------------------
AC_SUBST(VERSION)
AC_SUBST(VERSION_MAJOR)
AC_SUBST(VERSION_MINOR)
AC_SUBST(VERSION_PATCH)
AC_SUBST(VERSION_RELEASE)
AC_SUBST(JTS_PORT)
AC_SUBST(CAPI_VERSION)
AC_SUBST(CAPI_VERSION_MAJOR)
AC_SUBST(CAPI_VERSION_MINOR)
AC_SUBST(CAPI_VERSION_PATCH)
AC_SUBST(CAPI_INTERFACE_CURRENT)
AC_SUBST(CAPI_INTERFACE_REVISION)
AC_SUBST(CAPI_INTERFACE_AGE)

dnl pkg-config file -------------------------------------------------------
AC_CONFIG_FILES([tools/geos.pc])

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
	src/algorithm/construct/Makefile
	src/algorithm/distance/Makefile
	src/algorithm/locate/Makefile
	src/edgegraph/Makefile
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
	include/geos/algorithm/construct/Makefile
	include/geos/edgegraph/Makefile
	include/geos/geom/Makefile
	include/geos/geom/prep/Makefile
	include/geos/geom/util/Makefile
	include/geos/geomgraph/Makefile
	include/geos/geomgraph/index/Makefile
	include/geos/index/Makefile
	include/geos/index/bintree/Makefile
	include/geos/index/kdtree/Makefile
	include/geos/index/chain/Makefile
	include/geos/index/intervalrtree/Makefile
	include/geos/index/quadtree/Makefile
	include/geos/index/strtree/Makefile
	include/geos/index/sweepline/Makefile
	include/geos/io/Makefile
	include/geos/linearref/Makefile
	include/geos/math/Makefile
	include/geos/noding/Makefile
	include/geos/noding/snap/Makefile
	include/geos/noding/snapround/Makefile
	include/geos/operation/Makefile
	include/geos/operation/buffer/Makefile
	include/geos/operation/distance/Makefile
	include/geos/operation/intersection/Makefile
	include/geos/operation/linemerge/Makefile
	include/geos/operation/overlay/Makefile
	include/geos/operation/overlay/snap/Makefile
	include/geos/operation/overlayng/Makefile
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
	include/geos/shape/Makefile
	include/geos/shape/fractal/Makefile
	include/geos/util/Makefile
	include/geos/version.h
	src/index/Makefile
	src/index/bintree/Makefile
	src/index/kdtree/Makefile
	src/index/chain/Makefile
	src/index/intervalrtree/Makefile
	src/index/quadtree/Makefile
	src/index/strtree/Makefile
	src/index/sweepline/Makefile
	src/io/Makefile
	src/linearref/Makefile
	src/math/Makefile
	src/noding/Makefile
	src/noding/snap/Makefile
	src/noding/snapround/Makefile
	src/operation/Makefile
	src/operation/buffer/Makefile
	src/operation/distance/Makefile
	src/operation/intersection/Makefile
	src/operation/linemerge/Makefile
	src/operation/overlay/Makefile
	src/operation/overlayng/Makefile
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
	src/shape/Makefile
	src/shape/fractal/Makefile
	src/util/Makefile
	tests/Makefile
	tests/bigtest/Makefile
	tests/unit/Makefile
	benchmarks/Makefile
	benchmarks/algorithm/Makefile
	benchmarks/operation/Makefile
	benchmarks/operation/buffer/Makefile
	benchmarks/operation/predicate/Makefile
	benchmarks/capi/Makefile
	tests/xmltester/Makefile
	tests/geostest/Makefile
	tests/thread/Makefile
	tools/Makefile
	tools/geos-config
	tools/astyle/Makefile
	util/Makefile
	util/geosop/Makefile
	])

dnl AC_MSG_RESULT([ ------------ GEOS Build ------------ ])
dnl AC_MSG_RESULT([  Version:      ${VERSION}])
dnl AC_MSG_RESULT([  Inlining:     ${enable_inline}])
dnl AC_MSG_RESULT([ ------------------------------------ ])


