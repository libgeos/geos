/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2011      Sandro Santilli <strk@kbt.io>
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
 * Last port: noding/SegmentString.java r430 (JTS-1.12+)
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/noding/Octant.h>

#include <vector>

// Forward declarations
namespace geos {
namespace algorithm {
class LineIntersector;
}
}

namespace geos {
namespace noding { // geos.noding

/** \brief
 * An interface for classes which represent a sequence of contiguous
 * line segments.
 *
 * SegmentStrings can carry a context object, which is useful
 * for preserving topological or parentage information.
 */
class GEOS_DLL SegmentString {
public:
    typedef std::vector<const SegmentString*> ConstVect;
    typedef std::vector<SegmentString*> NonConstVect;

    friend std::ostream& operator<< (std::ostream& os,
                                     const SegmentString& ss);

    /// \brief Construct a SegmentString.
    ///
    /// @param newContext the context associated to this SegmentString
    /// @param newSeq coordinates of this SegmentString
    ///
    SegmentString(const void* newContext, geom::CoordinateSequence* newSeq)
        :
        seq(newSeq),
        context(newContext)
    {}

    virtual
    ~SegmentString() {}

    /** \brief
     * Gets the user-defined data for this segment string.
     *
     * @return the user-defined data
     */
    const void*
    getData() const
    {
        return context;
    }

    /** \brief
     * Sets the user-defined data for this segment string.
     *
     * @param data an Object containing user-defined data
     */
    void
    setData(const void* data)
    {
        context = data;
    }

    std::size_t size() const {
        return seq->size();
    }

    template<typename CoordType = geom::Coordinate>
    const CoordType& getCoordinate(std::size_t i) const {
        return seq->getAt<CoordType>(i);
    }

    /// \brief
    /// Return a pointer to the CoordinateSequence associated
    /// with this SegmentString.
    ///
    /// @note The CoordinateSequence is owned by this SegmentString!
    ///
    const geom::CoordinateSequence* getCoordinates() const {
        return seq;
    }

    geom::CoordinateSequence* getCoordinates() {
        return seq;
    }

    /** \brief
     * Gets the octant of the segment starting at vertex index.
     *
     * @param index the index of the vertex starting the segment.
     *              Must not be the last index in the vertex list
     * @return the octant of the segment at the vertex
     */
    int getSegmentOctant(std::size_t index) const
    {
        if (index >= size() - 1) {
            return -1;
        }
        return safeOctant(seq->getAt<geom::CoordinateXY>(index),
                          seq->getAt<geom::CoordinateXY>(index + 1));
    };

    static int getSegmentOctant(const SegmentString& ss, std::size_t index) {
        return ss.getSegmentOctant(index);
    }

    bool isClosed() const {
        return seq->front<geom::CoordinateXY>().equals(seq->back<geom::CoordinateXY>());
    }

    virtual std::ostream& print(std::ostream& os) const;

protected:
    geom::CoordinateSequence* seq;

private:
    const void* context;

    static int safeOctant(const geom::CoordinateXY& p0, const geom::CoordinateXY& p1)
    {
        if(p0.equals2D(p1)) {
            return 0;
        }
        return Octant::octant(p0, p1);
    };

    // Declare type as noncopyable
    SegmentString(const SegmentString& other) = delete;
    SegmentString& operator=(const SegmentString& rhs) = delete;
};

std::ostream& operator<< (std::ostream& os, const SegmentString& ss);

} // namespace geos.noding
} // namespace geos

