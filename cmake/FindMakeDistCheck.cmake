# Inspired by CMake Distcheck for LAAS-CNRS
#
# DEFINE_DISTCHECK
# ---------------
#
# Add a distcheck target to check the generated tarball.
#
# This step calls `make dist' to generate a copy of the sources as it
# stands in the current git HEAD i.e., unversioned files are skipped.
#
# Then:
# - create _build and _inst to respectively create a build
#   and an installation directory.
# - run cmake with _inst as the installation prefix
# - run make, make check, make install and make uninstall
# - remove _build and _inst.
# - remove dist directory and confirm success.
#
# During the compilation phase, all files in the source tree are modified
# to *not* be writeable to detect bad compilation steps which tries to modify
# the source tree. Permissions are reverted at the end of the check.
#
macro(AddMakeDistCheck)
  find_program(_tar tar)
  find_program(_bzip2 bzip2)
  string(TOLOWER "${CPACK_SOURCE_PACKAGE_FILE_NAME}" _distbasename)
  set(_distname "${_distbasename}.tar.bz2")
  set(_builddir "${CMAKE_BINARY_DIR}/${_distbasename}/_build")
  set(_instdir  "${CMAKE_BINARY_DIR}/${_distbasename}/_inst")
  # message(STATUS "GEOS: _distbasename: ${_distbasename}")
  # message(STATUS "GEOS: _distname: ${_distname}")
  # message(STATUS "GEOS: _builddir: ${_builddir}")
  # message(STATUS "GEOS: _instdir: ${_instdir}")

  add_custom_target(distcheck
    COMMAND rm -rf "${_distbasename}"
      && \(${_bzip2} -d -c "${_distname}" | ${_tar} xf -\)
      && mkdir -vp "${_builddir}"
      && mkdir -vp "${_instdir}"
      && cd "${_builddir}"
      && ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX:PATH="${_instdir}" ..
        || \(echo "ERROR: the cmake configuration failed." && false\)
      && make
        || \(echo "ERROR: the compilation failed." && false\)
      && make test
        || \(echo "ERROR: the test suite failed." && false\)
      && make install
        || \(echo "ERROR: the install target failed." && false\)
      && make uninstall
        || \(echo "ERROR: the uninstall target failed." && false\)
      && make clean
        || \(echo "ERROR: the clean target failed." && false\)
      && cd ../..
      && rm -rf "${_distbasename}"
      && echo "=============================================================="
      && echo "${_distname} is ready for distribution."
      && echo "=============================================================="
    )

  add_dependencies(distcheck dist)
endmacro()
