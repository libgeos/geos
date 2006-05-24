#ifndef UTILITY_H_INCLUDED
#define UTILITY_H_INCLUDED

#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/Polygon.h>

#include <iostream>
#include <cassert>

//
// GEOS Unit Test utilities
//
namespace utility
{

//
// Type cast helper utilities
//

template<class Type, class InstanceType>
inline bool isInstanceOf(InstanceType* instance)
{
    return ( 0 != dynamic_cast<Type *>(instance) );
}

template<class Type, class InstanceType>
inline Type* instanceOf(InstanceType* instance)
{
    return dynamic_cast<Type *>(instance);
}

//
// Geometries structure comparators
//

template <class T1, class T2>
inline bool isSameStructure(T1 lhs, T2 rhs)
{
    // Different types can't have the smame structure
    return false;
}

template <class T>
inline bool isSameStructure(T* lhs, T* rhs)
{
    using geos::geom::Polygon;
    using geos::geom::GeometryCollection;
    
    assert( 0 != lhs );
    assert( 0 != rhs );

    // Both are empty
    if (!(lhs->isEmpty() == rhs->isEmpty()))
        return false;

    // Both are non-empty
    if (!(!lhs->isEmpty()) == (!rhs->isEmpty()))
        return false;

    // Both are valid
    if (!(lhs->isValid() == rhs->isValid()))
        return false;

    // Dispatch to run more specific testes
    if (isInstanceOf<Polygon>(lhs)
        && isInstanceOf<Polygon>(rhs))
    {
        return isSameStructure(instanceOf<Polygon>(lhs),
                               instanceOf<Polygon>(rhs));
    }
    else if (isInstanceOf<GeometryCollection>(lhs)
            && isInstanceOf<GeometryCollection>(rhs))
    {
        return isSameStructure(instanceOf<GeometryCollection>(lhs),
                               instanceOf<GeometryCollection>(rhs));
    }

    return true;
}

template <>
inline bool isSameStructure(geos::geom::Polygon* lhs,
                            geos::geom::Polygon* rhs)
{
    assert( 0 != lhs );
    assert( 0 != rhs );

    return (lhs->getNumInteriorRing() == rhs->getNumInteriorRing());
}

template <>
inline bool isSameStructure(geos::geom::GeometryCollection* lhs,
                            geos::geom::GeometryCollection* rhs)
{
    using geos::geom::Geometry;

    assert( 0 != lhs );
    assert( 0 != rhs );

    if (lhs->getNumGeometries() != rhs->getNumGeometries())
        return false;

    for (int i = 0; i < lhs->getNumGeometries(); i++)
    {
        // Dirty, but necessary!
        // isSameStructure promises to not to try to change geometries
        Geometry* g1 = const_cast<Geometry*>(lhs->getGeometryN(i));
        Geometry* g2 = const_cast<Geometry*>(rhs->getGeometryN(i));
        if (!isSameStructure(g1, g2))
        {
            return false;
        }
    }
}

} // namespace unit

#endif // #ifndef UTILITY_H_INCLUDED


