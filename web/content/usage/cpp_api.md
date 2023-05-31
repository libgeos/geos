---
title: "C++ API Programming"
date: 2021-10-04T14:21:00-07:00
draft: false
weight: 40
---

The GEOS C++ API is included in the collection of header files installed in `include/geos` which is a **very large collection**. Effectively it includes both "public" headers that a user might be expected to make use of and "private" headers that are mostly only used by internal algorithms. Currently, the two kinds of headers are not marked in any way, nor is there an easy way to disentagle them.

You can explore the C++ model via the [Doxygen reference](../../doxygen/cpp_iface.html).

Using the C++ API means giving up:

* Stable API, since headers can be moved, re-named or deleted according to the implementation needs of the library.
* Stable ABI, since the complexity of the GEOS symbol space means that binary symbols are known to change between versions, even relatively small releases.

However, if you are careful in restricting your usage you can build applications against the C++ API that avoid most issues:

* Use `Geometry` as your primary handle, and sub-classes like `Point`, `LineString` and `Polygon` as necessary.
* Use the reader and writer classes for data access.
* Use the `TemplateSTRtree` for indexing.
* Use the `PreparedGeometry` class as needed.

One benefit of using the C++ API is access to more modern C++ facilities, like the `std::unique_ptr` and `std::string`.

## Building a Program

The simplest GEOS C++ API application needs to include the `geom::Geometry.h` header and `geom::GeometryFactory.h` header, to construct new geometries. To read geometries from input formats, a reader such as `geom::WKTReader.h` will also be required.

```c++
/*
* # GEOS C++ example 1
*
* Reads two WKT representations and calculates the
* intersection, prints it out, and cleans up.
*
* In general, to avoid API changes, stick to operations
* on Geometry. The more esoteric APIs are more likely
* to change between versions.
*/

#include <iostream>

/* For geometry operations */
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>

/* For WKT read/write */
#include <geos/io/WKTReader.h>
#include <geos/io/WKTWriter.h>

/* Geometry/GeometryFactory */
using namespace geos::geom;

/* WKTReader/WKTWriter */
using namespace geos::io;

int main()
{
    /* New factory with default (float) precision model */
    GeometryFactory::Ptr factory = GeometryFactory::create();

    /*
    * Reader requires a factory to bind the geometry to
    * for shared resources like the PrecisionModel
    */
    WKTReader reader(*factory);

    /* Input WKT strings */
    std::string wkt_a("POLYGON((0 0, 10 0, 10 10, 0 10, 0 0))");
    std::string wkt_b("POLYGON((5 5, 15 5, 15 15, 5 15, 5 5))");

    /* Convert WKT to Geometry */
    std::unique_ptr<Geometry> geom_a(reader.read(wkt_a));
    std::unique_ptr<Geometry> geom_b(reader.read(wkt_b));

    /* Calculate intersection */
    std::unique_ptr<Geometry> inter = geom_a->intersection(geom_b.get());

    /* Convert Geometry to WKT */
    WKTWriter writer;
    writer.setTrim(true); /* Only needed before GEOS 3.12 */
    std::string inter_wkt = writer.write(inter.get());

    /* Print out results */
    std::cout << "Geometry A:         " << wkt_a << std::endl;
    std::cout << "Geometry B:         " << wkt_b << std::endl;
    std::cout << "Intersection(A, B): " << inter_wkt << std::endl;

}
```

