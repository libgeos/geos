---
title: "GEOS"
date: 2021-10-04T14:13:20-07:00
draft: false
geekdocNav: true
---

GEOS is a C/C++ library for spatial computational geometry of the sort generally used by "geographic information systems" software. GEOS is a core dependency of [PostGIS](https://postgis.net), [QGIS](https://qgis.org), [GDAL](https://gdal.org), and [Shapely](https://shapely.readthedocs.io/en/stable/project.html).

## Capabilities

Spatial Model and Functions

* **Geometries**: Point, LineString, Polygon, MultiPoint, MultiLineString, MultiPolygon, GeometryCollection
* **Predicates**: Intersects, Touches, Disjoint, Crosses, Within, Contains, Overlaps, Equals, Covers
* **Operations**: Union, Distance, Intersection, Symmetric Difference, Convex Hull, Envelope, Buffer, Simplify, Polygon Assembly, Valid, Area, Length,
* Prepared geometries (pre-spatially indexed)
* STR spatial index
* OGC Well Known Text (WKT) and Well Known Binary (WKB) encoders and decoders.

## API Features

* [C API]({{< ref "usage/c_api" >}}) (provides long-term API and ABI stability)
* [C++ API]({{< ref "usage/cpp_api" >}}) (will likely change across versions)
* Thread safety (using the [reentrant API]({{<ref "usage/c_api/#reentrantthreadsafe-api" >}}))

## License

GEOS is [open source software](https://opensource.com/resources/what-open-source) available under the terms of [GNU Lesser General Public License](http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html) (LGPL).

## Citation

To cite GEOS in publications use:

> GEOS contributors (2021). GEOS computational geometry library. Open Source Geospatial Foundation. URL https://libgeos.org/.

A BibTeX entry for LaTeX users is
```
@Manual{,
  title = {{GEOS} coordinate transformation software library},
  author = {{GEOS contributors}},
  organization = {Open Source Geospatial Foundation},
  year = {2021},
  url = {https://libgeos.org/},
}
```

