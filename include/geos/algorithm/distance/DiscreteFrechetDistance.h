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

#pragma once

#include <geos/export.h>
#include <geos/algorithm/distance/PointPairDistance.h>
// #include <geos/algorithm/distance/DistanceToPoint.h> // for composition
//#include <geos/util/IllegalArgumentException.h> // for inlines
//#include <geos/geom/Geometry.h> // for inlines
//#include <geos/util/math.h> // for inlines
//#include <geos/geom/CoordinateFilter.h> // for inheritance
//#include <geos/geom/CoordinateSequenceFilter.h> // for inheritance

#include <cstdint>
#include <unordered_map>
#include <array>
#include <vector>
#include <memory>

// Forward declarations
namespace geos {
namespace geom {
class Geometry;
class Coordinate;
class CoordinateSequence;
}
}

namespace geos {
namespace algorithm { // geos::algorithm
namespace distance {  // geos::algorithm::distance

typedef std::unordered_map<double, std::array<std::size_t, 2>> MapDoubleToIndexPair;


/**
 * The Fréchet distance is a measure of similarity between curves. Thus, it can
 * be used like the Hausdorff distance.
 *
 * An analogy for the Fréchet distance taken from
 * <a href="http://www.kr.tuwien.ac.at/staff/eiter/et-archive/cdtr9464.pdf">
 *   Computing Discrete Fréchet Distance</a>
 * <pre>
 * A man is walking a dog on a leash: the man can move
 * on one curve, the dog on the other; both may vary their
 * speed, but backtracking is not allowed.
 * </pre>
 *
 * Its metric is better than the Hausdorff distance
 * because it takes the directions of the curves into account.
 * It is possible that two curves have a small Hausdorff but a large
 * Fréchet distance.
 *
 * This implementation is base on the following optimized Fréchet distance algorithm:
 * Thomas Devogele, Maxence Esnault, Laurent Etienne. Distance discrète de Fréchet optimisée. Spatial
 * Analysis and Geomatics (SAGEO), Nov 2016, Nice, France. hal-02110055
 *
 * Several matrix storage implementations are provided
 *
 *   * <a href="https://en.wikipedia.org/wiki/Fr%C3%A9chet_distance">Fréchet distance</a>
 *   * <a href="http://www.kr.tuwien.ac.at/staff/eiter/et-archive/cdtr9464.pdf">
 *     Computing Discrete Fréchet Distance</a>
 *   * <a href="https://hal.archives-ouvertes.fr/hal-02110055/document">Distance discrète de Fréchet optimisée</a>
 *   * <a href="https://towardsdatascience.com/fast-discrete-fr%C3%A9chet-distance-d6b422a8fb77">
 *     Fast Discrete Fréchet Distance</a>
 *
 * @see DiscreteHausdorffDistance
 */
class GEOS_DLL DiscreteFrechetDistance {

public:

    /**
     * Creates an instance of this class using the provided geometries.
     *
     * @param geom0 a geometry
     * @param geom1 a geometry
     */
    DiscreteFrechetDistance(const geom::Geometry& geom0, const geom::Geometry& geom1)
        : g0(geom0)
        , g1(geom1) {};


    /**
    * Abstract base class for storing 2d matrix data
    */
    class GEOS_DLL MatrixStorage {

        public:

            std::size_t m_numRows;
            std::size_t m_numCols;
            double m_defaultValue;

            /**
             * Creates an instance of this class
             * @param numRows the number of rows
             * @param numCols the number of columns
             * @param defaultValue A default value
             */
            MatrixStorage(std::size_t numRows, std::size_t numCols, double defaultValue)
                : m_numRows(numRows)
                , m_numCols(numCols)
                , m_defaultValue(defaultValue) {};

            virtual ~MatrixStorage() = 0;

            /**
             * Gets the matrix value at i, j
             * @param i the row index
             * @param j the column index
             * @return The matrix value at i, j
             */
            virtual double get(std::size_t i, std::size_t j) const = 0;

            /**
             * Sets the matrix value at i, j
             * @param i the row index
             * @param j the column index
             * @param value The matrix value to set at i, j
             */
            virtual void set(std::size_t i, std::size_t j, double value) = 0;

            /**
             * Gets a flag indicating if the matrix has a set value, e.g. one that is different
             * than MatrixStorage defaultValue.
             * @param i the row index
             * @param j the column index
             * @return a flag indicating if the matrix has a set value
             */
            virtual bool isValueSet(std::size_t i, std::size_t j) const = 0;
    };


    /**
     * Straight forward implementation of a rectangular matrix
     */
    class RectMatrix : public MatrixStorage {

        private:
            std::vector<double> m_matrix;

        public:

            /**
             * Creates an instance of this matrix using the given number of rows and columns.
             * A default value can be specified
             *
             * @param numRows the number of rows
             * @param numCols the number of columns
             * @param defaultValue A default value
             */
            RectMatrix(std::size_t numRows, std::size_t numCols, double defaultValue)
                : MatrixStorage(numRows, numCols, defaultValue)
            {
                m_matrix.resize(numRows * numCols, defaultValue);
            };

            double get(std::size_t i, std::size_t j) const override {
                return m_matrix[i * m_numCols + j];
            };

            void set(std::size_t i, std::size_t j, double value) override {
                m_matrix[i * m_numCols + j] = value;
            };

            bool isValueSet(std::size_t i, std::size_t j) const override {
                return get(i, j) != m_defaultValue;
            };
    };


    /**
     * A matrix implementation that adheres to the
     * <a href="https://en.wikipedia.org/wiki/Sparse_matrix#Compressed_sparse_row_(CSR,_CRS_or_Yale_format)">
     *   Compressed sparse row format</a>.<br/>
     * Note: Unfortunately not as fast as expected.
     */
    class CsrMatrix : public MatrixStorage {

        private:
            std::vector<double> m_v;
            std::vector<std::size_t> m_ri;
            std::vector<std::size_t> m_ci;

        public:

            CsrMatrix(std::size_t numRows, std::size_t numCols, double defaultValue, std::size_t expectedValues)
                : MatrixStorage(numRows, numCols, defaultValue)
            {
                m_v.reserve(expectedValues);
                m_ci.reserve(expectedValues);
                m_ri.reserve(numRows + 1);
            };

            CsrMatrix(std::size_t numRows, std::size_t numCols, double defaultValue)
                : CsrMatrix(numRows, numCols, defaultValue, expectedValuesHeuristic(numRows, numCols))
                {};

            /**
             * Computes an initial value for the number of expected values
             * @param numRows the number of rows
             * @param numCols the number of columns
             * @return the expected number of values in the sparse matrix
             */
            static std::size_t expectedValuesHeuristic(std::size_t numRows, std::size_t numCols) {
                std::size_t max = std::max(numRows, numCols);
                return max * max / 10;
            };

            std::pair<bool, std::size_t>
            cppBinarySearch(const std::vector<std::size_t>& vec, std::size_t fromIndex, std::size_t toIndex, std::size_t key) const {
                // Check for invalid range to match Java's behavior for such cases
                if (fromIndex > toIndex || fromIndex < 0 || toIndex > vec.size())
                    return {false, 0};

                // Define the iterators for the sub-range
                auto first_it = vec.begin() + static_cast<std::ptrdiff_t>(fromIndex);
                auto last_it = vec.begin() + static_cast<std::ptrdiff_t>(toIndex);

                // Perform the binary search using std::lower_bound
                auto it = std::lower_bound(first_it, last_it, key);

                // Calculate the index relative to the *beginning of the original vector*
                auto result_index = std::distance(vec.begin(), it);

                // Determine if the element was found and return the appropriate value
                if (it != last_it && *it == key) {
                    // Element found, return its index
                    return {true, result_index};
                } else {
                    // Element not found, return -(insertion point) - 1
                    return {false, 0};
                }
            };

            std::pair<bool, std::size_t> indexOf(std::size_t i, std::size_t j) const {
                std::size_t cLow = m_ri[i];
                std::size_t cHigh = m_ri[i+1];
                if (cHigh <= cLow) return {false, 0};
                return cppBinarySearch(m_ci, cLow, cHigh, j);
            };

            double get(std::size_t i, std::size_t j) const override {
                // get the index in the vector
                std::pair<bool, std::size_t> vi = indexOf(i, j);
                // if the vector index is negative, return default value
                if (!vi.first)
                    return m_defaultValue;

                return m_v[vi.second];
            };

            void set(std::size_t i, std::size_t j, double value) override {

                // get the index in the vector
                std::pair<bool, std::size_t> vi = indexOf(i, j);

                // do we already have a value?
                if (!vi.first)
                {
                    // no, we don't, we need to ensure space!
                    ensureCapacity(m_ri[m_numRows] + 1);

                    // update row indices
                    for (std::size_t ii = i + 1; ii <= m_numRows; ii++)
                        m_ri[ii] += 1;

                    // move and update column indices, move values
                    std::size_t viv = ~(vi.second);
                    for (std::size_t ii = m_ri[m_numRows]; ii > viv; ii--) {
                        m_ci[ii] = m_ci[ii - 1];
                        m_v[ii] = m_v[ii - 1];
                    }

                    // insert column index
                    m_ci[viv] = j;
                }

                // set the new value
                m_v[vi.second] = value;
            };

            bool isValueSet(std::size_t i, std::size_t j) const override {
                auto r = indexOf(i, j);
                return r.first;
            };

            /**
             * Ensures that the column index vector (m_ci) and value vector (m_v) are sufficiently large.
             * @param required the number of items to store in the matrix
             */
            void ensureCapacity(std::size_t required) {
                if (required < m_v.size())
                    return;

                std::size_t increment = std::max(m_numRows, m_numCols);
                m_v.resize(m_v.size() + increment);
                m_ci.resize(m_v.size() + increment);
            };
    };


    /**
     * A sparse matrix based on java's HashMap.
     */
    class HashMapMatrix : public MatrixStorage {

        private:
            std::unordered_map<std::size_t, double> m_matrix;

        public:

        /**
         * Creates an instance of this class
         * @param numRows the number of rows
         * @param numCols the number of columns
         * @param defaultValue a default value
         */
        HashMapMatrix(std::size_t numRows, std::size_t numCols, double defaultValue)
            : MatrixStorage(numRows, numCols, defaultValue)
            {};

        double get(std::size_t i, std::size_t j) const override {
            std::size_t key = i << 32 | j;
            auto it_found = m_matrix.find(key);
            if (it_found != m_matrix.end()) {
                return (*it_found).second;
            }
            else {
                return m_defaultValue;
            }
        };

        void set(std::size_t i, std::size_t j, double value) override {
            std::size_t key = i << 32 | j;
            m_matrix[key] = value;
        };

        bool isValueSet(std::size_t i, std::size_t j) const override {
            std::size_t key = i << 32 | j;
            auto it_found = m_matrix.find(key);
            return it_found != m_matrix.end();
        };
    };


    /**
     * Computes the Discrete Fréchet Distance between two Geometrys
     * using a Cartesian distance computation function.
     *
     * @param geom0 the 1st geometry
     * @param geom1 the 2nd geometry
     * @return the cartesian distance between geom0 and geom1
     */
    static double distance(const geom::Geometry& geom0, const geom::Geometry& geom1);

    /**
     * Gets the pair of {@link geom::Coordinate}s at which the distance is obtained.
     *
     * @return the pair of Coordinates at which the distance is obtained
     */
    std::array<geom::CoordinateXY, 2> getCoordinates();

    void setDensifyFraction(double dFrac);


private:

    // Members
    const geom::Geometry& g0;
    const geom::Geometry& g1;
    std::unique_ptr<PointPairDistance> ptDist;
    double densifyFraction = -1.0;

    /**
     * Computes the {@code Discrete Fréchet Distance} between the input geometries
     *
     * @return the Discrete Fréchet Distance
     */
    /* private */
    double distance();

    /*
     * Utility method to ape Java behaviour
     */
    void putIfAbsent(
        MapDoubleToIndexPair& distanceToPair,
        double key, std::array<std::size_t, 2> val);

    /**
     * Creates a matrix to store the computed distances.
     *
     * @param rows the number of rows
     * @param cols the number of columns
     * @return a matrix storage
     */
    static std::unique_ptr<MatrixStorage> createMatrixStorage(
        std::size_t rows, std::size_t cols);

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
    static std::unique_ptr<PointPairDistance> computeFrechet(
        const geom::CoordinateSequence& coords0,
        const geom::CoordinateSequence& coords1,
        std::vector<std::size_t>& diagonal,
        MatrixStorage& distances,
        MapDoubleToIndexPair& distanceToPair);

    /**
     * Returns the minimum distance at the corner ({@code i, j}).
     *
     * @param matrix A sparse matrix
     * @param i the column index
     * @param j the row index
     * @return the minimum distance
     */
    /* private */
    static double getMinDistanceAtCorner(
        MatrixStorage& matrix, std::size_t i, std::size_t j);

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
    void computeCoordinateDistances(
        const geom::CoordinateSequence& coords0, const geom::CoordinateSequence& coords1,
        std::vector<std::size_t>& diagonal,
        MatrixStorage& distances, MapDoubleToIndexPair& distanceToPair);

    /**
     * Computes the indices for the diagonal of a {@code numCols x numRows} grid
     * using the <a href=https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm>
     * Bresenham line algorithm</a>.
     *
     * @param numCols the number of columns
     * @param numRows the number of rows
     * @return a packed array of column and row indices
     */
    static std::vector<std::size_t> bresenhamDiagonal(
        std::size_t numCols, std::size_t numRows);





}; // DiscreteFrechetDistance

} // geos::algorithm::distance
} // geos::algorithm
} // geos
