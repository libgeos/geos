---
title: "GEOS"
date: 2021-10-04T14:13:20-07:00
draft: false
geekdocNav: true
---

GEOS is a C/C++ library for [computational geometry](https://en.wikipedia.org/wiki/Computational_geometry) with a focus on algorithms used in
[geographic information systems](https://en.wikipedia.org/wiki/Geographic_information_system) (GIS) software.
It implements the [OGC](https://www.ogc.org/standards/sfa)
[*Simple Features*](https://en.wikipedia.org/wiki/Simple_Features) geometry model and provides all the spatial functions in that standard as well as many others.
GEOS is a core dependency of [PostGIS](https://postgis.net), [QGIS](https://qgis.org), [GDAL](https://gdal.org), [Shapely](https://shapely.readthedocs.io/en/stable/project.html) and [many others]({{< ref "usage/bindings" >}}).

## Capabilities

Spatial Model and Functions

* **Geometry Model**: Point, LineString, Polygon, MultiPoint, MultiLineString, MultiPolygon, GeometryCollection
* **Predicates**: Intersects, Touches, Disjoint, Crosses, Within, Contains, Overlaps, Equals, Covers
* **Operations**: Union, Distance, Intersection, Symmetric Difference, Convex Hull, Envelope, Buffer, Simplify, Polygon Assembly, Valid, Area, Length,
* **Prepared geometry** (using internal spatial indexes)
* **Spatial Indexes**: STR (Sort-Tile-Recursive) packed R-tree spatial index
* **Input/Output**: OGC Well Known Text (WKT) and Well Known Binary (WKB) readers and writers.

## API Features

* [C API]({{< ref "usage/c_api" >}}) (provides long-term API and ABI stability)
* [C++ API]({{< ref "usage/cpp_api" >}}) (will likely change across versions)
* Thread safety (using the [reentrant C API]({{<ref "usage/c_api/#reentrantthreadsafe-api" >}}))

## License

GEOS is [open source software](https://opensource.com/resources/what-open-source) available under the terms of [GNU Lesser General Public License](http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html) (LGPL).


## Governance

The GEOS project is run by a
[Project Steering Committee]({{< ref "psc" >}}) made up of developers
and contributors to the project and is a project of
[OSGeo](https://www.osgeo.org/projects/geos/). We strive to keep our day-to-day interactions collegial and welcoming within a basic [Code of Conduct]({{< ref "coc" >}}).

## Relationship to JTS

GEOS started as a direct port to C++ of the
[JTS Topology Suite](https://github.com/locationtech/jts/) (JTS),
and remains tightly bound to that project. Most core algorithms have
been developed in JTS and ported to GEOS when complete.

The projects share testing data.  This helps to ascertain when failures
are caused by differences in implementation (GEOS fails and JTS does not)
and when they are caused by algorithm (both libraries fail).

## Citation

To cite GEOS in publications use:

> GEOS contributors (2025). GEOS computational geometry library. Open Source Geospatial Foundation. URL https://libgeos.org/.

A BibTeX entry for LaTeX users is
```
@Manual{,
  title = {{GEOS} computational geometry library},
  author = {{GEOS contributors}},
  organization = {Open Source Geospatial Foundation},
  year = {2025},
  url = {https://libgeos.org/},
  doi = {10.5281/zenodo.11396894}
}
```
