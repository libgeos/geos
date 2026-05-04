## Building

Building the benchmark tests must be enabled using:

```bash
cmake -DBUILD_BENCHMARKS=ON ..
```

Geos uses Google Benchmarks for validating performance of Geos library. It can be built using the following ways:

### Linux

Install via package manager:

```bash
# Debian/Ubuntu
sudo apt install libbenchmark-dev

# Fedora
sudo dnf install google-benchmark-devel
```

Then build:

```bash
mkdir build && cd build
cmake -DBUILD_BENCHMARKS=ON ..
make -j$(nproc)
```

### Windows (MSYS2)

Install Google Benchmark via pacman:

```bash
# MINGW64
pacman -S mingw-w64-x86_64-benchmark

# UCRT64
pacman -S mingw-w64-ucrt-x86_64-benchmark

# CLANG64
pacman -S mingw-w64-clang-x86_64-benchmark

# CLANGARM64
pacman -S mingw-w64-clang-aarch64-benchmark
```

Then build:

```bash
mkdir build && cd build
cmake -G "MinGW Makefiles" -DBUILD_BENCHMARKS=ON ..
mingw32-make -j$(nproc)
```

### Windows (MSVC / vcpkg)

```bash
vcpkg install benchmark:x64-windows
mkdir build && cd build
cmake -DBUILD_BENCHMARKS=ON -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake ..
cmake --build . --config Release
```
