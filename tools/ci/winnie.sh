#!/bin/sh
#
# Windows mingw64 CI script runner for GEOS
# 64-bit Windows 2012 (winnie)
#
# Copyright (c) 2021 Regina Obe <lr@pcorp.us>
#
# This is free software; you can redistribute and/or modify it under
# the terms of the GNU Lesser General Public Licence as published
# by the Free Software Foundation.
# See the COPYING file for more information.
export PROJECTS=/projects
cd ${WORKSPACE}
if [[ "$GCC_TYPE" == "gcc45" ]] ; then #gcc 45 has no extension
  export GCC_TYPE=
fi;
## end variables passed in by jenkins

if [ "$OS_BUILD" == "64" ] ; then
	export MINGHOST=x86_64-w64-mingw32
else
	export MINGHOST=i686-w64-mingw32
fi;
CMAKE_PATH=/cmake

if [[ "${GEOS_VER}"  == *SVN* || "${GEOS_VER}"  == *dev* ]] ; then
  export SRC_DIR=${WORKSPACE}
else
  #tagged version -- official release
  export SRC_DIR=${WORKSPACE}
fi;

export PATH="${PATH}:${CMAKE_PATH}/bin:/bin:/include"

cd ../
rm -rf build${OS_BUILD}${MAKE_TYPE}${GEOS_VER}
mkdir -p build${OS_BUILD}${MAKE_TYPE}${GEOS_VER}
cd build${OS_BUILD}${MAKE_TYPE}${GEOS_VER}
export GEOS_ENABLE_INLINE=ON #for older geos

if [[ "${OS_BUILD}" == "64" ]] ; then
  #cmake -G "MSYS Makefiles" DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX:PATH=${PROJECTS}/geos/rel-${GEOS_VER}w${OS_BUILD}${GCC_TYPE} -DHAVE_STD_ISNAN=1 -DHAVE_LONG_LONG_INT_64=1    -  ../${CMAKE_GEOS_VER}
  cmake -G "MSYS Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX:PATH=${PROJECTS}/geos/rel-${GEOS_VER}w${OS_BUILD}${GCC_TYPE} -DHAVE_STD_ISNAN=1 -DHAVE_LONG_LONG_INT_64=1 ${SRC_DIR}
else
  #cmake -G "MSYS Makefiles" DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX:PATH=${PROJECTS}/geos/rel-${GEOS_VER}w${OS_BUILD}${GCC_TYPE} -DHAVE_STD_ISNAN=1  -  ../${CMAKE_GEOS_VER}
  cmake -G "MSYS Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX:PATH=${PROJECTS}/geos/rel-${GEOS_VER}w${OS_BUILD}${GCC_TYPE} -DHAVE_STD_ISNAN=1 ${SRC_DIR}
fi
make && make install
#make check
ctest --output-on-failure
