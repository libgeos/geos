/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2024 ISciences, LLC
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

namespace geos {
namespace geom {

class Curve;
class CurvePolygon;
class GeometryCollection;
class LineString;
class LinearRing;
class MultiCurve;
class MultiLineString;
class MultiPoint;
class MultiPolygon;
class MultiSurface;
class Point;
class Polygon;
class SimpleCurve;
class Surface;

// These structures allow templates to have compile-time access to a type's human-readable name.
template<typename T>
struct GeometryTypeName {};

template<>
struct GeometryTypeName<geom::Curve> {
    static constexpr const char* name = "Curve";
};

template<>
struct GeometryTypeName<geom::CurvePolygon> {
    static constexpr const char* name = "CurvePolygon";
};

template<>
struct GeometryTypeName<geom::GeometryCollection> {
    static constexpr const char* name = "GeometryCollection";
};

template<>
struct GeometryTypeName<geom::LineString> {
    static constexpr const char* name = "LineString";
};

template<>
struct GeometryTypeName<geom::LinearRing> {
    static constexpr const char* name = "LinearRing";
};

template<>
struct GeometryTypeName<geom::MultiCurve> {
    static constexpr const char* name = "MultiCurve";
};

template<>
struct GeometryTypeName<geom::MultiLineString> {
    static constexpr const char* name = "MultiLineString";
};

template<>
struct GeometryTypeName<geom::MultiPoint> {
    static constexpr const char* name = "MultiPoint";
};

template<>
struct GeometryTypeName<geom::MultiPolygon> {
    static constexpr const char* name = "MultiPolygon";
};

template<>
struct GeometryTypeName<geom::MultiSurface> {
    static constexpr const char* name = "MultiSurface";
};

template<>
struct GeometryTypeName<geom::Point> {
    static constexpr const char* name = "Point";
};

template<>
struct GeometryTypeName<geom::Polygon> {
    static constexpr const char* name = "Polygon";
};

template<>
struct GeometryTypeName<geom::SimpleCurve> {
    static constexpr const char* name = "SimpleCurve";
};

template<>
struct GeometryTypeName<geom::Surface> {
    static constexpr const char* name = "Surface";
};

}
}
