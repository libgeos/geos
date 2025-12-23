/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2024-2025 ISciences, LLC
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/LineSegment.h>
#include <geos/geom/Quadrant.h>
#include <geos/algorithm/CircularArcs.h>
#include <geos/algorithm/Orientation.h>

namespace geos {
namespace geom {

/// A CircularArc is a reference to three points that define a circular arc.
/// It provides for the lazy calculation of various arc properties such as the center, radius, and orientation
class GEOS_DLL CircularArc {
public:

    /// Create an empty CircularArc backed by a null CoordinateSequence. A CircularArc created in this way will
    /// crash if methods are called on it, but a default constructor is necessary to construct std::array<CircularArc, >
    CircularArc();

    /// Create a CircularArc that refers to points in the provided CoordinateSequence. The CoordinateSequence is not
    /// owned by the CircularArc and should remain reachable for the lifetime of the CircularArc.
    /// The center, radius, and orientation can be specified to avoid lossy re-computation. They are not checked for
    /// consistency with the references coordinates.
    CircularArc(const CoordinateSequence&, std::size_t pos);
    CircularArc(const CoordinateSequence&, std::size_t pos, const CoordinateXY& center, double radius, int orientation);

    /// Create a CircularArc that refers to points in the provided CoordinateSequence.
    /// Ownership of the CoordinateSequence is transferred to the CircularArc.
    /// The center, radius, and orientation can be specified to avoid lossy re-computation. They are not checked for
    /// consistency with the references coordinates.
    CircularArc(std::unique_ptr<CoordinateSequence>, std::size_t pos);
    CircularArc(std::unique_ptr<CoordinateSequence>, std::size_t pos, const CoordinateXY& center, double radius, int orientation);

    CircularArc(const CircularArc& other);

    CircularArc(CircularArc&&) noexcept;

    CircularArc& operator=(const CircularArc& other);
    CircularArc& operator=(CircularArc&&) noexcept;

    ~CircularArc();

    /// Create a CircularArc from the given coordinates, automatically allocating a CoordinateSequence to store them.
    /// Typically, it is more efficient to create a larger CoordinateSequence that multiple CircularArcs refer to.
    /// However, the "create" methods are convenient for writing tests.
    template<typename CoordType>
    static CircularArc create(const CoordType& p0, const CoordType& p1, const CoordType& p2)
    {
        auto seq = std::make_unique<CoordinateSequence>(3, CoordType::template has<Ordinate::Z>(), CoordType::template has<Ordinate::M>());
        seq->setAt(p0, 0);
        seq->setAt(p1, 1);
        seq->setAt(p2, 2);

        CircularArc ret(std::move(seq), 0);

        return ret;
    }

    static CircularArc create(const CoordinateXY& p0, const CoordinateXY& p2, const CoordinateXY& center, double radius, int orientation);
    static CircularArc create(const Coordinate& p0, const Coordinate& p2, const CoordinateXY& center, double radius, int orientation);
    static CircularArc create(const CoordinateXYM& p0, const CoordinateXYM& p2, const CoordinateXY& center, double radius, int orientation);
    static CircularArc create(const CoordinateXYZM& p0, const CoordinateXYZM& p2, const CoordinateXY& center, double radius, int orientation);

    /// Return the inner angle of the sector associated with this arc
    double getAngle() const;

    /// Return the area enclosed by the arc p0-p1-p2 and the line segment p2-p0
    double getArea() const;

    /// Return the center point of the circle associated with this arc
    const CoordinateXY& getCenter() const {
        if (!m_center_known) {
            if (isCCW()) {
                m_center = algorithm::CircularArcs::getCenter(p0(), p1(), p2());
            } else {
                m_center = algorithm::CircularArcs::getCenter(p2(), p1(), p0());
            }
            m_center_known = true;
        }

        return m_center;
    }

    const CoordinateSequence* getCoordinateSequence() const {
        return m_seq;
    }

    std::size_t getCoordinatePosition() const {
        return m_pos;
    }

    /// Return the length of the arc
    double getLength() const;

    /// Return the orientation of the arc as one of:
    /// - algorithm::Orientation::CLOCKWISE,
    /// - algorithm::Orientation::COUNTERCLOCKWISE
    /// - algorithm::Orientation::COLLINEAR
    int getOrientation() const {
        if (!m_orientation_known) {
            m_orientation = algorithm::Orientation::index(p0(), p1(), p2());
            m_orientation_known = true;
        }
        return m_orientation;
    }

    /// Return the radius of the circle associated with this arc
    double getRadius() const {
        if (!m_radius_known) {
            if (isCCW()) {
                m_radius = getCenter().distance(p0());
            } else {
                m_radius = getCenter().distance(p2());
            }
            m_radius_known = true;
        }

        return m_radius;
    }

    /// Return the distance from the centerpoint of the arc to the line segment formed by the end points of the arc.
    double getSagitta() const {
        CoordinateXY midpoint = algorithm::CircularArcs::getMidpoint(p0(), p2(), getCenter(), getRadius(), isCCW());
        return algorithm::Distance::pointToSegment(midpoint, p0(), p2());
    }

    bool isCCW() const {
        return getOrientation() == algorithm::Orientation::COUNTERCLOCKWISE;
    }

    /// Return whether this arc forms a complete circle
    bool isCircle() const {
        return p0().equals(p2());
    }

    /// Returns whether this arc forms a straight line (p0, p1, and p2 are collinear)
    bool isLinear() const {
        return !std::isfinite(getRadius());
    }

    /// Return the angle of p0
    double theta0() const {
        return algorithm::CircularArcs::getAngle(p0(), getCenter());
    }

    /// Return the angle of p1
    double theta1() const {
        return algorithm::CircularArcs::getAngle(p1(), getCenter());
    }

    /// Return the angle of p2
    double theta2() const {
        return algorithm::CircularArcs::getAngle(p2(), getCenter());
    }
    /// Check to see if a given angle lies on this arc
    bool containsAngle(double theta) const;

    /// Check to see if a coordinate lies on the arc, after testing whether
    /// it lies on the circle.
    bool containsPoint(const CoordinateXY& q) const;

    /// Check to see if a coordinate lies on the arc
    /// Only the angle is checked, so it is assumed that the point lies on
    /// the circle of which this arc is a part.
    bool containsPointOnCircle(const CoordinateXY& q) const {
        double theta = std::atan2(q.y - getCenter().y, q.x - getCenter().x);
        return containsAngle(theta);
    }

    /// Return true if the arc is pointing positive in the y direction
    /// at the location of a specified point. The point is assumed to
    /// be on the arc.
    bool isUpwardAtPoint(const CoordinateXY& q) const;

    CircularArc reverse() const;

    // Split an arc at a specified point.
    // The point is assumed to be on the arc.
    //std::pair<CircularArc, CircularArc> splitAtPoint(const CoordinateXY& q) const;

    bool equals(const CircularArc& other, double tol) const;

    class Iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = geom::CoordinateXY;
        using pointer = const geom::CoordinateXY*;
        using reference = const geom::CoordinateXY&;

        Iterator(const CircularArc& arc, int i) : m_arc(arc), m_i(i) {}

        reference operator*() const {
            return m_i == 0 ? m_arc.p0() : (m_i == 1 ? m_arc.p1() : m_arc.p2());
        }

        Iterator& operator++() {
            m_i++;
            return *this;
        }

        Iterator operator++(int) {
            Iterator ret = *this;
            m_i++;
            return ret;
        }

        bool operator==(const Iterator& other) const {
            return m_i == other.m_i;
        }

        bool operator!=(const Iterator& other) const {
            return !(*this == other);
        }

    private:
        const CircularArc& m_arc;
        int m_i;

    };

    Iterator begin() const {
        return Iterator(*this, 0);
    }

    Iterator end() const {
        return Iterator(*this, 3);
    }

    template<typename T=CoordinateXY>
    const T& p0() const {
        return m_seq->getAt<T>(m_pos);
    }

    template<typename T=CoordinateXY>
    const T& p1() const {
        return m_seq->getAt<T>(m_pos + 1);
    }

    template<typename T=CoordinateXY>
    const T& p2() const {
        return m_seq->getAt<T>(m_pos + 2);
    }

    std::string toString() const;

    template<typename F>
    auto applyAt(std::size_t i, F&& f) const {
        return m_seq->applyAt(m_pos + i, f);
    }

private:
    const CoordinateSequence* m_seq;
    std::size_t m_pos;

    mutable CoordinateXY m_center;
    mutable double m_radius;
    mutable int m_orientation;
    mutable bool m_center_known = false;
    mutable bool m_radius_known = false;
    mutable bool m_orientation_known = false;
    bool m_own_coordinates;
};

}
}
