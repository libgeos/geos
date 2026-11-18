#!/bin/sh

# Common shell functions for post-install tests

main_setup(){
  # usage: main_setup $1 [$2]
  export prefix=$1
  if [ -z "${prefix}" ]; then
    echo "First positional argument to the the installed prefix is required"
    exit 1
  fi
  case $2 in
    "" | shared) export library_type=shared ;;
         static) export library_type=static ;;
    *)
      echo "Second optional argument must be either 'shared' (default) or 'static'"
      exit 1 ;;
  esac
}

test_libpath(){
  # usage: test_libpath ${PROGRAM} ${LIBPATH} ${LIBNAME}
  # use optional 'library_type=static' to pass if match is not found
  _uname_s=$(uname -s)
  case ${_uname_s} in
    Darwin*)
      USE_OTOOL=yes
      EXPECTED_SUBSTR=$2
      ;;
    MINGW* | MSYS*)
      EXPECTED_SUBSTR=$(cygpath -u "$2/$3" | sed 's/\/lib\//\/bin\//')
      ;;
    Linux* | *BSD* )
      EXPECTED_SUBSTR=$2/$3
      ;;
    *)
      echo "test_libpath not set-up for UNAME=${_uname_s}"
      return 77  # skip
      ;;
  esac
  printf "Testing expected libpath output "
  if [ "x${library_type}" = xstatic ]; then
    printf "not "
  fi
  printf "containing '${EXPECTED_SUBSTR}' ... "
  if [ "x${USE_OTOOL}" = xyes ]; then
    CMD_OUTPUT=$(otool -l ./$1 | grep -m1 "$2")
  else
    CMD_OUTPUT=$(ldd ./$1 | grep -m1 "$3")
  fi
  case "${CMD_OUTPUT}" in
    *"not found"*)
      echo "failed: ${CMD_OUTPUT}"
      return 1 ;;
    *${EXPECTED_SUBSTR}*) found=yes ;;
    *) found=no ;;
  esac
  if [ "x${library_type}" = xstatic ]; then
    if [ "x${found}" = "xyes" ] ; then
      echo "failed: ${CMD_OUTPUT}"
      return 1
    fi
  elif [ "x${found}" = "xno" ] ; then
    echo "failed:"
    if [ "x${USE_OTOOL}" = xyes ]; then
      echo "otool -l ./$1:"
      otool -l ./$1
    else
      echo "ldd ./$1:"
      ldd ./$1
    fi
    return 1
  fi
  echo "passed"
  return 0
}

test_length(){
  # usage: test_length ${PROGRAM_LENGTH}
  printf "Testing length ... "
  EXPECTED="42.0"
  case "$1" in
    "${EXPECTED}")
      echo "passed"
      return 0 ;;
    *)
      echo "failed, expected ${EXPECTED}, found $1"
      return 1 ;;
  esac
}

test_version(){
  # usage: test_version ${PROGRAM_VERSION} ${EXPECTED_VERSION}
  printf "Testing program version $1 ... "
  if [ -z $2 ]; then
    echo "failed (empty expected)"
    return 1
  fi
  case $1 in
    $2*)
      echo "passed"
      return 0 ;;
    *)
      echo "failed, expected $2"
      return 1 ;;
  esac
}
