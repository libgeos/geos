/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2011 Sandro Santilli <strk@kbt.io>
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: geomgraph/EdgeEndStar.java r428 (JTS-1.12+)
 *
 **********************************************************************/


#pragma once

#include <geos/export.h>
#include <geos/geomgraph/EdgeEnd.h>  // for EdgeEndLT
#include <geos/geom/Location.h>
#include <geos/geom/Coordinate.h>  // for p0,p1

#include <array>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include <algorithm> // for inlines (find)

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // warning C4251: needs to have dll-interface to be used by clients of class
#endif

// Forward declarations
namespace geos {
namespace algorithm {
class BoundaryNodeRule;
}
namespace geomgraph {
class GeometryGraph;
}
}

namespace geos {
namespace geomgraph { // geos.geomgraph


/** \brief
 * A EdgeEndStar is an ordered list of EdgeEnds around a node.
 *
 * They are maintained in CCW order (starting with the positive x-axis)
 * around the node for efficient lookup and topology building.
 *
 * @version 1.4
 */
class GEOS_DLL EdgeEndStar /* non-final */ {
public:

    typedef std::set<EdgeEnd*, EdgeEndLT> container;

    typedef container::iterator iterator;
    typedef container::const_iterator const_iterator;
    typedef container::reverse_iterator reverse_iterator;

    EdgeEndStar();

    virtual
    ~EdgeEndStar() {}

    /** \brief
     * Insert a EdgeEnd into this EdgeEndStar
     */
    virtual void insert(EdgeEnd* e) = 0;

    /**
     * @return the coordinate for the node this star is based at
     *         or NULL if this is still an unbound star.
     * Be aware that the returned pointer will point to
     * a Coordinate owned by the specific EdgeEnd happening
     * to be the first in the star (ordered CCW)
     */
    geom::Coordinate& getCoordinate();

    const geom::Coordinate& getCoordinate() const;

    std::size_t getDegree();

    iterator begin();

    iterator end();

    reverse_iterator rbegin();

    reverse_iterator rend();

    const_iterator
    begin() const
    {
        return edgeMap.begin();
    }

    const_iterator
    end() const
    {
        return edgeMap.end();
    }

    container& getEdges();

    EdgeEnd* getNextCW(EdgeEnd* ee);

    virtual void computeLabelling(const std::vector<std::unique_ptr<GeometryGraph>>&geomGraph);
    // throw(TopologyException *);

    bool isAreaLabelsConsistent(const GeometryGraph& geomGraph);

    void propagateSideLabels(uint32_t geomIndex);
    // throw(TopologyException *);

    //virtual int findIndex(EdgeEnd *eSearch);
    iterator find(EdgeEnd* eSearch);

    virtual std::string print() const;

protected:

    /** \brief
     * A map which maintains the edges in sorted order
     * around the node
     */
    EdgeEndStar::container edgeMap;

    /** \brief
     * Insert an EdgeEnd into the map.
     */
    void
    insertEdgeEnd(EdgeEnd* e)
    {
        edgeMap.insert(e);
    }

private:

    geom::Location getLocation(uint32_t geomIndex,
                                       const geom::Coordinate&p,
                                       const std::vector<std::unique_ptr<GeometryGraph>>&geom);

    /** \brief
     * The location of the point for this star in
     * Geometry i Areas
     */
    std::array<geom::Location, 2> ptInAreaLocation;

    void computeEdgeEndLabels(const algorithm::BoundaryNodeRule&);

    bool checkAreaLabelsConsistent(uint32_t geomIndex);

};

inline std::size_t
EdgeEndStar::getDegree()
{
    return edgeMap.size();
}

inline EdgeEndStar::iterator
EdgeEndStar::begin()
{
    return edgeMap.begin();
}

inline EdgeEndStar::container&
EdgeEndStar::getEdges()
{
    return edgeMap;
}

inline EdgeEndStar::reverse_iterator
EdgeEndStar::rend()
{
    return edgeMap.rend();
}

inline EdgeEndStar::iterator
EdgeEndStar::end()
{
    return edgeMap.end();
}

inline EdgeEndStar::reverse_iterator
EdgeEndStar::rbegin()
{
    return edgeMap.rbegin();
}

inline EdgeEndStar::iterator
EdgeEndStar::find(EdgeEnd* eSearch)
{
    return edgeMap.find(eSearch);
}

std::ostream& operator<< (std::ostream&, const EdgeEndStar&);

} // namespace geos.geomgraph
} // namespace geos

#ifdef _MSC_VER
#pragma warning(pop)
#endif

