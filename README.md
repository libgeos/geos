GEOS -- Geometry Engine, Open Source
====================================

Project homepage: http://geos.osgeo.org/

## Build status

| branch / CI | Debbie | Winnie | Dronie | Travis CI | GitLab CI | AppVeyor | Bessie | Bessie32 |
|:---         |:---    |:---    |:---    |:---       |:---       |:---      |:---    |:---    |
| 3.7      | [![debbie](https://debbie.postgis.net/buildStatus/icon?job=GEOS_3.7)](https://debbie.postgis.net/view/GEOS/job/GEOS_3.7/) | [![winnie](https://winnie.postgis.net:444/view/GEOS/job/GEOS_3.7/badge/icon)](https://winnie.postgis.net:444/view/GEOS/job/GEOS_3.7/) | [![dronie](https://dronie.osgeo.org/api/badges/geos/geos/status.svg?ref=refs/heads/3.7)](https://drone.osgeo.org/geos/geos?branch=3.7) | [![travis](https://travis-ci.com/libgeos/geos.svg?branch=3.7)](https://travis-ci.com/libgeos/geos?branch=3.7) | [![gitlab-ci](https://gitlab.com/geos/libgeos/badges/3.7/build.svg)](https://gitlab.com/geos/libgeos/commits/3.7) | [![appveyor](https://ci.appveyor.com/api/projects/status/ao909hwpsb1yu062/branch/3.7?svg=true)](https://ci.appveyor.com/project/OSGeo/geos/branch/3.7) | [![bessie](https://debbie.postgis.net/buildStatus/icon?job=GEOS_Worker_Run/label=bessie&BRANCH=3.7)](https://debbie.postgis.net/view/GEOS/job/GEOS_Worker_Run/label=bessie) | [![bessie32](https://debbie.postgis.net/buildStatus/icon?job=GEOS_Worker_Run/label=bessie32&BRANCH=3.7)](https://debbie.postgis.net/view/GEOS/job/GEOS_Worker_Run/label=bessie32) ||
| 3.7     | [![debbie](https://debbie.postgis.net/buildStatus/icon?job=GEOS_Branch_3.7)](https://debbie.postgis.net/view/GEOS/job/GEOS_Branch_3.7/) | [![winnie](https://winnie.postgis.net:444/view/GEOS/job/GEOS_Branch_3.7/badge/icon)](https://winnie.postgis.net:444/view/GEOS/job/GEOS_Branch_3.7/) | [![dronie](https://dronie.osgeo.org/api/badges/geos/geos/status.svg?ref=refs/heads/3.7)](https://dronie.osgeo.org/geos/geos?branch=3.7) | [![travis](https://travis-ci.com/libgeos/geos.svg?branch=3.7)](https://travis-ci.com/libgeos/geos?branch=3.7) | [![gitlab-ci](https://gitlab.com/geos/libgeos/badges/3.7/build.svg)](https://gitlab.com/geos/libgeos/commits/3.7) | [![appveyor](https://ci.appveyor.com/api/projects/status/ao909hwpsb1yu062/branch/3.7?svg=true)](https://ci.appveyor.com/project/OSGeo/geos/branch/3.7) |

More on: https://trac.osgeo.org/geos#BuildandInstall

## Building, testing, installing

### Prerequisites

Building GEOS requires a C++11 compiler

### Unix

#### Using Autotools:

    ./autogen.sh  # in ${srcdir}, if obtained from SVN or GIT
    (mkdir obj && cd obj && ../configure)

#### Using CMake:

    (mkdir build && cd build && cmake ..)

#### Either Autotools or CMake

    make
    make check
    make install # (as root, assuming PREFIX is not writable by the build user)

    On a GNU/Linux system, if installed in a system prefix:
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

GEOS promises long term stability of C API

The C library uses the C++ interface, but the C library follows
normal ABI-change-sensitive versioning, so programs that link only
against the C library should work without relinking when GEOS is upgraded.

To compile programs against the C lib (recommended):

    CFLAGS += `geos-config --cflags`
    LDFLAGS += `geos-config --ldflags` -lgeos_c
    #include <geos_c.h>

Example usage:

    capi/geostest.c contains basic usage examples.

### Using the C++ interface (no stability promise)

Developers who decide to use the C++ interface should be aware GEOS
does not promise API or ABI stability of C++ API between releases.
Moreover C++ API/ABI breaking changes may not even be announced
or include in the NEWS file

The C++ library name will change on every minor release because
it is too hard to know if there have been ABI changes.

To compile programs against the C++ lib:

    CFLAGS += `geos-config --cflags`
    LDFLAGS += `geos-config --ldflags` -lgeos
    #include <geos.h>

Basic usage examples can be found in `doc/example.cpp`.


### Scripting language bindings

Ruby bindings are fully supported. To build, use the `--enable-ruby` option
when configuring:

    ./configure ... --enable-ruby

Since version 3.7.0 PHP bindings are not included in the core
library anymore but available as a separate project:

    https://git.osgeo.org/gogs/geos/php-geos

Since version 3.0, the Python bindings are unsupported. Recommended options:

 1. Become or recruit a new maintainer.
 2. Use [Shapely](http://pypi.python.org/pypi/Shapely) with Python
    versions 2.4 or greater.
 3. Simply call functions from `libgeos_c` via Python ctypes.

## Documentation

To build Doxygen documentation:

    cd doc
    make doxygen-html

