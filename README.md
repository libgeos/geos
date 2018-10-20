GEOS -- Geometry Engine, Open Source
====================================

Project homepage: http://geos.osgeo.org/

## Build status

| branch / CI | Debbie | Winnie | Dronie | Travis CI | GitLab CI | AppVeyor | Bessie | Bessie32 |
|:---         |:---    |:---    |:---    |:---       |:---       |:---      |:---    |:---    |
| master      | [![debbie](https://debbie.postgis.net/buildStatus/icon?job=GEOS_Master)](https://debbie.postgis.net/view/GEOS/job/GEOS_Master/) | [![winnie](https://winnie.postgis.net:444/view/GEOS/job/GEOS_Master/badge/icon)](https://winnie.postgis.net:444/view/GEOS/job/GEOS_Master/) | [![dronie](https://drone.osgeo.org/api/badges/geos/geos/status.svg?branch=master)](https://drone.osgeo.org/geos/geos?branch=master) | [![travis](https://travis-ci.com/libgeos/geos.svg?branch=master)](https://travis-ci.com/libgeos/geos?branch=master) | [![gitlab-ci](https://gitlab.com/geos/libgeos/badges/master/build.svg)](https://gitlab.com/geos/libgeos/commits/master) | [![appveyor](https://ci.appveyor.com/api/projects/status/ao909hwpsb1yu062/branch/master?svg=true)](https://ci.appveyor.com/project/OSGeo/geos/branch/master) | [![bessie](https://debbie.postgis.net/buildStatus/icon?job=GEOS_Worker_Run/label=bessie&BRANCH=master)](https://debbie.postgis.net/view/GEOS/job/GEOS_Worker_Run/label=bessie) | [![bessie32](https://debbie.postgis.net/buildStatus/icon?job=GEOS_Worker_Run/label=bessie32&BRANCH=master)](https://debbie.postgis.net/view/GEOS/job/GEOS_Worker_Run/label=bessie32) ||
| 3.7     | [![debbie](https://debbie.postgis.net/buildStatus/icon?job=GEOS_Branch_3.7)](https://debbie.postgis.net/view/GEOS/job/GEOS_Branch_3.7/) | [![winnie](https://winnie.postgis.net:444/view/GEOS/job/GEOS_Branch_3.7/badge/icon)](https://winnie.postgis.net:444/view/GEOS/job/GEOS_Branch_3.7/) | [![dronie](https://drone.osgeo.org/api/badges/geos/geos/status.svg?branch=3.7)](https://drone.osgeo.org/geos/geos?branch=3.7) | [![travis](https://travis-ci.com/libgeos/geos.svg?branch=3.7)](https://travis-ci.com/libgeos/geos?branch=3.7) | [![gitlab-ci](https://gitlab.com/geos/libgeos/badges/svn-3.7/build.svg)](https://gitlab.com/geos/libgeos/commits/3.7) | [![appveyor](https://ci.appveyor.com/api/projects/status/ao909hwpsb1yu062/branch/3.7?svg=true)](https://ci.appveyor.com/project/OSGeo/geos/branch/svn-3.6) ||
| 3.6     | [![debbie](https://debbie.postgis.net/buildStatus/icon?job=GEOS_Branch_3.6)](https://debbie.postgis.net/view/GEOS/job/GEOS_Branch_3.6/) | [![winnie](https://winnie.postgis.net:444/view/GEOS/job/GEOS_Branch_3.6/badge/icon)](https://winnie.postgis.net:444/view/GEOS/job/GEOS_Branch_3.6/) | [![dronie](https://drone.osgeo.org/api/badges/geos/geos/status.svg?branch=svn-3.6)](https://drone.osgeo.org/geos/geos?branch=svn-3.6) | [![travis](https://travis-ci.com/libgeos/geos.svg?branch=svn-3.6)](https://travis-ci.com/libgeos/geos?branch=svn-3.6) | [![gitlab-ci](https://gitlab.com/geos/libgeos/badges/svn-3.6/build.svg)](https://gitlab.com/geos/libgeos/commits/svn-3.6) | [![appveyor](https://ci.appveyor.com/api/projects/status/ao909hwpsb1yu062/branch/svn-3.6?svg=true)](https://ci.appveyor.com/project/OSGeo/geos/branch/svn-3.6) |

More on: https://trac.osgeo.org/geos#BuildandInstall

## Building, testing, installing

### Prerequisites

Building GEOS requires a C++11 compiler and
[CMake 3.12](https://cmake.org/files/v3.12/) or later.

### Building GEOS

```shell
git clone https://github.com/libgeos/geos.git
mkdir geos/_build
cd geos/_build
cmake -DCMAKE_INSTALL_PREFIX=/usr/local ..
cmake --build . --config Release
```

### Testing GEOS

```shell
cd geos/_build
ctest --build-config Release
```

List all available test targets:

```shell
cmake --build . --target help | grep test
```

Run selected test target:

```shell
ctest --build-config Release -R test_xmltester
```

### Installing GEOS

```shell
cd geos/_build
cmake --build . --config Release --target install
```

### Building GEOS client

```shell
mkdir geos/_build_client
cd geos/_build_client
cmake -DCMAKE_PREFIX_PATH=/usr/local ../examples/client
cmake --build . --config Release
```
