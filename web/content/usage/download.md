---
title: "Download and Build"
date: 2022-11-14T00:00-07:00
draft: false
---


## Download Project

| Release | Release Date | First Release | Final Release | Download Link | Changes |
| :--: | :--:| :--: |:--: | :--: | :--: |
|  **3.11.1** | 2022/11/14 | 2022/07/01 | *2026/07/01* | [geos-3.11.1.tar.bz2](https://download.osgeo.org/geos/geos-3.11.1.tar.bz2) | [Changes](https://github.com/libgeos/geos/blob/3.11.1/NEWS.md) |
|  **3.10.4** | 2022/11/14 | 2021/10/20 | *2025/10/20* | [geos-3.10.4.tar.bz2](https://download.osgeo.org/geos/geos-3.10.4.tar.bz2) | [Changes](https://github.com/libgeos/geos/blob/3.10.4/NEWS)
|  **3.9.4**  | 2022/11/14 | 2020/12/09 | *2024/12/09* | [geos-3.9.4.tar.bz2](https://download.osgeo.org/geos/geos-3.9.4.tar.bz2) | [Changes](https://github.com/libgeos/geos/blob/3.9.4/NEWS) |
|  **3.8.3** | 2022/06/02 |  2019/10/10 | *2023/10/10* | [geos-3.8.3.tar.bz2](https://download.osgeo.org/geos/geos-3.8.3.tar.bz2) | [Changes](https://github.com/libgeos/geos/blob/3.8.3/NEWS) |
| **3.7.5**  | 2022/06/08  | 2018/09/10 | *2023/03/31*  | [geos-3.7.5.tar.bz2](https://download.osgeo.org/geos/geos-3.7.5.tar.bz2) | [Changes](https://github.com/libgeos/geos/blob/3.7.5/NEWS) |
| **3.6.5**  | 2020/12/11 | 2016/10/25 | *2022/10/25* | [geos-3.6.5.tar.bz2](https://download.osgeo.org/geos/geos-3.6.5.tar.bz2) | [Changes](https://github.com/libgeos/geos/blob/3.6.5/NEWS) |
| **3.5.2** EOL | 2019/10/04 | 2015/08/16 | 2019/10/04|  [geos-3.5.2.tar.bz2](https://download.osgeo.org/geos/geos-3.5.2.tar.bz2) | [Changes](https://github.com/libgeos/geos/blob/3.5.2/NEWS) |

If you see **EOL** next to a released version, it means no security or patch updates will be added to that minor and you should upgrade to a higher release.
Refer to our [End-Of-Life (EOL) Policy](/development/rfcs/rfc11/) for details.
Older releases prior to the ones listed above, are all EOL'd.

Old releases can be downloaded from https://download.osgeo.org/geos/.

All *future* **Final Release** dates are subject to change.

## Build From Source

### Build Requirements

* [CMake](https://cmake.org/download/) 3.13 or later.
* C++11 compiler. We regularly test GCC, Clang and Microsoft Visual C++.
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
Options are specified via [`cmake` variables](https://cmake.org/cmake/help/v2.8.8/cmake.html#section_Variables).
They are specified on the `cmake` cmdline as `-DVAR=VALUE`.

{{< hint warning >}}
*NOTE: Running cmake with no variables does NOT clear current variable settings. They must be set explicitly on the cmdline or interactively using `ccmake ..`. To revert to the defaults, clear the build directory contents using `rm -rf *`.*
{{< /hint >}}

| Option               | Default    | Note  |
| :------------------: | :--------: | :---: |
| CMAKE_BUILD_TYPE     | Release    | Use `Debug` to build with debug flags and optimizations off. Use `Release` for packaging and production installs. Use `RelWithDebInfo` for optimized build with debug symbols. |
| CMAKE_INSTALL_PREFIX | /usr/local | Set to install root. Librarys end up in `./libs` headers in `./include` |
| BUILD_DOCUMENTATION  | ON         | Attempt to find `doxygen` executable and build API docs |
| BUILD_SHARED_LIBS    | ON         | Build dynamically linkable libraries. |


## Testing

It is possible to run `ctest` directly. This gives access to ctest command line options (see `ctest --help` for a listing).

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
