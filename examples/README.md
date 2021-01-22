# GEOS Example Programs

* `capi_read` uses the standard C API to read two WKT geometries, calculate the intersection and print the result
* `capi_read_ts` uses the "re-entrant" C API (threadsafe) to read two WKT geometries, calculate the intersection and print the result
* `cpp_read` uses the C++ API to read two WKT geometries, calculate the intersection and print the result
* `capi_prepared` uses the standard C API to read one WKT geometry, and fill it with a point grid, applying a high performance "prepared" geometry to speed up intersection testing

## Build

```
mkdir _build
cd _build
cmake ..
make
```
