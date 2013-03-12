## ------------------------       
## Ruby file handling
## From Charlie Savage
## ------------------------
# Copyright (C) 1999, 2000, 2001, 2002, 2003, 2004, 2005
# Free Software Foundation, Inc.
#
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.


AC_DEFUN([AC_RUBY_DEVEL],
[
	dnl Find a Ruby interpreter	
  AC_PATH_PROG([RUBY],[ruby])
  
  if test -z "$RUBY" ; then
		AC_MSG_ERROR([Could not find ruby interpreter.])
    RUBY=''
  else
		dnl Get Ruby version number
		RUBY_VERSION=`$RUBY -e "puts RUBY_VERSION"`

		dnl Get Ruby bin directory
		RUBY_BIN_DIR=`$RUBY -rrbconfig -e 'puts RbConfig::CONFIG[["bindir"]] || Config::CONFIG[["bindir"]]'`

    dnl Get Ruby site arch
		RUBY_SITE_ARCH=`$RUBY -rrbconfig -e 'puts RbConfig::CONFIG[["sitearch"]] || Config::CONFIG[["sitearch"]]'`

		dnl Get Ruby include directory
		RUBY_INCLUDE_DIR=`$RUBY -rrbconfig -e 'puts RbConfig::CONFIG[["rubyhdrdir"]] || Config::CONFIG[["archdir"]]'`
	
		dnl Get Ruby lib directory
		RUBY_LIB_DIR=`$RUBY -rrbconfig -e 'puts RbConfig::CONFIG[["libdir"]] || Config::CONFIG[["libdir"]]'`

		dnl Get Ruby extensions directory
		RUBY_EXTENSION_DIR=`$RUBY -rrbconfig -e 'puts RbConfig::CONFIG[["sitearchdir"]] || Config::CONFIG[["sitearchdir"]]'`

		dnl Get Ruby shared library name, this does not include the lib prefix or extension name
		RUBY_SO_NAME=`$RUBY -rrbconfig -e 'puts RbConfig::CONFIG[["LIBRUBY_SO"]] || Config::CONFIG[["RUBY_SO_NAME"]]'`
		
		dnl Get Ruby shared libary name
		RUBY_SHARED_LIB=`$RUBY -rrbconfig -e 'puts RbConfig::CONFIG[["LIBRUBY"]] || Config::CONFIG[["LIBRUBY"]]'`
		
    AC_MSG_NOTICE([Ruby executable is '$RUBY'])
    AC_MSG_NOTICE([Ruby version is '$RUBY_VERSION'])
    AC_MSG_NOTICE([Ruby bin directory is '$RUBY_BIN_DIR'])
    AC_MSG_NOTICE([Ruby site arch is '$RUBY_SITE_ARCH'])
    AC_MSG_NOTICE([Ruby include directory is '$RUBY_INCLUDE_DIR'])
    AC_MSG_NOTICE([Ruby library directory is '$RUBY_LIB_DIR'])
    AC_MSG_NOTICE([Ruby extension directory is '$RUBY_EXTENSION_DIR'])
    AC_MSG_NOTICE([Ruby library is '$RUBY_SO_NAME'])
    AC_MSG_NOTICE([Ruby import library is '$RUBY_SHARED_LIB'])
   
    AC_SUBST([RUBY_VERSION])
    AC_SUBST([RUBY_BIN_DIR])
    AC_SUBST([RUBY_SITE_ARCH])
    AC_SUBST([RUBY_INCLUDE_DIR])
    AC_SUBST([RUBY_LIB_DIR])
    AC_SUBST([RUBY_EXTENSION_DIR])
    AC_SUBST([RUBY_SO_NAME])
    AC_SUBST([RUBY_SHARED_LIB])
	fi
  AC_SUBST([RUBY])
])

