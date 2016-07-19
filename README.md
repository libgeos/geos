GEOS -- Geometry Engine, Open Source
====================================

Project homepage: http://geos.osgeo.org/

## Build status

| Debbie | Travis CI | GitLab CI | AppVeyor | Drone | OSGeo |
|:--- |:--- |:--- |:--- |:--- |:--- |
| [![trunk](https://debbie.postgis.net/buildStatus/icon?job=GEOS_Trunk)](https://debbie.postgis.net/view/GEOS/job/GEOS_Trunk/) | [![trunk](https://secure.travis-ci.org/libgeos/libgeos.png)](https://travis-ci.org/libgeos/libgeos) | [![trunk](https://gitlab.com/geos/libgeos/badges/svn-trunk/build.svg)](https://gitlab.com/geos/libgeos/commits/svn-trunk) | [![trunk](https://ci.appveyor.com/api/projects/status/c4b47oa8k50qyqo5/branch/svn-trunk?svg=true)](https://ci.appveyor.com/project/mloskot/libgeos/branch/svn-trunk) | [![master](https://drone.io/github.com/libgeos/libgeos/status.png)](https://drone.io/github.com/libgeos/libgeos/latest) | [![status](https://drone.osgeo.kbt.io/api/badges/geos/geos/status.svg?branch=svn-trunk)](https://drone.osgeo.kbt.io/geos/geos?branch=svn-trunk, alt=status) |

More on: https://trac.osgeo.org/geos#BuildandInstall

## Building, testing, installing

### Unix

Using Autotools:

    ./autogen.sh  # in ${srcdir}, if obtained from SVN or GIT
    ${srcdir}/configure # in build dir

Using CMake:

    cmake ${srcdir} # in build dir

Now, all versions:

    make
    make check
    make install # as root
    ldconfig # as root


### Microsoft Windows

If you use Microsoft Visual C++ (7.1 or later) compiler, you can build
GEOS using NMAKE program and provided `makefile.vc` files.

If you are building from SVN or GIT checkout, first run: `autogen.bat`
Then:

    nmake /f makefile.vc MSVC_VER=1400

where 1400 is version number of Visual C++ compiler, here Visual C++ 8.0
from Visual Studio 2005 (supported versions are 1300, 1310, 1400, 1500,
1600, 1700, 1800 and 1900).
The bootstrap.bat step is required to generate a couple of header files.

In order to build debug configuration of GEOS, additional flag `DEBUG=1`
is required:

    nmake /f makefile.vc MSVC_VER=1400 DEBUG=1


## Client applications

### Using the C interface (recommended)

To compile programs against the C lib (recommended):

    CFLAGS += `geos-config --cflags`
    LDFLAGS += `geos-config --ldflags` -lgeos_c
    #include <geos_c.h>

Example usage:

    capi/geostest.c contains basic usage examples.

### Using the C++ interface (discouraged)

NB: The C++ interface should not be used directly; the GEOS project
views it as a bug for another program to use the C++ interface or even
to directly link against the C++ library.  The C++ library name will
change on every minor release because it is too hard to know if there
have been ABI changes.  (The C library uses the C++ interface, but the
C library follows normal ABI-change-sensitive versioning, so programs
that link only against the C library should work without relinking
when GEOS is upgraded.)

To compile programs against the C++ lib:

    CFLAGS += `geos-config --cflags`
    LDFLAGS += `geos-config --ldflags` -lgeos
    #include <geos.h>

Basic usage examples can be found in `doc/example.cpp`.


### Scripting language bindings

Ruby bindings are fully supported. To build, use the `--enable-ruby` option
when configuring:

    ./configure ... --enable-ruby

Since version 3.6.0 PHP bindings are not included in the core
library anymore but available as a separate project:

    https://git.osgeo.org/gogs/geos/php-geos

Since version 3.0, the Python bindings are unsupported. Recommended options:

 1. Become or recruit a new maintainer.
 2. Use [Shapely](http://pypi.python.org/pypi/Shapely) with Python
    versions 2.4 or greater.
 3. Simply call functions from libgeos_c via Python ctypes.

## Documentation

To build Doxygen documentation:

    cd doc
    make doxygen-html

