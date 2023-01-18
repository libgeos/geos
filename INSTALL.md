## Building GEOS From Source

### Prerequisites

GEOS has no external library dependencies and can be built with any C++14
compiler.

### Unix

GEOS can be built on Unix systems using the CMake build system.

To build `GEOS` using CMake, create a build directory and run the `cmake` command
from that location:

```
mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=Release ..
```

Setting `CMAKE_BUILD_TYPE` to `Release` is necessary to enable compiler
optimizations.

Once the `cmake` tool has run, GEOS can be built by:

* running `make` and installed by running `make install`, or
* running `cmake --build .` and `cmake --build . --target install`

The entire test suite can be run by:

* using `make check`, or
* using `ctest --output-on-failure .`

The `ctest` command can be used to control which tests are run.
For example, `ctest -R unit-capi -j2` uses a regular expression to run all tests
associated with the C API, using two processes in parallel.
A list of available tests can be obtained using `ctest -N`.

### Microsoft Windows

GEOS can be built with Microsoft Visual C++ by opening the `CMakeLists.txt` in
the project root using `File > Open > CMake`.

#### Build with CMake generator for Ninja (fast)

If you prefer the command-line, in the Visual Studio 2019 command prompt, `x64 Native Tools Command Prompt for VS 2019` or `x64_x86 Cross Tools Command Prompt for VS 2019` run:

```
cmake -S . -B _build_vs2019_ninja -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build _build_vs2019_ninja -j 16 --verbose
```

#### Build with CMake generator for MSBuild (default)

In the non-specific Command Prompt:

##### 64-bit

```
cmake -S . -B _build_vs2019x64 -G "Visual Studio 16 2019" -A x64 -DCMAKE_GENERATOR_TOOLSET=host=x64
cmake --build _build_vs2019x64 --config Release -j 16 --verbose
```

##### 32-bit

```
cmake -S . -B _build_vs2019x32 -G "Visual Studio 16 2019" -A x32 -DCMAKE_GENERATOR_TOOLSET=host=x64
cmake --build _build_vs2019x32 --config Release -j 16 --verbose
```

