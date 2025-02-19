---
title: "Well-Known Text (WKT)"
date: 2021-10-04T14:18:42-07:00
draft: false
---

"Well-known text" is a scheme for writing a [simple features](https://en.wikipedia.org/wiki/Simple_Features) geometry into a standard text string.

## Examples

* POINT (0 0)
* POINT EMPTY
* LINESTRING (0 0, 0 1, 1 2)
* LINESTRING EMPTY
* POLYGON ((0 0, 1 0, 1 1, 0 1, 0 0))
* POLYGON ((0 0, 4 0, 4 4, 0 4, 0 0), (1 1, 1 2, 2 2, 2 1, 1 1))
* POLYGON EMPTY
* MULTIPOINT ((0 0), (1 1))
* MULTILINESTRING ((0 0, 1 1), (2 2, 3 3))
* MULTIPOLYGON (((1 1, 1 3, 3 3, 3 1, 1 1)), ((4 3, 6 3, 6 1, 4 1, 4 3)))
* GEOMETRYCOLLECTION (MULTIPOINT((0 0), (1 1)), POINT(3 4), LINESTRING(2 3, 3 4))
* POINTZ (0 0 0)
* POINT Z (0 0 0)
* POINT Z EMPTY
* POINTM (0 0 0)
* POINT M (0 0 0)
* POINTZM (0 0 0 0)
* POINT ZM (0 0 0 0)

## WKT BNF

```
<x> ::= <number>
<y> ::= <number>
<z> ::= <number>
<m> ::= <number>
<empty set> ::= EMPTY
<zm> ::= ZM | Z | M
<comma> ::= ,
<left paren> ::= (
<right paren> ::= )

<point> ::= <x> <y> [ <z> ] [ <m> ]

<point text> ::=
    <empty set> |
    <left paren> <point> <right paren>

<linestring text> ::=
    <empty set> |
    <left paren> <point> {<comma> <point>} ... <right paren>

<polygon text> ::=
    <empty set> |
    <left paren> <linestring text> {<comma> <linestring text>} ... <right paren>

<multipoint text> ::=
    <empty set> |
    <left paren> <point> {<comma> <point>} ... <right paren>

<multilinestring text> ::=
    <empty set> |
    <left paren> <linestring text> {<comma> <linestring text>} ... <right paren>

<multipolygon text> ::=
    <empty set> |
    <left paren> <polygon text> {<comma> <polygon text>} ... <right paren>

<geometrycollection text> ::=
    <empty set> |
    <left paren> <wkt representation> {<comma> <wkt representation>} ... <right paren>

<point text representation> ::=
    POINT [ <zm> ] <point text>

<linestring text representation> ::=
    LINESTRING [ <zm> ] <linestring text body>

<polygon text representation> ::=
    POLYGON [ <zm> ] <polygon text body>

<multipoint text representation> ::=
    MULTIPOINT [ <zm> ] <multipoint text>

<multilinestring text representation> ::=
    MULTILINESTRING [ <zm> ] <multilinestring text>

<multipolygon text representation> ::=
    MULTIPOLYGON [ <zm> ] <multipolygon text>

<geometrycollection text representation> ::=
    GEOMETRYCOLLECTION [ <zm> ] <geometrycollection text>

<wkt representation> ::=
    <point text representation> |
    <linestring text representation> |
    <polygon text representation> |
    <collection text representation> |
    <multipoint text representation> |
    <multilinestring text representation> |
    <multipolygon text representation> |
    <geometrycollection text representation>
```

### Writing WKT

Because WKT is a decimal text output, it will never be able to exactly represent the underlying machine representation of the coordinates. In general, WKT is used for display to humans, or for transport when exact coordinate representation is not required (some web applications).

Accordingly, the GEOS well-known text writer allows you to customize the coordinate precision and rounding behaviour, as well as the dimensionality of outputs.

```c
/* Read a linestring */
const char* linestring = "LINESTRING(0 0 1, 1 1 1, 2 1 2)";
GEOSWKTReader* reader = GEOSWKTReader_create();
GEOSGeom* geom = GEOSWKTReader_read(reader, linestring);

/* Get a WKT writer */
GEOSWKTWriter* writer = GEOSWKTWriter_create();

/* Preserve the Z dimension -- only needed before GEOS 3.12 */
GEOSWKTWriter_setOutputDimension(writer, 3);

/* Sets the number places after the decimal to output in WKT. Default 16. */
GEOSWKTWriter_setRoundingPrecision(writer, 4);

/*
 * Sets the number trimming option on a \ref GEOSWKTWriter.
 * With trim set to 1, the writer will strip trailing 0's from
 * the output coordinates. With 0, all coordinates will be
 * padded with 0's out to the rounding precision.
 * This is generally only needed before GEOS 3.12.
 */
GEOSWKTWriter_setTrim(writer, 1);

/* Generate the WKT */
unsigned char* wkt = GEOSWKTWriter_write(writer, geom);

/* do something ... */

/* Free the WKT */
GEOSFree(wkt);
GEOSGeom_destroy(geom);
GEOSWKTWriter_destroy(writer);
GEOSWKTReader_destroy(reader);
```
