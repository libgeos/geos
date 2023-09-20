/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2011 Sandro Santilli <strk@kbt.io>
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: geom/PrecisionModel.java r378 (JTS-1.12)
 *
 **********************************************************************/

#pragma once

#include <geos/geom/Coordinate.h>
#include <geos/export.h>

#include <cassert>
#include <string>

// Forward declarations
namespace geos {
namespace io {
class Unload;
}
namespace geom {
class Coordinate;
}
}

namespace geos {
namespace geom { // geos::geom

/**
 * \class PrecisionModel geom.h geos.h
 *
 * \brief Specifies the precision model of the Coordinate in a Geometry.
 *
 * In other words, specifies the grid of allowable points for a <code>Geometry</code>.
 * A precision model may be <b>floating</b> (PrecisionModel::Type::FLOATING or
 * PrecisionModel::Type::FLOATING_SINGLE), in which case normal floating-point value semantics apply.
 *
 * For a PrecisionModel::Type::FIXED precision model the
 * makePrecise(geom::Coordinate) method allows rounding a coordinate to
 * a "precise" value; that is, one whose precision is known exactly.
 *
 * Coordinates are assumed to be precise in geometries.
 * That is, the coordinates are assumed to be rounded to the
 * precision model given for the geometry.
 * All internal operations
 * assume that coordinates are rounded to the precision model.
 * Constructive methods (such as boolean operations) always round computed
 * coordinates to the appropriate precision model.
 *
 * Three types of precision model are supported:
 * - FLOATING - represents full double precision floating point.
 *   This is the default precision model used in JTS
 * - FLOATING_SINGLE - represents single precision floating point.
 * - FIXED - represents a model with a fixed number of decimal places.
 *   A Fixed Precision Model is specified by a scale factor.
 *   The scale factor specifies the grid which numbers are rounded to.
 *   Input coordinates are mapped to fixed coordinates according to the
 *   following equations:
 *   - jtsPt.x = round( inputPt.x * scale ) / scale
 *   - jtsPt.y = round( inputPt.y * scale ) / scale
 *
 * For example, to specify 3 decimal places of precision, use a scale factor
 * of 1000. To specify -3 decimal places of precision (i.e. rounding to
 * the nearest 1000), use a scale factor of 0.001.
 *
 * It is also supported to specify a precise grid size
 * by providing it as a negative scale factor.
 * For example, to specify rounding to the nearest 1000 use a scale factor of -1000.
 *
 * Coordinates are represented internally as Java double-precision values.
 * Java uses the IEEE-394 floating point standard, which
 * provides 53 bits of precision. (Thus the maximum precisely representable
 * integer is 9,007,199,254,740,992).
 *
 */
class GEOS_DLL PrecisionModel {
    friend class io::Unload;

public:

    /// The types of Precision Model which GEOS supports.
    typedef enum {

        /**
         * Fixed Precision indicates that coordinates have a fixed
         * number of decimal places.
         * The number of decimal places is determined by the log10
         * of the scale factor.
         */
        FIXED,

        /**
         * Floating precision corresponds to the standard Java
         * double-precision floating-point representation, which is
         * based on the IEEE-754 standard
         */
        FLOATING,

        /**
         * Floating single precision corresponds to the standard Java
         * single-precision floating-point representation, which is
         * based on the IEEE-754 standard
         */
        FLOATING_SINGLE

    } Type;

    /// Creates a PrecisionModel with a default precision of FLOATING.
    PrecisionModel(void);

    /// Creates a PrecisionModel specifying an explicit precision model type.
    ///
    /// If the model type is FIXED the scale factor will default to 1.
    ///
    /// @param nModelType the type of the precision model
    ///
    PrecisionModel(Type nModelType);

    /** \brief
     * Creates a <code>PrecisionModel</code> with Fixed precision.
     *
     * Fixed-precision coordinates are represented as precise internal
     * coordinates, which are rounded to the grid defined by the
     * scale factor.
     *
     * @param  newScale  amount by which to multiply a coordinate after
     *                   subtracting the offset, to obtain a precise coordinate
     * @param  newOffsetX  not used.
     * @param  newOffsetY  not used.
     *
     * @deprecated offsets are no longer supported, since internal
     * representation is rounded floating point
     */
    PrecisionModel(double newScale, double newOffsetX, double newOffsetY);

    /**
     * \brief
     * Creates a PrecisionModel with Fixed precision.
     *
     * Fixed-precision coordinates are represented as precise
     * internal coordinates which are rounded to the grid defined
     * by the scale factor.
     * The provided scale may be negative, to specify an exact grid size.
     * The scale is then computed as the reciprocal.
     *
     * @param newScale amount by which to multiply a coordinate
     * after subtracting the offset, to obtain a precise coordinate. Must be non-zero.
     */
    PrecisionModel(double newScale);

    /// The maximum precise value representable in a double.
    ///
    /// Since IEE754 double-precision numbers allow 53 bits of mantissa,
    /// the value is equal to 2^53 - 1.
    /// This provides <i>almost</i> 16 decimal digits of precision.
    ////
    static const double maximumPreciseValue;

    /** \brief
     * Rounds a numeric value to the PrecisionModel grid.
     *
     * Asymmetric Arithmetic Rounding is used, to provide
     * uniform rounding behaviour no matter where the number is
     * on the number line.
     *
     * <b>Note:</b> Java's <code>Math#rint</code> uses the "Banker's Rounding" algorithm,
     * which is not suitable for precision operations elsewhere in JTS.
     */
    double makePrecise(double val) const;

    /// Rounds the given Coordinate to the PrecisionModel grid.
    void makePrecise(CoordinateXY& coord) const
    {
        // optimization for full precision
        if(modelType == FLOATING) {
            return;
        }

        coord.x = makePrecise(coord.x);
        coord.y = makePrecise(coord.y);
    };

    void makePrecise(CoordinateXY* coord) const
    {
        assert(coord);
        return makePrecise(*coord);
    };

    /// Tests whether the precision model supports floating point
    ///
    /// @return <code>true</code> if the precision model supports
    /// floating point
    ///
    bool isFloating() const;

    /// \brief
    /// Returns the maximum number of significant digits provided by
    /// this precision model.
    ///
    /// Intended for use by routines which need to print out precise
    /// values.
    ///
    /// @return the maximum number of decimal places provided by this
    /// precision model
    ///
    int getMaximumSignificantDigits() const;

    /// Gets the type of this PrecisionModel
    ///
    /// @return the type of this PrecisionModel
    ///
    Type getType() const
    {
        return modelType;
    };

    /// Returns the multiplying factor used to obtain a precise coordinate.
    double getScale() const
    {
        assert(!(scale < 0));
        return scale;
    };

    /**
    * Computes the grid size for a fixed precision model.
    * This is equal to the reciprocal of the scale factor.
    * If the grid size has been set explicity (via a negative scale factor)
    * it will be returned.
    *
    * @return the grid size at a fixed precision scale.
    */
    double getGridSize() const
    {
        if (isFloating())
           return DoubleNotANumber;

        if (gridSize != 0)
            return gridSize;

        return 1.0 / scale;
    };

    /// Returns the x-offset used to obtain a precise coordinate.
    ///
    /// @return the amount by which to subtract the x-coordinate before
    ///         multiplying by the scale
    /// @deprecated Offsets are no longer used
    ///
    double getOffsetX() const;

    /// Returns the y-offset used to obtain a precise coordinate.
    ///
    /// @return the amount by which to subtract the y-coordinate before
    ///         multiplying by the scale
    /// @deprecated Offsets are no longer used
    ///
    double getOffsetY() const;

    /*
     *  Sets ´internal` to the precise representation of `external`.
     *
     * @param external the original coordinate
     * @param internal the coordinate whose values will be changed to the
     *                 precise representation of <code>external</code>
     * @deprecated use makePrecise instead
     */
    //void toInternal(const Coordinate& external, Coordinate* internal) const;

    /*
     *  Returns the precise representation of <code>external</code>.
     *
     *@param  external  the original coordinate
     *@return
     *	the coordinate whose values will be changed to the precise
     *	representation of <code>external</code>
     * @deprecated use makePrecise instead
     */
    //Coordinate* toInternal(const Coordinate& external) const;

    /*
     *  Returns the external representation of <code>internal</code>.
     *
     *@param  internal  the original coordinate
     *@return           the coordinate whose values will be changed to the
     *      external representation of <code>internal</code>
     * @deprecated no longer needed, since internal representation is same as external representation
     */
    //Coordinate* toExternal(const Coordinate& internal) const;

    /*
     *  Sets <code>external</code> to the external representation of
     *  <code>internal</code>.
     *
     * @param  internal  the original coordinate
     * @param  external
     *	the coordinate whose values will be changed to the
     *	external representation of <code>internal</code>
     * @deprecated no longer needed, since internal representation is same as external representation
     */
    //void toExternal(const Coordinate& internal, Coordinate* external) const;

    std::string toString() const;

    /// \brief
    /// Compares this PrecisionModel object with the specified object
    /// for order.
    ///
    /// A PrecisionModel is greater than another if it provides greater
    /// precision.
    /// The comparison is based on the value returned by the
    /// getMaximumSignificantDigits method.
    /// This comparison is not strictly accurate when comparing floating
    /// precision models to fixed models;
    /// however, it is correct when both models are either floating or
    /// fixed.
    ///
    /// @param other the PrecisionModel with which this PrecisionModel
    ///      is being compared
    /// @return a negative integer, zero, or a positive integer as this
    ///      PrecisionModel is less than, equal to, or greater than the
    ///      specified PrecisionModel.
    ///
    int compareTo(const PrecisionModel* other) const;

private:

    /** \brief
     * Sets the multiplying factor used to obtain a precise coordinate.
     *
     * This method is private because PrecisionModel is intended to
     * be an immutable (value) type.
     *
     */
    void setScale(double newScale);
    // throw IllegalArgumentException

    /** \brief
     * Snaps a value to nearest integer, if within tolerance.
     */
    static double snapToInt(double val, double tolerance);

    Type modelType;

    /**
    * The scale factor which determines the number of decimal places in fixed precision.
    */
    double scale;

    /**
    * If non-zero, the precise grid size specified.
    * In this case, the scale is also valid and is computed from the grid size.
    * If zero, the scale is used to compute the grid size where needed.
    */
    double gridSize = 0.0;

};

// Equality operator for PrecisionModel, deprecate it ?
//inline bool operator==(const PrecisionModel& a, const PrecisionModel& b);

} // namespace geos::geom
} // namespace geos
