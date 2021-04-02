#!/bin/sh

# Common shell functions for post-install tests

check_ldd(){
  printf "Testing expected ldd output containing '${EXPECTED_LDD_SUBSTR}' ... "
  if [ -z ${EXPECTED_LDD_SUBSTR+x} ]; then
    echo "EXPECTED_LDD_SUBSTR is not set"
    return 1
  fi
  LDD_OUTPUT=$(ldd ./$1 | grep libgeos)
  case "${LDD_OUTPUT}" in
    *${EXPECTED_LDD_SUBSTR}*)
      echo "passed"
      return 0 ;;
    *)
      echo "failed:"
      ldd ./$1
      return 1 ;;
  esac
}


check_version(){
  printf "Testing expected version ${EXPECTED_GEOS_VERSION} ... "
  # GEOS_VERSION=$(geos-config --version)
  if [ -z ${EXPECTED_GEOS_VERSION+x} ]; then
    echo "EXPECTED_GEOS_VERSION is not set"
    return 1
  fi
  VERSION_OUTPUT=$(./$1)
  case "${VERSION_OUTPUT}" in
    ${EXPECTED_GEOS_VERSION}*)
      echo "passed"
      return 0 ;;
    *)
      echo "failed: '${VERSION_OUTPUT}'"
      return 1 ;;
  esac
}
