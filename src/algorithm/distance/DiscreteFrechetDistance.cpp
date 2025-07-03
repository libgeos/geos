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

/**
 * Computes the Discrete Fréchet Distance between two {@link Geometry}s
 * using a {@code Cartesian} distance computation function.
 *
 * @param geom0 the 1st geometry
 * @param geom1 the 2nd geometry
 * @return the cartesian distance between {#g0} and {#g1}
 */
/* public static */
double
DiscreteFrechetDistance::distance(const Geometry& geom0, const Geometry& geom1)
{
    DiscreteFrechetDistance dist(geom0, geom1);
    return dist.distance();
}


/**
 * Computes the {@code Discrete Fréchet Distance} between the input geometries
 *
 * @return the Discrete Fréchet Distance
 */
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

    std::unique_ptr<CoordinateSequence> coords0 = g0.getCoordinates();
    std::unique_ptr<CoordinateSequence> coords1 = g1.getCoordinates();

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
    // !(dFrac > 0) written that way to catch NaN
    // and test on 1.0/dFrac to avoid a potential later undefined behaviour
    // when casting to std::size_t
    bool isUnusableFraction = util::round(1.0 / dFrac) > static_cast<double>(std::numeric_limits<std::size_t>::max());
    if (std::isnan(dFrac) || dFrac > 1.0 || dFrac < 0.0 || isUnusableFraction) {
        throw util::IllegalArgumentException(
            "Fraction is not in range (0.0 - 1.0]");
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

/**
 * Creates a matrix to store the computed distances.
 *
 * @param rows the number of rows
 * @param cols the number of columns
 * @return a matrix storage
 */
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

/**
 * Gets the pair of {@link Coordinate}s at which the distance is obtained.
 *
 * @return the pair of Coordinates at which the distance is obtained
 */
/* public */
std::array<CoordinateXY, 2>
DiscreteFrechetDistance::getCoordinates()
{
    if (ptDist == nullptr)
        distance();

    return ptDist->getCoordinates();
}

/**
 * Computes the Fréchet Distance for the given distance matrix.
 *
 * @param coords0 an array of {@code Coordinate}s.
 * @param coords1 an array of {@code Coordinate}s.
 * @param diagonal an array of alternating col/row index values for the diagonal of the distance matrix
 * @param distances the distance matrix
 * @param distanceToPair a lookup for coordinate pairs based on a distance
 *
 */
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

/**
 * Returns the minimum distance at the corner ({@code i, j}).
 *
 * @param matrix A sparse matrix
 * @param i the column index
 * @param j the row index
 * @return the minimum distance
 */
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


/**
 * Computes relevant distances between pairs of {@link Coordinate}s for the
 * computation of the {@code Discrete Fréchet Distance}.
 *
 * @param coords0 an array of {@code Coordinate}s.
 * @param coords1 an array of {@code Coordinate}s.
 * @param diagonal an array of alternating col/row index values for the diagonal of the distance matrix
 * @param distances the distance matrix
 * @param distanceToPair a lookup for coordinate pairs based on a distance
 */
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

    //System.out.println(distances.toString());
}

/**
 * Computes the indices for the diagonal of a {@code numCols x numRows} grid
 * using the <a href=https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm>
 * Bresenham line algorithm</a>.
 *
 * @param numCols the number of columns
 * @param numRows the number of rows
 * @return a packed array of column and row indices
 */
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

/* Implementation of pure virtual destructor for C++ */
DiscreteFrechetDistance::MatrixStorage::~MatrixStorage() {};

}
}
};
