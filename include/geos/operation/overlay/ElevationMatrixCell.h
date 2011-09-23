/**********************************************************************
 * $Id: ElevationMatrixCell.h 3255 2011-03-01 17:56:10Z mloskot $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 ***********************************************************************
 *
 * Last port: original (by strk)
 *
 **********************************************************************/

#ifndef GEOS_OP_OVERLAY_ELEVATIONMATRIXCELL_H
#define GEOS_OP_OVERLAY_ELEVATIONMATRIXCELL_H

#include <geos/export.h>

#include <set>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // warning C4251: needs to have dll-interface to be used by clients of class
#endif

// Forward declarations
namespace geos {
	namespace geom {
		class Coordinate;
	}
}

namespace geos {
namespace operation { // geos::operation
namespace overlay { // geos::operation::overlay


class GEOS_DLL ElevationMatrixCell {
public:
	ElevationMatrixCell();
	~ElevationMatrixCell();
	void add(const geom::Coordinate &c);
	void add(double z);
	double getAvg(void) const;
	double getTotal(void) const;
	std::string print() const;
private:
	std::set<double>zvals;
	double ztot;
};

} // namespace geos::operation::overlay
} // namespace geos::operation
} // namespace geos

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // ndef GEOS_OP_OVERLAY_ELEVATIONMATRIXCELL_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/17 13:24:59  strk
 * opOverlay.h header splitted. Reduced header inclusions in operation/overlay implementation files. ElevationMatrixFilter code moved from own file to ElevationMatrix.cpp (ideally a class-private).
 *
 **********************************************************************/

