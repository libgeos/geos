/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2013-2020 Sandro Santilli <strk@kbt.io>
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: ORIGINAL WORK
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/Dimension.h>


#include <memory> // for unique_ptr
#include <vector>


namespace geos {
namespace geom {
class Geometry;
class GeometryFactory;
}
}


namespace geos {
namespace geom { // geos::geom

std::unique_ptr<Geometry> GEOS_DLL
HeuristicOverlay(const Geometry* g0, const Geometry* g1, int opCode);

class StructuredCollection {

public:

    StructuredCollection(const Geometry* g)
        : factory(g->getFactory())
        , pt_union(nullptr)
        , line_union(nullptr)
        , poly_union(nullptr)
    {
        readCollection(g);
        unionByDimension();
    };

    StructuredCollection()
        : factory(nullptr)
        , pt_union(nullptr)
        , line_union(nullptr)
        , poly_union(nullptr)
    {};

    void readCollection(const Geometry* g);
    const Geometry* getPolyUnion()  const { return poly_union.get(); }
    const Geometry* getLineUnion()  const { return line_union.get(); }
    const Geometry* getPointUnion() const { return pt_union.get(); }

    std::unique_ptr<Geometry> doUnion(const StructuredCollection& a) const;
    std::unique_ptr<Geometry> doIntersection(const StructuredCollection& a) const;
    std::unique_ptr<Geometry> doSymDifference(const StructuredCollection& a) const;
    std::unique_ptr<Geometry> doDifference(const StructuredCollection& a) const;
    std::unique_ptr<Geometry> doUnaryUnion() const;

    static void toVector(const Geometry* g, std::vector<const Geometry*>& v);
    void unionByDimension(void);


private:

    const GeometryFactory* factory;
    std::vector<const Geometry*> pts;
    std::vector<const Geometry*> lines;
    std::vector<const Geometry*> polys;
    std::unique_ptr<Geometry> pt_union;
    std::unique_ptr<Geometry> line_union;
    std::unique_ptr<Geometry> poly_union;

};





} // namespace geos::geom
} // namespace geos

