/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once


#include <geos/export.h>
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/geom/CoordinateSequenceFactory.h> // for inheritance
#include <vector>



// Forward declarations
namespace geos {
namespace geom {
class Coordinate;
}
}

namespace geos {
namespace geom { // geos::geom

/**
 * \class CoordinateArraySequenceFactory geom.h geos.h
 *
 * \brief
 * Creates CoordinateSequences internally represented as an array of
 * Coordinates.
 */
class GEOS_DLL CoordinateArraySequenceFactory: public CoordinateSequenceFactory {

public:
    std::unique_ptr<CoordinateSequence> create() const override;

    std::unique_ptr<CoordinateSequence> create(
        std::vector<Coordinate>* coords,
        size_t dimension) const override
    {
        return std::unique_ptr<CoordinateSequence>(
            new CoordinateArraySequence(coords, dimension));
    };

    std::unique_ptr<CoordinateSequence> create(
        std::vector<Coordinate> && coords,
        size_t dimension) const override
    {
        return std::unique_ptr<CoordinateSequence>(new CoordinateArraySequence(std::move(coords), dimension));
    };

    /** @see CoordinateSequenceFactory::create(std::size_t, int) */
    std::unique_ptr<CoordinateSequence> create(std::size_t size, std::size_t dimension) const override
    {
        return std::unique_ptr<CoordinateSequence>(
            new CoordinateArraySequence(size, dimension));
    };

    std::unique_ptr<CoordinateSequence> create(const CoordinateSequence& seq) const override
    {
        return std::unique_ptr<CoordinateSequence>(
            new CoordinateArraySequence(seq));
    };

    /** \brief
     * Returns the singleton instance of CoordinateArraySequenceFactory
     */
    static const CoordinateSequenceFactory* instance();
};

/// This is for backward API compatibility
typedef CoordinateArraySequenceFactory DefaultCoordinateSequenceFactory;

} // namespace geos::geom
} // namespace geos










