#!/bin/sh

LC_ALL="C" # what for ?

[ -z "$top_srcdir" ] && top_srcdir="."
rev_file=$top_srcdir'/geos_svn_revision.h'

read_rev() {

    if test -d $top_srcdir"/.svn"; then
      read_rev_svn
    elif test -d $top_srcdir"/.git"; then
      read_rev_git
    else
      echo "Can't fetch local revision (neither .svn nor .git found)" >&2
      echo 0
    fi
}

read_rev_git() {

  # TODO: test on old systems, I think I saw some `which`
  #       implementations returning "nothing found" or something
  #       like that, making the later if ( ! $svn_exe ) always false
  #
  git_exe=`which git`;
  if test -z "$git_exe"; then
    echo "Can't fetch SVN revision: no git executable found" >&2
    echo 0;
  fi

  rev=`${git_exe} log --grep=git-svn -1 | grep git-svn | cut -d@ -f2 | cut -d' ' -f1`

  if test -z "$rev"; then
    echo "Can't fetch SVN revision from git log" >&2 
    echo 0
  else
    echo $rev
  fi
}

read_rev_svn() {

  # TODO: test on old systems, I think I saw some `which`
  #       implementations returning "nothing found" or something
  #       like that, making the later if ( ! $svn_exe ) always false
  #
  svn_exe=`which svn`
  if test -z "$svn_exe"; then
    echo "Can't fetch SVN revision: no svn executable found" >&2
    echo 0;
  fi

  svn_info=`"${svn_exe}" info | grep 'Last Changed Rev:' | cut -d: -f2`

  if test -z "$svn_info"; then
    echo "Can't fetch SVN revision with `svn info`" >&2
    echo 0
  else
    echo ${svn_info}
  fi
}

write_defn() {
  rev=$1
  oldrev=0

  # Do not override the file if new detected
  # revision isn't zero nor different from the existing one
  if test -f $rev_file; then
    oldrev=`grep GEOS_SVN_REVISION ${rev_file} | awk '{print $2}'`
    if test "$rev" = 0 -o "$rev" = "$oldrev"; then
      echo "Not updating existing rev file at $oldrev" >&2
      return;
    fi
  fi

  echo "#define GEOS_SVN_REVISION $rev" | tee $rev_file
  echo "Wrote rev '$rev' in file '$rev_file'" >&2
}

# Read the svn revision number
svn_rev=`read_rev`

# Write it 
write_defn $svn_rev
