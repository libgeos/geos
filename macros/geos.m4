dnl 
dnl GEOS_INIT (MINIMUM_VERSION)
dnl
dnl Test for GEOS: define HAVE_GEOS, GEOS_LIBS, GEOS_C_LIBS, GEOS_CFLAGS, GEOS_VERSION
dnl 
dnl Call as GEOS_INIT or GEOS_INIT(minimum version) in configure.in. Test
dnl HAVE_GEOS (yes|no) afterwards. If yes, all other vars above can be 
dnl used in program.
dnl
dnl UPDATE 2006/07/06
dnl Now, --with-geos takes values: yes or no, or full path to the geos-config
dnl utility. Note that the utility can have different name,
dnl e.g. geos-config-cvs.
dnl Example: --with-geos=/home/foo/usr/bin/geos-config-cvs
dnl
AC_DEFUN([GEOS_INIT],[
	AC_SUBST(GEOS_LIBS)
	AC_SUBST(GEOS_CFLAGS)
	AC_SUBST(GEOS_C_LIBS)
	AC_SUBST(GEOS_C_CFLAGS)
	AC_SUBST(HAVE_GEOS) 
	AC_SUBST(GEOS_VERSION)

    if test x"$with_geos" = x"no" ; then
        AC_MSG_RESULT([GEOS support disabled])
        GEOS_CONFIG="no"
        HAVE_GEOS="no"	
    elif test x"$with_geos" = x"yes" -o x"$with_geos" = x"" ; then
        AC_PATH_PROG([GEOS_CONFIG], [geos-config], [no])
    else
        ac_geos_config=`basename $with_geos`
        ac_geos_config_dir=`dirname $with_geos`

        AC_CHECK_PROG(
            GEOS_CONFIG,
            $ac_geos_config,
            $with_geos,
            [no],
            [$ac_geos_config_dir],
            []
        )
    fi

	min_geos_version=ifelse([$1], ,0.0.1,$1)

    if test "$GEOS_CONFIG" = "no"; then
      no_geos_config="yes"
    else
       AC_MSG_CHECKING(for GEOS version >= $min_geos_version)

       geos_major_version=`$GEOS_CONFIG --version | \
          sed 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\).*/\1/'`
       geos_minor_version=`$GEOS_CONFIG --version | \
          sed 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\).*/\2/'`
       geos_micro_version=`$GEOS_CONFIG --version | \
          sed 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\).*/\3/'`

    req_major=`echo $min_geos_version | \
       sed 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\).*/\1/'`
    req_minor=`echo $min_geos_version | \
       sed 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\).*/\2/'`
    req_micro=`echo $min_geos_version | \
       sed 's/\([[0-9]]*\)\.\([[0-9]]*\)\.\([[0-9]]*\).*/\3/'`
    
    version_ok="no"
    ac_req_version=`expr $req_major \* 100000 \+  $req_minor \* 100 \+ $req_micro`
    ac_geos_version=`expr $geos_major_version \* 100000 \+  $geos_minor_version \* 100 \+ $geos_micro_version`

    if test $ac_req_version -le $ac_geos_version; then
        version_ok="yes"
    fi

      if test $version_ok = "no"; then
         HAVE_GEOS="no"	
      else
        if $GEOS_CONFIG --libs >/dev/null 2>&1; then
          AC_MSG_RESULT(yes)
          HAVE_GEOS="yes"
          GEOS_LIBS="`$GEOS_CONFIG --libs`"
          GEOS_C_LIBS="`$GEOS_CONFIG --ldflags` -lgeos_c"
          GEOS_CFLAGS="`$GEOS_CONFIG --cflags`"
          GEOS_VERSION="`$GEOS_CONFIG --version`"
        else
          AC_MSG_RESULT(no)
          HAVE_GEOS="no"
          no_geos_config="yes"
        fi
    fi
fi
])

