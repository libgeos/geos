dnl
dnl GEOS_INIT (MINIMUM_VERSION)
dnl
dnl Test for GEOS: define HAVE_GEOS, GEOS_LIBS, GEOS_CFLAGS, GEOS_VERSION
dnl 
dnl Call as GEOS_INIT or GEOS_INIT(minimum version) in configure.in. Test
dnl HAVE_GEOS (yes|no) afterwards. If yes, all other vars above can be 
dnl used in program.
dnl
AC_DEFUN([GEOS_INIT],[
	AC_SUBST(GEOS_LIBS)
	AC_SUBST(GEOS_CFLAGS)
	AC_SUBST(HAVE_GEOS) 
	AC_SUBST(GEOS_VERSION)

	min_geos_version=ifelse([$1], ,0.0.1,$1)

	AC_PATH_PROG(GEOS_CONFIG,geos-config,no)
	    if test "$GEOS_CONFIG" = "no"; then
	      no_geos-config="yes"
	    else
	      AC_MSG_CHECKING(for GEOS version >= $min_geos_version)

              geos_major_version=`$GEOS_CONFIG --version | \
                 sed 's/.* \([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
              geos_minor_version=`$GEOS_CONFIG --version | \
                 sed 's/.* \([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
              geos_micro_version=`$GEOS_CONFIG --version | \
                 sed 's/.* \([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`

	      req_major=`echo $min_geos_version | \
	          sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
	      req_minor=`echo $min_geos_version | \
                 sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
	      req_micro=`echo $min_geos_version | \
                 sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
		

              version_ok="no"
	      if test $req_major -le $geos_major_version; then
                 if test $req_minor -le $geos_minor_version; then
                    if test $req_micro -le $geos_micro_version; then
                       version_ok="yes"
                    fi
                 fi
              fi

	      if test $version_ok = "no"; then
	         HAVE_GEOS="no"	
              else


	        if $GEOS_CONFIG --libdir >/dev/null 2>&1; then
	          AC_MSG_RESULT(yes)
	  	  HAVE_GEOS="yes"
	          GEOS_LIBS="-L`$GEOS_CONFIG --libs` -lpq"
	          GEOS_CFLAGS="-I`$GEOS_CONFIG --cflags`"
	          GEOS_VERSION="`$GEOS_CONFIG --version`"
	        else
	          AC_MSG_RESULT(no)
		  HAVE_GEOS="no"
	          no_geos-config="yes"
                fi
            fi
	fi
])

