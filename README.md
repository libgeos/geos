GEOS -- Geometry Engine, Open Source
====================================

GEOS is a C++ library for performing operations on two-dimensional vector
geometries. It is primarily a port of the [JTS Topology
Suite](https://github.com/locationtech/jts) Java library.  It provides many of
the algorithms used by [PostGIS](http://www.postgis.net/), the
[Shapely](https://pypi.org/project/Shapely/) package for Python, the
[sf](https://github.com/r-spatial/sf) package for R, and others.

More information is available the [project homepage](https://libgeos.org).

The official Git repository is at [GitHub](https://github.com/libgeos/geos).

## Build Status

| CI    | Status | CI    | Status | CI    | Status |
| :---: | :----- | :---: | :----- | :---: | :----- |
| GitHub | [![github](https://github.com/libgeos/geos/workflows/CI/badge.svg?branch/main)](https://github.com/libgeos/geos/actions?query=workflow:CI+branch:main) | Bessie | [![bessie](https://debbie.postgis.net/buildStatus/icon?job=GEOS_Worker_Run/label=bessie&build=last:${params.reference=refs/heads/main})](https://debbie.postgis.net/view/GEOS/job/GEOS_Worker_Run/label=bessie) | Debbie | [![debbie](https://debbie.postgis.net/buildStatus/icon?job=GEOS_Master)](https://debbie.postgis.net/view/GEOS/job/GEOS_Master/) |
| GitLab CI | [![gitlab-ci](https://gitlab.com/geos/libgeos/badges/main/pipeline.svg)](https://gitlab.com/geos/libgeos/commits/main) | Bessie32  | [![bessie32](https://debbie.postgis.net/buildStatus/icon?job=GEOS_Worker_Run/label=bessie32&build=last:${params.reference=refs/heads/main})](https://debbie.postgis.net/view/GEOS/job/GEOS_Worker_Run/label=bessie32) | Winnie | [![winnie](https://winnie.postgis.net/view/GEOS/job/GEOS_Master/badge/icon)](https://winnie.postgis.net/view/GEOS/job/GEOS_Master/) |
| | | Berrie | [![berrie](https://debbie.postgis.net/buildStatus/icon?job=GEOS_Worker_Run/label=berrie&build=last:${params.reference=refs/heads/main})](https://debbie.postgis.net/view/GEOS/job/GEOS_Worker_Run/label=berrie) | Dronie | [![dronie](https://dronie.osgeo.org/api/badges/geos/geos/status.svg?branch=main)](https://dronie.osgeo.org/geos/geos?branch=master) |
| | | Berrie64 | [![berrie64](https://debbie.postgis.net/buildStatus/icon?job=GEOS_Worker_Run/label=berrie64&build=last:${params.reference=refs/heads/main})](https://debbie.postgis.net/view/GEOS/job/GEOS_Worker_Run/label=berrie64) |

## Community Resources

* Website: https://libgeos.org
* **git** repository: https://github.com/libgeos/geos
* [**geos-devel** mailing list](https://lists.osgeo.org/mailman/listinfo/geos-devel) and [archive](https://lists.osgeo.org/pipermail/geos-devel/)
* **#geos** chat channel (all bridged):
  * Matrix: https://matrix.to/#/#geos:osgeo.org
  * IRC: irc://irc.libera.chat/#osgeo-geos (https://kiwiirc.com/nextclient/irc.libera.chat/#osgeo-geos)
  * Slack: https://osgeo.slack.com/messages/C07RKJ06B/

## Build/Install

See the [INSTALL](INSTALL.md) file.

## Reference Docs

* [C API](https://libgeos.org/doxygen/geos__c_8h.html)
* [C++ API](https://libgeos.org/doxygen/cpp_iface.html)

See also the [C API tutorial](https://libgeos.org/usage/c_api/)
and the [C++ API tutorial](https://libgeos.org/usage/cpp_api/).
There are code [examples](https://github.com/libgeos/geos/tree/main/examples) in the code repository.

## Client Applications

### Using the C interface

GEOS promises long-term stability of the C API. In general, successive releases
of the C API may add new functions but will not remove or change existing types
or function signatures. The C library uses the C++ interface, but the C library
follows normal ABI-change-sensitive versioning, so programs that link only
against the C library should work without relinking when GEOS is upgraded. For
this reason, it is recommended to use the C API for software that is intended
to be dynamically linked to a system install of GEOS.

The `geos-config` program can be used to determine appropriate compiler and
linker flags for building against the C library:

    CFLAGS += `geos-config --cflags`
    LDFLAGS += `geos-config --ldflags` -lgeos_c

All functionality of the C API is available through the `geos_c.h` header file.

Documentation for the C API is provided via comments in the `geos_c.h` header
file. C API usage examples can be found in the GEOS unit tests and in the
source code of software that uses GEOS, such as PostGIS and the sf package
for R.

### Using the C++ interface

The C++ interface to GEOS provides a more natural API for C++ programs, as well
as additional functionality that has not been exposed in the C API.  However,
developers who decide to use the C++ interface should be aware that GEOS does
not promise API or ABI stability of the C++ API between releases.  Breaking
changes in the C++ API/ABI are not typically announced or included in the NEWS
file.

The C++ library name will change on every minor release.

The `geos-config` program can be used to determine appropriate compiler and
linker flags for building against the C++ library:

    CFLAGS += `geos-config --cflags`
    LDFLAGS += `geos-config --ldflags` -lgeos

A compiler warning may be issued when building against the C++ library. To
remove the compiler warning, define `USE_UNSTABLE_GEOS_CPP_API` somewhere
in the program.

Commonly-used functionality of GEOS is available in the `geos.h` header file.
Less-common functionality can be accessed by including headers for individual
classes, e.g. `#include <geos/algorithm/distance/DiscreteHausdorffDistance.h>`.

    #include <geos.h>

C++ usage examples can be found in [examples](examples/).

### Using other languages

GEOS has bindings in many languages, see the [bindings
page](https://libgeos.org/usage/bindings/).

## Documentation

API documentation can be generated using Doxygen. Documentation is not included
in the default build. To build the documentation, run:

    cmake -DBUILD_DOCUMENTATION=YES
    cmake --build . --target docs

## Style

To format your code into the desired style, use the astyle
version included in source tree:

    tools/astyle.sh <yourfile.cpp>

## Testing

See documentation in [tests/README.md](tests/README.md).

## Tools

* `geosop` - a CLI for GEOS.  Documentation is in [util/geosop/README.md](util/geosop/README.md).
