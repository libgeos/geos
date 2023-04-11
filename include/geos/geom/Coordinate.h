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
#include <geos/constants.h> // for DoubleNotANumber
#include <set>
#include <unordered_set>
#include <stack>
#include <vector> // for typedefs
#include <string>
#include <limits>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // warning C4251: needs to have dll-interface to be used by clients of class
#endif

namespace geos {
namespace geom { // geos.geom

// Forward declarations
struct CoordinateLessThan;
class CoordinateXYZM;
class CoordinateXYM;
class Coordinate;

enum class CoordinateType : std::uint8_t {
    XY,
    XYZ,
    XYZM,
    XYM,
};

enum class Ordinate : std::uint8_t {
    X,
    Y,
    Z,
    M
};

GEOS_DLL std::ostream& operator<< (std::ostream&, const CoordinateType);

class GEOS_DLL CoordinateXY {

    const static CoordinateXY _nullCoord;

protected:
    constexpr const static double DEFAULT_X = 0.0;
    constexpr const static double DEFAULT_Y = 0.0;
    constexpr const static double DEFAULT_Z = DoubleNotANumber;
    constexpr const static double DEFAULT_M = DoubleNotANumber;

public:
    CoordinateXY()
        : x(DEFAULT_X)
        , y(DEFAULT_Y)
    {}

    CoordinateXY(double xNew, double yNew)
        : x(xNew)
        , y(yNew)
    {}

    template<Ordinate>
    double get() const;

    /// x-coordinate
    double x;

    /// y-coordinate
    double y;

    /// Equality operator for Coordinate. 2D only.
    GEOS_DLL friend bool operator==(const CoordinateXY& a, const CoordinateXY& b)
    {
        return a.equals2D(b);
    };

    /// Inequality operator for Coordinate. 2D only.
    GEOS_DLL friend bool operator!=(const CoordinateXY& a, const CoordinateXY& b)
    {
        return ! a.equals2D(b);
    };

    bool isValid() const
    {
        return std::isfinite(x) && std::isfinite(y);
    };

    bool equals2D(const CoordinateXY& other) const
    {
        if(x != other.x) {
            return false;
        }
        if(y != other.y) {
            return false;
        }
        return true;
    };

    bool equals2D(const CoordinateXY& other, double tolerance) const
    {
        if (std::abs(x - other.x) > tolerance) {
            return false;
        }
        if (std::abs(y - other.y) > tolerance) {
            return false;
        }
        return true;
    };

    /// 2D only
    bool equals(const CoordinateXY& other) const
    {
        return equals2D(other);
    };

    /// TODO: deprecate this, move logic to CoordinateLessThan instead
    inline int compareTo(const CoordinateXY& other) const
    {
        if(x < other.x) {
            return -1;
        }
        if(x > other.x) {
            return 1;
        }
        if(y < other.y) {
            return -1;
        }
        if(y > other.y) {
            return 1;
        }
        return 0;
    };

    static const CoordinateXY& getNull();

    double distance(const CoordinateXY& p) const
    {
        double dx = x - p.x;
        double dy = y - p.y;
        return std::sqrt(dx * dx + dy * dy);
    };

    double distanceSquared(const CoordinateXY& p) const
    {
        double dx = x - p.x;
        double dy = y - p.y;
        return dx * dx + dy * dy;
    };

    bool isNull() const
    {
        return (std::isnan(x) && std::isnan(y));
    };

    void setNull()
    {
        x = DoubleNotANumber;
        y = DoubleNotANumber;
    };

    struct GEOS_DLL HashCode
    {
        inline std::size_t operator()(const CoordinateXY& c) const
        {
            size_t h = std::hash<double>{}(c.x);
            h ^= std::hash<double>{}(c.y) << 1;
            // z ordinate ignored for consistency with operator==
            return h;
        };
    };

    using UnorderedSet = std::unordered_set<Coordinate, HashCode>;

    ///  Returns a string of the form <I>(x,y,z)</I> .
    std::string toString() const;
};

/**
 * \class Coordinate geom.h geos.h
 *
 * \brief
 * Coordinate is the lightweight class used to store coordinates.
 *
 * It is distinct from Point, which is a subclass of Geometry.
 * Unlike objects of type Point (which contain additional
 * information such as an envelope, a precision model, and spatial
 * reference system information), a Coordinate only contains
 * ordinate values and accessor methods.
 *
 * Coordinate objects are two-dimensional points, with an additional
 * z-ordinate. JTS does not support any operations on the z-ordinate except
 * the basic accessor functions.
 *
 * Constructed coordinates will have a z-ordinate of DoubleNotANumber.
 * The standard comparison functions will ignore the z-ordinate.
 *
 */
// Define the following to make assignments and copy constructions
// NON-(will let profilers report usages)
//#define PROFILE_COORDINATE_COPIES 1
class GEOS_DLL Coordinate : public CoordinateXY {

private:

    static const Coordinate _nullCoord;

public:
    /// A set of const Coordinate pointers
    typedef std::set<const Coordinate*, CoordinateLessThan> ConstSet;

    /// A vector of const Coordinate pointers
    typedef std::vector<const Coordinate*> ConstVect;

    /// A stack of const Coordinate pointers
    typedef std::stack<const Coordinate*> ConstStack;

    /// A vector of Coordinate objects (real object, not pointers)
    typedef std::vector<Coordinate> Vect;

    /// z-coordinate
    double z;

    Coordinate()
        : CoordinateXY()
        , z(DEFAULT_Z)
        {};

    Coordinate(double xNew, double yNew, double zNew = DEFAULT_Z)
        : CoordinateXY(xNew, yNew)
        , z(zNew)
        {};

    explicit Coordinate(const CoordinateXY& other)
        : CoordinateXY(other)
        , z(DEFAULT_Z)
        {};

    template<Ordinate>
    double get() const;

    void setNull()
    {
        CoordinateXY::setNull();
        z = DoubleNotANumber;
    };

    static const Coordinate& getNull();

    bool isNull() const
    {
        return CoordinateXY::isNull() && std::isnan(z);
    };

    /// 3D comparison
    bool equals3D(const Coordinate& other) const
    {
        return (x == other.x) && (y == other.y) &&
               ((z == other.z) || (std::isnan(z) && std::isnan(other.z)));
    };

    ///  Returns a string of the form <I>(x,y,z)</I> .
    std::string toString() const;

    Coordinate& operator=(const CoordinateXY& other){
        x = other.x;
        y = other.y;
        z = DEFAULT_Z;

        return *this;
    }
};


class GEOS_DLL CoordinateXYM : public CoordinateXY {
private:
    static const CoordinateXYM _nullCoord;

public:
    CoordinateXYM() : CoordinateXYM(DEFAULT_X, DEFAULT_Y, DEFAULT_M) {}

    explicit CoordinateXYM(const CoordinateXY& c)
        : CoordinateXY(c)
        , m(DEFAULT_M) {}

    CoordinateXYM(double x_, double y_, double m_)
        : CoordinateXY(x_, y_)
        , m(m_) {}

    double m;

    template<Ordinate>
    double get() const;

    static const CoordinateXYM& getNull();

    void setNull()
    {
        CoordinateXY::setNull();
        m = DoubleNotANumber;
    };

    bool isNull() const
    {
        return CoordinateXY::isNull() && std::isnan(m);
    }
    bool equals3D(const CoordinateXYM& other) const {
        return x == other.x && y == other.y && (m == other.m || (std::isnan(m) && std::isnan(other.m)));
    }

    CoordinateXYM& operator=(const CoordinateXYZM& other);

    CoordinateXYM& operator=(const CoordinateXY& other) {
        x = other.x;
        y = other.y;
        m = DEFAULT_M;

        return *this;
    }

    std::string toString() const;
};


class GEOS_DLL CoordinateXYZM : public Coordinate {
private:
    static const CoordinateXYZM _nullCoord;

public:
    CoordinateXYZM() : CoordinateXYZM(DEFAULT_X, DEFAULT_Y, DEFAULT_Z, DEFAULT_M) {}

    explicit CoordinateXYZM(const CoordinateXY& c)
        : Coordinate(c)
        , m(DEFAULT_M) {}

    explicit CoordinateXYZM(const CoordinateXYM& c)
        : Coordinate(c)
        , m(c.m) {}

    explicit CoordinateXYZM(const Coordinate& c)
        : Coordinate(c)
        , m(DEFAULT_M) {}

    CoordinateXYZM(double x_, double y_, double z_, double m_)
        : Coordinate(x_, y_, z_)
        , m(m_) {}

    double m;

    template<Ordinate>
    double get() const;

    static const CoordinateXYZM& getNull();

    void setNull()
    {
        Coordinate::setNull();
        m = DoubleNotANumber;
    };


    bool isNull() const
    {
        return Coordinate::isNull() && std::isnan(m);
    }

    bool equals4D(const CoordinateXYZM& other) const {
        return x == other.x && y == other.y &&
                (z == other.z || (std::isnan(z) && std::isnan(other.z))) &&
                (m == other.m || (std::isnan(m) && std::isnan(other.m)));
    }

    CoordinateXYZM& operator=(const CoordinateXY& other) {
        x = other.x;
        y = other.y;
        z = DEFAULT_Z;
        m = DEFAULT_M;

        return *this;
    }

    CoordinateXYZM& operator=(const Coordinate& other) {
        x = other.x;
        y = other.y;
        z = other.z;
        m = DEFAULT_M;

        return *this;
    }

    CoordinateXYZM& operator=(const CoordinateXYM& other) {
        x = other.x;
        y = other.y;
        z = DEFAULT_Z;
        m = other.m;

        return *this;
    }

    std::string toString() const;
};

inline CoordinateXYM&
CoordinateXYM::operator=(const CoordinateXYZM& other) {
    x = other.x;
    y = other.y;
    m = other.m;

    return *this;
}


/// Strict weak ordering Functor for Coordinate
struct GEOS_DLL CoordinateLessThan {

    bool operator()(const CoordinateXY* a, const CoordinateXY* b) const
    {
        if(a->compareTo(*b) < 0) {
            return true;
        }
        else {
            return false;
        }
    };

    bool operator()(const CoordinateXY& a, const CoordinateXY& b) const
    {
        if(a.compareTo(b) < 0) {
            return true;
        }
        else {
            return false;
        }
    };

};

/// Strict weak ordering operator for Coordinate
inline bool operator<(const CoordinateXY& a, const CoordinateXY& b)
{
    return CoordinateLessThan()(a, b);
}


// Generic accessors, XY

template<>
inline double CoordinateXY::get<Ordinate::X>() const
{
    return x;
}

template<>
inline double CoordinateXY::get<Ordinate::Y>() const
{
    return y;
}

template<>
inline double CoordinateXY::get<Ordinate::Z>() const
{
    return DEFAULT_Z;
}

template<>
inline double CoordinateXY::get<Ordinate::M>() const
{
    return DEFAULT_M;
}

// Generic accessors, XYZ

template<>
inline double Coordinate::get<Ordinate::X>() const
{
    return x;
}

template<>
inline double Coordinate::get<Ordinate::Y>() const
{
    return y;
}

template<>
inline double Coordinate::get<Ordinate::Z>() const
{
    return z;
}

template<>
inline double Coordinate::get<Ordinate::M>() const
{
    return DEFAULT_M;
}

// Generic accessors, XYM

template<>
inline double CoordinateXYM::get<Ordinate::X>() const
{
    return x;
}

template<>
inline double CoordinateXYM::get<Ordinate::Y>() const
{
    return y;
}

template<>
inline double CoordinateXYM::get<Ordinate::Z>() const
{
    return DEFAULT_Z;
}

template<>
inline double CoordinateXYM::get<Ordinate::M>() const
{
    return m;
}

// Generic accessors, XYZM

template<>
inline double CoordinateXYZM::get<Ordinate::X>() const
{
    return x;
}

template<>
inline double CoordinateXYZM::get<Ordinate::Y>() const
{
    return y;
}

template<>
inline double CoordinateXYZM::get<Ordinate::Z>() const
{
    return z;
}

template<>
inline double CoordinateXYZM::get<Ordinate::M>() const
{
    return m;
}

GEOS_DLL std::ostream& operator<< (std::ostream& os, const CoordinateXY& c);
GEOS_DLL std::ostream& operator<< (std::ostream& os, const Coordinate& c);
GEOS_DLL std::ostream& operator<< (std::ostream& os, const CoordinateXYM& c);
GEOS_DLL std::ostream& operator<< (std::ostream& os, const CoordinateXYZM& c);

} // namespace geos.geom
} // namespace geos

// Add specializations of std::common_type for Coordinate types
namespace std {
    template<> struct common_type<geos::geom::CoordinateXY, geos::geom::CoordinateXY>     { using type = geos::geom::CoordinateXY;   };
    template<> struct common_type<geos::geom::CoordinateXY, geos::geom::Coordinate>       { using type = geos::geom::Coordinate;     };
    template<> struct common_type<geos::geom::CoordinateXY, geos::geom::CoordinateXYM>    { using type = geos::geom::CoordinateXYM;  };
    template<> struct common_type<geos::geom::CoordinateXY, geos::geom::CoordinateXYZM>   { using type = geos::geom::CoordinateXYZM; };

    template<> struct common_type<geos::geom::Coordinate, geos::geom::CoordinateXY>       { using type = geos::geom::Coordinate;     };
    template<> struct common_type<geos::geom::Coordinate, geos::geom::Coordinate>         { using type = geos::geom::Coordinate;     };
    template<> struct common_type<geos::geom::Coordinate, geos::geom::CoordinateXYM>      { using type = geos::geom::CoordinateXYZM; };
    template<> struct common_type<geos::geom::Coordinate, geos::geom::CoordinateXYZM>     { using type = geos::geom::CoordinateXYZM; };

    template<> struct common_type<geos::geom::CoordinateXYM, geos::geom::CoordinateXY>    { using type = geos::geom::CoordinateXYM;  };
    template<> struct common_type<geos::geom::CoordinateXYM, geos::geom::Coordinate>      { using type = geos::geom::CoordinateXYZM; };
    template<> struct common_type<geos::geom::CoordinateXYM, geos::geom::CoordinateXYM>   { using type = geos::geom::CoordinateXYM;  };
    template<> struct common_type<geos::geom::CoordinateXYM, geos::geom::CoordinateXYZM>  { using type = geos::geom::CoordinateXYZM; };

    template<> struct common_type<geos::geom::CoordinateXYZM, geos::geom::CoordinateXY>   { using type = geos::geom::CoordinateXYZM; };
    template<> struct common_type<geos::geom::CoordinateXYZM, geos::geom::Coordinate>     { using type = geos::geom::CoordinateXYZM; };
    template<> struct common_type<geos::geom::CoordinateXYZM, geos::geom::CoordinateXYM>  { using type = geos::geom::CoordinateXYZM; };
    template<> struct common_type<geos::geom::CoordinateXYZM, geos::geom::CoordinateXYZM> { using type = geos::geom::CoordinateXYZM; };
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif


























