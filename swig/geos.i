/* =========================================================================
 * $Id$
 *
 * geos.i
 * 
 * Copyright 2004 Sean Gillies, sgillies@frii.com
 *
 * Interface for a SWIG generated geos module.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 * ========================================================================= */
 
%module geos
%include "std_string.i"
%include "exception.i"

%{ 
#include "../../source/headers/geos.h"
%}

// Following methods are prototypes but unimplemented and are to be ignored
%ignore geos::MultiPoint::isClosed;

// Required renaming
%rename(Coordinate_Coordinate) Coordinate::Coordinate;

%exception {
    try {
        $action
    }
    catch (geos::GEOSException *e) {
        SWIG_exception(SWIG_RuntimeError, e->toString().data());
    }
}


// Now include the headers to be wrapped
%include "../../source/headers/geos/geom.h"
%include "../../source/headers/geos/util.h"
%include "../../source/headers/geos/io.h"

