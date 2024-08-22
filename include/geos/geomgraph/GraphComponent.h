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
 * Last port: geomgraph/GraphComponent.java r428 (JTS-1.12+)
 *
 **********************************************************************/


#pragma once

#include <geos/export.h>

#include <geos/geomgraph/Label.h>

// Forward declarations
namespace geos {
namespace geom {
class IntersectionMatrix;
}
}

namespace geos {
namespace geomgraph { // geos.geomgraph


/** \brief
 * A GraphComponent is the parent class for the objects'
 * that form a graph.
 *
 * Each GraphComponent can carry a Label.
 */
class GEOS_DLL GraphComponent /* non-final */ {
public:
    GraphComponent();

    /*
     * GraphComponent copies the given Label.
     */
    GraphComponent(const Label& newLabel);

    virtual ~GraphComponent() = default;

    Label&
    getLabel()
    {
        return label;
    }
    const Label&
    getLabel() const
    {
        return label;
    }
    void
    setLabel(const Label& newLabel)
    {
        label = newLabel;
    }

    void
    setInResult(bool p_isInResult)
    {
        isInResultVar = p_isInResult;
    }
    bool
    isInResult() const
    {
        return isInResultVar;
    }
    void setCovered(bool isCovered);
    bool
    isCovered() const
    {
        return isCoveredVar;
    }
    bool
    isCoveredSet() const
    {
        return isCoveredSetVar;
    }
    bool
    isVisited() const
    {
        return isVisitedVar;
    }
    void
    setVisited(bool p_isVisited)
    {
        isVisitedVar = p_isVisited;
    }
    virtual bool isIsolated() const = 0;
    void updateIM(geom::IntersectionMatrix& im);
protected:
    Label label;
    virtual void computeIM(geom::IntersectionMatrix& im) = 0;
private:
    bool isInResultVar;
    bool isCoveredVar;
    bool isCoveredSetVar;
    bool isVisitedVar;
};

} // namespace geos.geomgraph
} // namespace geos

