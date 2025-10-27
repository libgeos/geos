---
title: "Download and Build"
date: 2025-08-21
draft: false
---

## Download Project

| Release | Release Date | First Release | Final Release | Download Link | Changes |
| :--: | :--:| :--: |:--: | :--: | :--: |
|  **3.14.1** | 2025/10/27 | 2025/08/21 | *2029/10/01* | [geos-3.14.1.tar.bz2](https://download.osgeo.org/geos/geos-3.14.1.tar.bz2) | [Changes](https://github.com/libgeos/geos/blob/3.14.1/NEWS.md) |
|  **3.13.1** | 2025/03/03 | 2024/09/06 | *2028/10/01* | [geos-3.13.1.tar.bz2](https://download.osgeo.org/geos/geos-3.13.1.tar.bz2) | [Changes](https://github.com/libgeos/geos/blob/3.13.1/NEWS.md) |
|  **3.12.3** | 2025/03/03 | 2023/06/27 | *2027/07/01* | [geos-3.12.3.tar.bz2](https://download.osgeo.org/geos/geos-3.12.3.tar.bz2) | [Changes](https://github.com/libgeos/geos/blob/3.12.3/NEWS.md) |
|  **3.11.5** | 2025/03/03 | 2022/07/01 | *2026/07/01* | [geos-3.11.5.tar.bz2](https://download.osgeo.org/geos/geos-3.11.5.tar.bz2) | [Changes](https://github.com/libgeos/geos/blob/3.11.5/NEWS.md) |
|  **3.10.7** | 2025/03/03 | 2021/10/20 | *2025/10/20* | [geos-3.10.7.tar.bz2](https://download.osgeo.org/geos/geos-3.10.7.tar.bz2) | [Changes](https://github.com/libgeos/geos/blob/3.10.7/NEWS)

Old releases can be downloaded from https://download.osgeo.org/geos/.  Any releases not in this list are end-of-life (EOL). Refer to our [EOL policy](/project/rfcs/rfc11/) for details. All *Final Release* dates are subject to change.


## Build From Source

### Build Requirements

* [CMake](https://cmake.org/download/) 3.15 or later.
* C++14 compiler. We regularly test GCC, Clang and Microsoft Visual C++.
* [Doxygen](https://www.doxygen.nl/) to build the API documentation.

### Build

Builds with CMake are done "outside the tree" in a build directory either *in* the source tree or *next* to it.  The following shows building into a directory in the source tree.

```bash
# Unpack and setup build directory
tar xvfj geos-{{<current_release>}}.tar.bz2
cd geos-{{<current_release>}}
mkdir _build
cd _build
# Set up the build
cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    ..
# Run the build, test, install
make
ctest
make install
```


### Build Options

The GEOS build can be customized using build options.
Options are specified via [`cmake` variables](https://cmake.org/cmake/help/latest/manual/cmake-variables.7.html).
They are specified on the `cmake` cmdline as `-DVAR=VALUE`.

{{< hint warning >}}
*NOTE: Running cmake with no variables does NOT clear current variable settings. They must be set explicitly on the cmdline, or interactively using `ccmake ..`. To revert to the defaults, clear the build directory contents using `rm -rf *`.*
{{< /hint >}}

| Option               | Default    | Note  |
| :------------------: | :--------: | :---: |
| CMAKE_BUILD_TYPE     | Release    | Use `Debug` to build with debug flags and optimizations off. Use `Release` for packaging and production installs. Use `RelWithDebInfo` for optimized build with debug symbols. Use `ASAN` to enable memory usage checking. |
| CMAKE_INSTALL_PREFIX | /usr/local | Set to install root. Libraries end up in `./lib` or `./lib64`, headers in `./include`, executables in `./bin` |
| BUILD_DOCUMENTATION  | ON         | Attempt to find `doxygen` executable and build API docs |
| BUILD_SHARED_LIBS    | ON         | Build dynamically linkable libraries |
| BUILD_TESTING        | ON         | Build unit tests |
| BUILD_BENCHMARKS     | OFF        | Build benchmark tests |
| USE_CCACHE           | OFF        | Use [`ccache`](https://ccache.dev/) to compile C/C++ objects, making subsequent builds quicker |



## Testing

It is possible to run `ctest` directly. This gives access to ctest command line options (see `ctest --help` for a listing).

```bash
ctest
ctest --output-on-failure
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
