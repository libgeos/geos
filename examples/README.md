# GEOS Example Programs

* `capi_read` uses the standard C API to read two WKT geometries, calculate the intersection and print the result
* `capi_read_ts` uses the "re-entrant" C API (threadsafe) to read two WKT geometries, calculate the intersection and print the result
* `capi_prepared` uses the standard C API to read one WKT geometry, and fill it with a point grid, applying a high performance "prepared" geometry to speed up intersection testing
* `capi_strtree` uses the standard C API to build a random collection of points, and then search that collection quickly to find the nearest to a query point
* `capi_indexed_predicate` uses the standard C API API to build an STRtree index on a custom class, and then query that index with a prepared geometry, returning a list of matching items
* `cpp_read` uses the C++ API to read two WKT geometries, calculate the intersection and print the result
* `cpp_strtree` uses the C++ API to build an STRtree index on a custom class, and then query that index


## Build

To build the examples _in situ_:
```
mkdir _build
cd _build
cmake ..
make
```

To build programs in general, you will need to ensure that the appropriate header and include files are available to the compiler. Here's a raw compile command example for a CAPI program:

```
cc -I/usr/local/include \
    capi_read.c \
    -o capi_read \
    -L/usr/local/lib \
    -lgeos_c
```

* Ensure the header files can be found by the compiler
* Ensure the library files can be found by the compiler
* Ensure the C API library is passed to the linker

To build a C++ API program, you must pass a define indicating you're OK with the fact that the API will change over time.

```
c++ -I/usr/local/include -v \
    -std=c++14 \
    -D USE_UNSTABLE_GEOS_CPP_API \
    cpp_read.cpp \
    -o cpp_read \
    -L/usr/local/lib \
    -lgeos
```
