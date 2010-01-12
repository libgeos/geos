/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2009  Sandro Santilli <strk@keybit.net>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 ***********************************************************************
 *
 * Last port: operation/overlay/snap/SnapIfNeededOverlayOp.java rev 1.1
 * (JTS-1.10)
 *
 **********************************************************************/

#include <geos/operation/overlay/snap/SnapIfNeededOverlayOp.h>
#include <geos/operation/overlay/snap/SnapOverlayOp.h>
#include <geos/operation/overlay/OverlayOp.h>
#include <geos/geom/Geometry.h> // for use in auto_ptr
#include <geos/util.h>

#include <cassert>
#include <limits> // for numeric_limits
#include <memory> // for auto_ptr

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

using namespace std;
using namespace geos::geom;

namespace geos {
namespace operation { // geos.operation
namespace overlay { // geos.operation.overlay
namespace snap { // geos.operation.overlay.snap

/* public */
auto_ptr<Geometry>
SnapIfNeededOverlayOp::getResultGeometry(OverlayOp::OpCode opCode)
{
	auto_ptr<Geometry> result;

	bool isSuccess = false;

	try {
		result.reset( OverlayOp::overlayOp(&geom0, &geom1, opCode) );
		bool isValid = true;

		// not needed if noding validation is used
		// bool isValid = OverlayResultValidator::isValid(
		//                     geom0, geom1, OverlayOp::INTERSECTION,
		//                     result);

		if (isValid) isSuccess = true;

	}
	catch (std::exception& ex) {
        ::geos::ignore_unused_variable_warning(ex);
#if GEOS_DEBUG
		std::cerr << "Overlay op threw " << ex.what() << ". Will try snapping now" << std::endl;
#endif
	}

	if (! isSuccess) {
		// this may still throw an exception - just let it go if it does
		result = SnapOverlayOp::overlayOp(geom0, geom1, opCode);
	}

	return result;
}


} // namespace geos.operation.snap
} // namespace geos.operation.overlay
} // namespace geos.operation
} // namespace geos

