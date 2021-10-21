---
title: "GEOS RFC 3 - Thread Safe CAPI "
date: 2021-10-04T14:21:00-07:00
draft: false
geekdocHidden: true
geekdocHiddenTocTree: false
---

## Summary

The current CAPI in GEOS is not thread safe.  The error handling and initialization/finalization process specifically can cause problems.

## Definitions

(As defined by Frank Warmerdam in http://trac.osgeo.org/gdal/wiki/rfc16_ogr_reentrancy)

**Reentrant:** A reentrant function can be called simultaneously by multiple threads provided that each invocation of the function references unique data.

**Thread-safe:** A thread-safe function can be called simultaneously by multiple threads when each invocation references shared data. All access to the shared data is serialized.

## Objective

Allow the GEOS CAPI to be thread safe.

## Implementation

In order to implement the thread safe API, the current API will be copied and all static variables will be placed into a 'handle.'  This handle will be initialized on the initGeos call.  Once initialized it will be passed to all subsequent GEOS functions, allowing each thread to have it's own copy of the data.  This will not affect the current API as it will be provided in addition to the old API.  In order to prevent  maintenance issues the OLD API will be changed to call the NEW API with a global handle.  The handle (GEOSContextHandle_t) will be an opaque type to allow exentensions without recompilation being required.  Function names in the new API will be updated with an _r, as is the familiar C standard for reentrant/thread safe versions.  Current GEOS functions that do not make reference to the context handle will not be changed.

The intent will be to altogether replace the existing functions with the _r functions in a future release, making the thread safe versions the only supported functions.

### Handle Definition

Here are the internals of the handle and how the application visual handle will look.

```c
typedef struct GEOSContextHandleInternal
{
    const void *geomFactory;
    GEOSMessageHandler NOTICE_MESSAGE;
    GEOSMessageHandler ERROR_MESSAGE;
    int WKBOutputDims;
    int WKBByteOrder;
    int initialized;
} GEOSConextHandleInternal_t;

typedef struct GEOSContextHandle_HS *GEOSContextHandle_t;
```

The typedef for GEOSContextHandle_t will make it easier for the compiler to help detect an incorrect pointer being passed to the functions.

### Example Prototypes

Here are examples of what some of the new function prototypes would be.

```c
GEOSContextHandle_t GEOS_DLL initGEOS_r( GEOSMessageHandler notice_function,
                                         GEOSMessageHandler error_function);
extern void GEOS_DLL finishGEOS_r(GEOSContextHandle_t handle);


extern GEOSGeometry GEOS_DLL *GEOSGeom_createPoint_r(GEOSContextHandle_t handle,
                                                      GEOSCoordSequence* s);
extern GEOSGeometry GEOS_DLL *GEOSGeom_createLinearRing_r(GEOSContextHandle_t handle,
                                                      GEOSCoordSequence* s);
extern GEOSGeometry GEOS_DLL *GEOSGeom_createLineString_r(GEOSContextHandle_t handle,
                                                      GEOSCoordSequence* s);
```

For comparison, here are the same functions as they exist now.

```c
extern void GEOS_DLL initGEOS(GEOSMessageHandler notice_function,
        GEOSMessageHandler error_function);
extern void GEOS_DLL finishGEOS(void);

extern GEOSGeometry GEOS_DLL *GEOSGeom_createPoint(GEOSCoordSequence* s);
extern GEOSGeometry GEOS_DLL *GEOSGeom_createLinearRing(GEOSCoordSequence* s);
extern GEOSGeometry GEOS_DLL *GEOSGeom_createLineString(GEOSCoordSequence* s);
```

## Limitations

This change will focus on making a thread safe version of the API.  Other extensions to the context handle have been suggested, e.g. Access to other geometry factories, overriding memory allocators.  These extensions are beyond the current scope of this design, but this design will be implemented to allow such extensions in the future.

## Testing

An example test executable will be provided that shows the current problem.  It is copied from the existing CAPI test tool.  Once the thread safe API is created the test tool will be updated to the new interface which will address the former problems.

