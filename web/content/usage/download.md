---
title: "Download Source"
date: 2021-10-04T13:44:59-07:00
draft: false
---

| Release Date | Release | Download Link |
| ------- | ------ | ------ |
|  2021/11/02 | **{{<current_release>}}** | [geos-{{<current_release>}}.tar.bz2](http://download.osgeo.org/geos/geos-{{<current_release>}}.tar.bz2) |
|  2021/11/01 | **3.9.2** |  [geos-3.9.2.tar.bz2](http://download.osgeo.org/geos/geos-3.9.2.tar.bz2) |
|  2021/04/10 | **3.8.2** |  [geos-3.8.2.tar.bz2](http://download.osgeo.org/geos/geos-3.8.2.tar.bz2) |
|  2019/10/04 | **3.7.3** |  [geos-3.7.3.tar.bz2](http://download.osgeo.org/geos/geos-3.7.3.tar.bz2) |
|  2020/12/11 | **3.6.5** |  [geos-3.6.5.tar.bz2](http://download.osgeo.org/geos/geos-3.6.5.tar.bz2) |
|  2019/10/04 | **3.5.2** |  [geos-3.5.2.tar.bz2](http://download.osgeo.org/geos/geos-3.5.2.tar.bz2) |


## Build From Source

### Build Requirements

* [CMake](https://cmake.org/download/) 3.13 or later.
* C++11 compiler. We regularly test GCC, Clang and Microsoft Visual C++.
* [Doxygen](https://www.doxygen.nl/) to build the API documentation.

### Build

Builds with CMake are done "outside the tree" either in a build directory in the source tree or next to the tree.

```bash
# Unpack and setup build directory
tar xvfz geos-{{<current_release>}}.tar.bz2
cd geos-{{<current_release>}}
mkdir _build
cd _build
# Set up the build
cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    ..
# Run the build, test, install steps
make
ctest
make install
```

### Build Options

The GEOS build can be customized using build options.

| Option               | Default    | Note  |
| :------------------: | :--------: | :---: |
| CMAKE_BUILD_TYPE     | Release    | Use `Debug` to build with debug flags and optimizations off. Use `Release` for packaging and production installs. Use `RelWithDebInfo` for optimized build with debug symbols. |
| CMAKE_INSTALL_PREFIX | /usr/local | Set to install root. Librarys end up in `./libs` headers in `./include` |
| BUILD_DOCUMENTATION  | ON         | Attempt to find `doxygen` executable and build API docs |
| BUILD_SHARED_LIBS    | ON         | Build dynamically linkable libraries. |
| DISABLE_GEOS_INLINE  | OFF        | Turn off inlining. This is bad for performance, only do this if you cannot build to pass tests on your platform with inlining on. |

### Test Options

It is possible to run ctest directly. This gives access to ctest command line options (see ctest --help for a listing).

```bash
ctest
ctest --verbose
```

A list of GEOS test suites is obtained by running `ctest --show-only`:

```bash
$ ctest --show-only
#
# Test project /home/dan/dev/libgeos/cmake-build-debug
#  Test #1: test_geos_unit
#  Test #2: test_xmltester
#  Test #3: test_bug234
#  Test #4: test_sweep_line_speed
```

A subset of test suites can be run using a regular expression (and in this case, running 4 jobs in parallel):

```bash
$ ctest --tests-regex test_ --parallel 4
```


