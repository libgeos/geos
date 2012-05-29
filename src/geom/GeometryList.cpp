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
 **********************************************************************
 *
 * Last port: ORIGINAL WORK
 *
 **********************************************************************/

#include <geos/geom/GeometryList.h>
#include <geos/geom/Geometry.h> // for auto_ptr

#include <memory> // for auto_ptr
#include <vector> 

namespace geos {
namespace geom { // geos.geom


/*private*/
GeometryList::GeometryList()
{
}

/*private*/
GeometryList::~GeometryList()
{
	for (std::vector<Geometry*>::size_type i=0, n=geoms.size(); i<n; i++)
	{
		delete geoms[i];
	}
}

/*public static*/
GeometryList::AutoPtr
GeometryList::create()
{
	return GeometryList::AutoPtr(new GeometryList());
}

/*public*/
void
GeometryList::add(Geometry::AutoPtr geom)
{
	geoms.push_back(geom.release());
}

/*public*/
GeometryList::size_type
GeometryList::size() const
{
	return geoms.size();
}

/*public*/
Geometry *
GeometryList::operator[] (GeometryList::size_type i)
{
	return geoms[i];
}

/*public*/
const Geometry *
GeometryList::operator[] (GeometryList::size_type i) const
{
	return geoms[i];
}


} // namespace geos.geom
} // namespace geos
