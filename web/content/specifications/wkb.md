---
title: "Well-Known Binary (WKB)"
date: 2021-10-04T14:18:42-07:00
draft: false
---

"Well-known binary" is a scheme for writing a [simple features](https://en.wikipedia.org/wiki/Simple_Features) geometry into a platform-independent array of bytes, usually for transport between systems or between programs. By using WKB, systems can avoid exposing their particular internal implementation of geometry storage, for greater overall interoperability.

GEOS can read and write three "flavours" of WKB:

* [Standard WKB]({{< ref "#standard-wkb" >}})
* [Extended WKB]({{< ref "#extended-wkb" >}})
* [ISO WKB]({{< ref "#iso-wkb" >}})

When reading WKB, GEOS will transparently detect the flavour of WKB and automatically read it. When writing WKB, the application developer must indicate what flavour of WKB to use, in particular when writing higher dimensional outputs.

## Standard WKB

Standard WKB supports two-dimensional geometry, and is a proper subset of both extended WKB and ISO WKB.

### Data Types

The WKB specification uses three basic types common to most typed languages: an unsigned byte; a 4-byte unsigned integer; and an [8-byte IEEE double](https://en.wikipedia.org/wiki/Double-precision_floating-point_format).

```
// byte : 1 byte
// uint32 : 32 bit unsigned integer (4 bytes)
// double : double precision number (8 bytes)
```

### Byte Order

In order to allow portability between systems with difference architectures, the representation of those types is conditioned by the `wkbByteOrder`.

```
enum wkbByteOrder  {
    wkbXDR = 0, // Big Endian
    wkbNDR = 1  // Little Endian
};
```

A "little endian" integer has the least-significant bytes first, hence "little". For example, the number 1, encoded in little- and big- endian:

```
# Little endian
01 00 00 00

# Big endian
00 00 00 01
```

In practice this means that almost all WKB is encoded little endian, since most modern processors are little endian, but the existence of the `wkbByteOrder` allows WKB to transport geometry easily between systems of different endianness.

### Coordinates

The double precision numbers in the coordinates are also subject to the byte order rules. To encode geometries with more than 2 dimensions, see the [Extended WKB]({{< ref "#extended-wkb" >}}) format below.

```
Point {
    double x;
    double y;
};
```

### Rings

Linear rings are components of the polygon type, and never appear outside of the polygon. Hence they dispense with the byte order and geometry type, since both are implicit in their location in the polygon structure.

```
LinearRing {
    uint32 numPoints;
    Point  points[numPoints];
};
```

### Empty Geometries

For most geometry types, empty geometries are indicated naturally by having `numPoints`, `numRings`, or `numGeometries` = 0.
Points do not have a `numPoints` field, so `POINT EMPTY` is represented by a `Point` 
with each ordinate value set to an IEEE-754 quiet NaN value
(big endian 0x7ff8000000000000 or little endian 0x000000000000f87f).

### Geometry Types

GEOS only supports the seven original simple features geometry types.

```
enum wkbGeometryType {
    wkbPoint = 1,
    wkbLineString = 2,
    wkbPolygon = 3,
    wkbMultiPoint = 4,
    wkbMultiLineString = 5,
    wkbMultiPolygon = 6,
    wkbGeometryCollection = 7
};
```

Other systems (eg PostGIS) support a wider range of types (for example, CircularString, CurvePolygon), and hence more geometry type numbers, but GEOS is currently unable to consume those geometries.

```
WKBPoint {
    byte    byteOrder;
    uint32  wkbType; // 1
    Point   point;
};

WKBLineString {
    byte    byteOrder;
    uint32  wkbType; // 2
    uint32  numPoints;
    Point   points[numPoints];
};

WKBPolygon {
    byte    byteOrder;
    uint32  wkbType; // 3
    uint32  numRings;
    LinearRing rings[numRings]
}

WKBMultiPoint {
    byte     byteOrder;
    uint32   wkbType; // 4
    uint32   numWkbPoints;
    WKBPoint WKBPoints[numWkbPoints];
}

WKBMultiLineString {
    byte     byteOrder;
    uint32   wkbType; // 5
    uint32   num numWkbLineStrings;
    WKBLineString WKBLineStrings[numWkbLineStrings];
}

wkbMultiPolygon {
    byte    byteOrder;
    uint32  wkbType; // 6
    uint32  numWkbPolygons;
    WKBPolygon wkbPolygons[numWkbPolygons];
}

WKBGeometry {
    union {
        WKBPoint point;
        WKBLineString linestring;
        WKBPolygon polygon;
        WKBGeometryCollection collection;
        WKBMultiPoint mpoint;
        WKBMultiLineString mlinestring;
        WKBMultiPolygon mpolygon;
    }
}

WKBGeometryCollection {
    byte    byteOrder;
    uint32  wkbType; // 7
    uint32  numWkbGeometries;
    WKBGeometry wkbGeometries[numWkbGeometries];
}
```

### Example

The following bytes (in hex) make up the WKB for a `LINESTRING(0 0, 1 1, 2 1)`:

```
01                - byteOrder(wkbNDR)
02000000          - wkbType(LineString)
03000000          - numPoints(3)
0000000000000000  - x(0.0)
0000000000000000  - y(0.0)
000000000000F03F  - x(1.0)
000000000000F03F  - y(1.0)
0000000000000040  - x(2.0)
000000000000F03F  - y(1.0)
```


## Extended WKB

The original WKB specification made no allowances for adding extra dimensions, like Z and M, that are common in GIS applications. It also had no space for embedding a spatial reference identifier (SRID), which made it unusable as a database import/export format.

The "Extended WKB" variant is a superset of the standard WKB, which allows applications to optionally add extra dimensions, and optionally embed an SRID.

The presence of extra dimensions is indicated by adding flag bits to the existing `wkbType` that appears in all WKB geometries.

For example, here is the structure and flagging for a 3D point -- the dimensionaly flag indicates a Z dimension, and the point member therefore has three coordinates.

```
wkbZ = 0x80000000
wkbM = 0x40000000
wkbSRID = 0x20000000

enum wkbGeometryTypeZ {

    wkbPoint = 1,
    wkbLineString = 2,
    wkbPolygon = 3,
    wkbMultiPoint = 4,
    wkbMultiLineString = 5,
    wkbMultiPolygon = 6,
    wkbGeometryCollection = 7,

    // | 0x80000000
    wkbPointZ = 0x80000001,
    wkbLineStringZ = 0x80000002,
    wkbPolygonZ = 0x80000003,
    wkbMultiPointZ = 0x80000004,
    wkbMultiLineStringZ = 0x80000005,
    wkbMultiPolygonZ = 0x80000006,
    wkbGeometryCollectionZ = 0x80000007,
}

WKBPointZ {
    byte   byteOrder; // wkbXDR or wkbNDR
    uint32 wkbType;   // wkbPointZ = (wkbPoint | wkbZ) = 0x80000001
    Point {
        Double x;
        Double y;
        Double z;
    }
}
```

When the optional `wkbSRID` is added to the `wkbType`, an SRID number is inserted after the `wkbType` number.

```
WKBPointS {
    byte   byteOrder; // wkbXDR or wkbNDR
    uint32 wkbType;   // wkbPointS = (wkbPoint | wkbSRID) = 0x20000001
    uint32 SRID;
    Point {
        Double x;
        Double y;
    }
}
```

The dimensionality and SRID flags can also be combined. Here is a 3D point with an embedded SRID.

```
WKBPointS {
    byte   byteOrder; // wkbXDR or wkbNDR
    uint32 wkbType;   // wkbPointS = (wkbPoint | wbkZ | wkbSRID) = 0xA0000001
    uint32 SRID;
    Point {
        Double x;
        Double y;
        Double z;
    }
}
```

### Writing Extended WKB

```c
/* Read a linestring */
const char* linestring = "LINESTRING(0 0 1, 1 1 1, 2 1 2)";
GEOSWKBReader* reader = GEOSWKBReader_create();
GEOSGeom* geom = GEOSWKTReader_read(reader, linestring);
GEOSSetSRID(geom, 4326);

/* Write it out as Extended WKB */
GEOSWKBWriter* writer = GEOSWKBWriter_create();
/* Next line only needed before GEOS 3.12 */
GEOSWKBWriter_setOutputDimension(writer, 3);
GEOSWKBWriter_setFlavor(writer, GEOS_WKB_EXTENDED);
GEOSWKBWriter_setIncludeSRID(writer, 1);

/* Generate the WKB , and store the output length */
size_t wkb_size;
unsigned char* wkb = GEOSWKBWriter_write(writer, geom, &wkb_size);

/* do something ... */

/* Free the WKB */
GEOSFree(wkb);
GEOSGeom_destroy(geom);
```


## ISO WKB

ISO WKB was included in the [ISO 13249-3](https://www.iso.org/standard/60343.html) document (SQL/MM Part 3: Spatial). It allows for higher dimensional geometries, but does not have a facility for SRID embedding.

### Geometry Type

As with [extended WKB]({{< ref "#extended-wkb" >}}), ISO WKB supports higher dimensions by adding information to the `wkbType` number. Rather than using flag bits, however, ISO WKB simply adds a round number to the type number to indicate extra dimensions.

* wkbType + 1000 = wkbTypeZ
* wkbType + 2000 = wkbTypeM
* wkbType + 3000 = wkbTypeZM

### Writing ISO WKB

```c
/* Read a linestring */
const char* linestring = "LINESTRING(0 0 1, 1 1 1, 2 1 2)";
GEOSWKTReader* reader = GEOSWKTReader_create();
GEOSGeom* geom = GEOSWKTReader_read(reader, linestring);

/* Write it out as ISO WKB */
GEOSWKBWriter* writer = GEOSWKBWriter_create();
/* Next line only needed before GEOS 3.12 */
GEOSWKBWriter_setOutputDimension(writer, 3);
GEOSWKBWriter_setFlavor(writer, GEOS_WKB_ISO);

/* Generate the WKB , and store the output length */
size_t wkb_size;
unsigned char* wkb = GEOSWKBWriter_write(writer, geom, &wkb_size);

/* do something ... */

/* Free the WKB */
GEOSFree(wkb);
GEOSGeom_destroy(geom);
GEOSWKBWriter_destroy(writer);
GEOSWKTReader_destroy(reader);
```

