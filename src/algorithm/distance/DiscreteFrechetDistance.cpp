/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (c) 2021 Felix Obermaier
 * Copyright (c) 2025 Paul Ramsey
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/algorithm/distance/DiscreteFrechetDistance.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Geometry.h>
#include <geos/util/IllegalStateException.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/util.h>
#include <geos/util/math.h>

#include <unordered_map>

using geos::geom::Geometry;
using geos::geom::CoordinateSequence;
using geos::geom::CoordinateXY;

namespace geos {
namespace algorithm { // geos.algorithm
namespace distance {  // geos.algorithm.distance


/* public static */
double
DiscreteFrechetDistance::distance(const Geometry& geom0, const Geometry& geom1)
{
    DiscreteFrechetDistance dist(geom0, geom1);
    return dist.distance();
}


/* public static */
double
DiscreteFrechetDistance::distance(const Geometry& geom0, const Geometry& geom1, double densityFrac)
{
    DiscreteFrechetDistance dist(geom0, geom1);
    dist.setDensifyFraction(densityFrac);
    return dist.distance();
}


/* private */
double
DiscreteFrechetDistance::distance()
{
    if (g0.isEmpty() || g1.isEmpty()) {
        throw util::IllegalArgumentException(
            "DiscreteFrechetDistance called with empty inputs.");
    }

    util::ensureNoCurvedComponents(g0);
    util::ensureNoCurvedComponents(g1);

    std::unique_ptr<CoordinateSequence> coords0;
    std::unique_ptr<CoordinateSequence> coords1;
    if (densifyFraction < 0) {
        coords0 = g0.getCoordinates();
        coords1 = g1.getCoordinates();
    }
    else {
        coords0 = getDensifiedCoordinates(g0, densifyFraction);
        coords1 = getDensifiedCoordinates(g1, densifyFraction);
    }

    std::unique_ptr<MatrixStorage> distances = createMatrixStorage(coords0->size(), coords1->size());
    std::vector<std::size_t> diagonal = bresenhamDiagonal(coords0->size(), coords1->size());
    MapDoubleToIndexPair distanceToPair;
    computeCoordinateDistances(*coords0, *coords1, diagonal, *distances, distanceToPair);
    ptDist = computeFrechet(*coords0, *coords1, diagonal, *distances, distanceToPair);

    return ptDist->getDistance();
}

/* public */
void
DiscreteFrechetDistance::setDensifyFraction(double dFrac)
{
    // Densifying too far is too CPU intensive, and
    // we also want to catching NaN and other out-of-range
    // values here.
    if (std::isnan(dFrac) || dFrac > 1.0 || dFrac <= 0.001) {
        throw util::IllegalArgumentException(
            "Fraction is not in range (0.001 - 1.0]");
    }

    densifyFraction = dFrac;
}

void
DiscreteFrechetDistance::putIfAbsent(
    MapDoubleToIndexPair& distanceToPair,
    double key, std::array<std::size_t, 2> val)
{
    auto it = distanceToPair.find(key);
    if (it == distanceToPair.end())
        distanceToPair[key] = val;
}


/* private static */
std::unique_ptr<DiscreteFrechetDistance::MatrixStorage>
DiscreteFrechetDistance::createMatrixStorage(std::size_t rows, std::size_t cols)
{
    std::size_t max = std::max(rows, cols);
    double inf = std::numeric_limits<double>::infinity();
    // NOTE: these constraints need to be verified
    if (max < 1024)
        return std::make_unique<RectMatrix>(rows, cols, inf);

    return std::make_unique<CsrMatrix>(rows, cols, inf);
}


/* public */
std::array<CoordinateXY, 2>
DiscreteFrechetDistance::getCoordinates()
{
    if (ptDist == nullptr)
        distance();

    return ptDist->getCoordinates();
}


/* private static */
std::unique_ptr<PointPairDistance>
DiscreteFrechetDistance::computeFrechet(
    const CoordinateSequence& coords0, const CoordinateSequence& coords1,
    std::vector<std::size_t>& diagonal,
    MatrixStorage& distances,
    MapDoubleToIndexPair& distanceToPair)
{
    for (std::size_t d = 0; d < diagonal.size(); d += 2) {
        std::size_t i0 = diagonal[d];
        std::size_t j0 = diagonal[d + 1];

        for (std::size_t i = i0; i < coords0.size(); i++) {
            if (distances.isValueSet(i, j0)) {
                double dist = getMinDistanceAtCorner(distances, i, j0);
                if (dist > distances.get(i, j0))
                    distances.set(i, j0, dist);
            }
            else {
                break;
            }
        }
        for (std::size_t j = j0 + 1; j < coords1.size(); j++) {
            if (distances.isValueSet(i0, j)) {
                double dist = getMinDistanceAtCorner(distances, i0, j);
                if (dist > distances.get(i0, j))
                    distances.set(i0, j, dist);
            }
            else {
                break;
            }
        }
    }

    double distance = distances.get(coords0.size()-1, coords1.size()-1);
    auto it = distanceToPair.find(distance);
    if (it == distanceToPair.end()) {
        throw geos::util::IllegalStateException("Pair of points not recorded for computed distance");
    }
    std::array<std::size_t, 2> index = (*it).second;
    const CoordinateXY& c0 = coords0.getAt(index[0]);
    const CoordinateXY& c1 = coords1.getAt(index[1]);
    auto result = std::make_unique<PointPairDistance>();
    result->initialize(c0, c1, distance * distance);
    return result;
}


/* private static */
double
DiscreteFrechetDistance::getMinDistanceAtCorner(MatrixStorage& matrix, std::size_t i, std::size_t j)
{
    if (i > 0 && j > 0) {
        double d0 = matrix.get(i - 1, j - 1);
        double d1 = matrix.get(i - 1, j);
        double d2 = matrix.get(i, j - 1);
        return std::min(std::min(d0, d1), d2);
    }
    if (i == 0 && j == 0)
        return matrix.get(0, 0);

    if (i == 0)
        return matrix.get(0, j - 1);

    // j == 0
    return matrix.get(i - 1, 0);
}


/* private */
void
DiscreteFrechetDistance::computeCoordinateDistances(
    const CoordinateSequence& coords0, const CoordinateSequence& coords1,
    std::vector<std::size_t>& diagonal,
    MatrixStorage& distances, MapDoubleToIndexPair& distanceToPair)
{
    std::size_t numDiag = diagonal.size();
    double maxDistOnDiag = 0.0;
    std::size_t imin = 0, jmin = 0;
    std::size_t numCoords0 = coords0.size();
    std::size_t numCoords1 = coords1.size();

    // First compute all the distances along the diagonal.
    // Record the maximum distance.

    for (std::size_t k = 0; k < numDiag; k += 2) {
        std::size_t i0 = diagonal[k];
        std::size_t j0 = diagonal[k + 1];
        double diagDist = coords0[i0].distance(coords1[j0]);
        if (diagDist > maxDistOnDiag) maxDistOnDiag = diagDist;
        distances.set(i0, j0, diagDist);
        // TBD convert to std::map
        putIfAbsent(distanceToPair, diagDist, {i0, j0});
    }

    // Check for distances shorter than maxDistOnDiag along the diagonal
    for (std::size_t k = 0; k < numDiag - 2; k += 2) {
        // Decode index
        std::size_t i0 = diagonal[k];
        std::size_t j0 = diagonal[k + 1];

        // Get reference coordinates for col and row
        const CoordinateXY& coord0 = coords0.getAt(i0);
        const CoordinateXY& coord1 = coords1.getAt(j0);

        // Check for shorter distances in this row
        std::size_t i = i0 + 1;
        for (; i < numCoords0; i++) {
            if (!distances.isValueSet(i, j0)) {
                double dist = coords0[i].distance(coord1);
                if (dist < maxDistOnDiag || i < imin) {
                    distances.set(i, j0, dist);
                    putIfAbsent(distanceToPair, dist, {i, j0});
                }
                else {
                    break;
                }
            }
            else {
                break;
            }
        }
        imin = i;

        // Check for shorter distances in this column
        std::size_t j = j0 + 1;
        for (; j < numCoords1; j++) {
            if (!distances.isValueSet(i0, j)) {
                double dist = coord0.distance(coords1[j]);
                if (dist < maxDistOnDiag || j < jmin)
                {
                    distances.set(i0, j, dist);
                    putIfAbsent(distanceToPair, dist, {i0, j});
                }
                else
                    break;
            }
            else {
                break;
            }
        }
        jmin = j;
    }
}


/* static */
std::vector<std::size_t>
DiscreteFrechetDistance::bresenhamDiagonal(std::size_t numCols, std::size_t numRows)
{
    std::size_t dim = std::max(numCols, numRows);
    std::vector<std::size_t> diagXY;
    diagXY.reserve(2 * dim);

    int64_t dx = static_cast<int64_t>(numCols) - 1;
    int64_t dy = static_cast<int64_t>(numRows) - 1;
    int64_t err;
    if (numCols > numRows) {
        std::size_t y = 0;
        err = 2 * dy - dx;
        for (std::size_t x = 0; x < numCols; x++) {
            diagXY.push_back(x);
            diagXY.push_back(y);
            if (err > 0) {
                y += 1;
                err -= 2 * dx;
            }
            err += 2 * dy;
        }
    }
    else {
        std::size_t x = 0;
        err = 2 * dx - dy;
        for (std::size_t y = 0; y < numRows; y++) {
            diagXY.push_back(x);
            diagXY.push_back(y);
            if (err > 0) {
                x += 1;
                err -= 2 * dy;
            }
            err += 2 * dx;
        }
    }
    return diagXY;
}

/* private static */
std::unique_ptr<CoordinateSequence>
DiscreteFrechetDistance::getDensifiedCoordinates(const Geometry& geom, double densifyFrac)
{
    auto coords = std::make_unique<CoordinateSequence>();
    DensifiedCoordinatesFilter dcf(densifyFrac, *coords);
    geom.apply_ro(dcf);

    return coords;
}

void
DiscreteFrechetDistance::DensifiedCoordinatesFilter::filter_ro(
    const geom::CoordinateSequence& seq, std::size_t index)
{
    // Skip the first coordinate so we can operate an edge
    // at a time
    if (index == 0)
        return;

    const geom::Coordinate& p0 = seq.getAt(index - 1);
    const geom::Coordinate& p1 = seq.getAt(index);

    double dx = (p1.x - p0.x) / m_numSubSegs;
    double dy = (p1.y - p0.y) / m_numSubSegs;

    for(uint32_t i = 0; i < m_numSubSegs; i++) {
        geom::CoordinateXY pt(p0.x + i * dx, p0.y + i * dy);
        m_coords.add(pt, true); // allowRepeated
    }

    // Include the final point in the sequence
    if (index == seq.size() - 1)
        m_coords.add(p1, true); // allowRepeated
}


/* Implementation of pure virtual destructor for C++ */
DiscreteFrechetDistance::MatrixStorage::~MatrixStorage() {};

}
}
};
