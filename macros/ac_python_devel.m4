AC_DEFUN([AC_PYTHON_DEVEL],[
        #
        # should allow for checking of python version here...
        #
        AC_REQUIRE([AM_PATH_PYTHON])

        base_python_path=`echo $PYTHON | sed "s,/bin.*$,,"`
				
        # Need to fix up a couple of problems on Windows/MingW:
        # 1.  python lib is named with MAJOR.VERSION collapsed in a single value,
        #     so libpython2.4 versus libpython24
        #	2.  Directory names have the same issue as item 1 so
        #			python2.4 versus python24
        # 3.  Change paths from c:\python\include to c:/python/include.

        if test $am_cv_python_platform = "win32" ; then
                # Fix python path
                base_python_path=`echo $PYTHON | sed "s,/[[^/]]*$,,"`

                PYTHON_VERSION=`echo $PYTHON_VERSION | sed "s/\.//"`
                AC_SUBST([PYTHON_VERSION], [$PYTHON_VERSION])
                
                pythondir=`echo $pythondir | sed 's,\\\,/,g'`
                AC_SUBST([pkgpythondir], [\${pythondir}/$PACKAGE])

                pyexecdir=`echo $pyexecdir | sed 's,\\\,/,g'`
                AC_SUBST([pkgpyexecdir], [\${pyexecdir}/$PACKAGE])
        fi

        # Check for Python include path
        AC_MSG_CHECKING([for Python include path])
        for i in "$base_python_path/include/python$PYTHON_VERSION/" "$base_python_path/include/python/" "$base_python_path/include/" "$base_python_path/" ; do
                python_path=`find $i -type f -name Python.h -print 2> /dev/null | sed "1q"`
                if test -n "$python_path" ; then
                        break
                fi
        done
        python_path=`echo $python_path | sed "s,/Python.h$,,"`
        AC_MSG_RESULT([$python_path])
        if test -z "$python_path" ; then
                AC_MSG_ERROR([cannot find Python include path])
        fi
        AC_SUBST([PYTHON_CPPFLAGS],[-I$python_path])

        # Check for Python library path
        AC_MSG_CHECKING([for Python library path])
        for i in "$base_python_path/lib/python$PYTHON_VERSION/config/" "$base_python_path/lib/python$PYTHON_VERSION/" "$base_python_path/lib/python/config/" "$base_python_path/lib/python/" "$base_python_path/" "$base_python_path/libs/" ; do
                python_path=`find $i -name libpython$PYTHON_VERSION.* -print 2> /dev/null | sed "1q"`
                if test -n "$python_path" ; then
                        break
                fi
        done
        python_path=`echo $python_path | sed "s,/libpython.*$,,"`
        AC_MSG_RESULT([$python_path])
        if test -z "$python_path" ; then
                AC_MSG_ERROR([cannot find Python library path])
        fi
        AC_SUBST([PYTHON_LDFLAGS],["-L$python_path -lpython$PYTHON_VERSION"])
        #
        python_site=`echo $base_python_path | sed "s/config/site-packages/"`
        AC_SUBST([PYTHON_SITE_PKG],[$python_site])
        #
        # libraries which must be linked in when embedding
        #
        AC_MSG_CHECKING(python extra libraries)
        PYTHON_EXTRA_LIBS=`$PYTHON -c "import distutils.sysconfig; \
                conf = distutils.sysconfig.get_config_var; \
                print (conf('LOCALMODLIBS') or '')+' '+(conf('LIBS') or '')"
        AC_MSG_RESULT($PYTHON_EXTRA_LIBS)`
        AC_SUBST(PYTHON_EXTRA_LIBS)
])

