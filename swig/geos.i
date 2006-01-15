/* =========================================================================
 * $Id$
 *
 * geos.i
 * 
 * Copyright 2004 Sean Gillies, sgillies@frii.com
 * Updated 2005 Charlie Savage, cfis@interserv.com
 *
 * Interface for a SWIG generated geos module.  Depends on 
 * SWIG 1.3.28 and higher.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 * ========================================================================= */
 
%module geos
%include "std_string.i"
%include "std_vector.i"
%include "exception.i"

%{ 
#include "geos.h"
#include "geos/planargraph.h"
#include "geos/opLinemerge.h"
#include "geos/opPolygonize.h"
%}


/* ================= Shadowed Methods ============== */

/* The overloaded C++ methods cannot be disambiguated by
   SWIG.  For example:
   
	geos::GeometryFactory::createPoint(CoordinateSequence *) const;
	geos::GeometryFactory::createPoint(const CoordinateSequence&) const;
   
   For each pair of methods, one will take ownership
   of the parameter (thus it is passed as a pointer) while
   the other one will not (thus is passed as a const reference).
   To be on the safe-side, we tell SWIG to ignore the method
   that takes over ownership.  To expose these ignored methods
   to a scripting language requires renaming the method to
   something else.  This can be done in each language module
   as it is loaded below.*/

%ignore geos::GeometryFactory::buildGeometry(vector<Geometry * > *) const;
%ignore geos::GeometryFactory::createGeometryCollection(vector<Geometry * > *) const;
%ignore geos::GeometryFactory::createLinearRing(CoordinateSequence *) const;
%ignore geos::GeometryFactory::createLineString(CoordinateSequence *) const;
%ignore geos::GeometryFactory::createMultiLineString(vector<Geometry * > *) const;
%ignore geos::GeometryFactory::createMultiPoint(vector<Geometry * > *) const;
%ignore geos::GeometryFactory::createMultiPolygon(vector<Geometry * > *) const;
%ignore geos::GeometryFactory::createPoint(CoordinateSequence *) const;
%ignore geos::GeometryFactory::createPolygon(LinearRing *,vector<Geometry * > *) const;

/* For scripting languages the CoordinateSequence::add method is ambigious
   since there are two overloaded versions that are the same except
   a const declaration.  However, one of them is available only for 
   backwards compatibility so hide that one.*/
%ignore geos::CoordinateSequence::add(CoordinateSequence *cl,bool allowRepeated,bool direction);


/* ================= Typemaps ============== */

/* These typemaps allows scripting languages to call methods that take
   iostreams and ostreams, such as WKTWriter.printHEX and WKBWriter.write. 
   The results are returned from the method as a string object in the
   scripting language.*/
   
/* Get rid of the ostream parameter by setting numinputs to 0 */   
%typemap(in,numinputs=0) ostream& (std::ostringstream stream)
{
  $1 = &stream;
}

/* Return the results as a string object in the scripting language */
%typemap(argout) ostream& 
{
	std::string str = stream$argnum.str();	
	$result = SWIG_FromCharPtrAndSize(str.data(), str.size());
}

/* This typemap allows the scripting language to pass in a string
   which will be converted to an istringstream for GEOS */
%typemap(in) istream& (char *buf = 0, size_t size = 0, int alloc = 0, std::stringstream stream)
{
  /* Convert from scripting language string to char* */
  if (SWIG_AsCharPtrAndSize($input, &buf, &size, &alloc) != SWIG_OK)
  {
    %argument_fail(SWIG_TypeError, "(TYPEMAP, SIZE)", $symname, $argnum);
  }
  
  /* Write data to the stream */
  stream.write(buf, size);

  $1 = &stream;
}

/* Free allocated buffer created in the (in) typemap */
%typemap(freearg) istream&
{
  if (alloc$argnum == SWIG_NEWOBJ) %delete_array(buf$argnum);
}


/* ==============  Language Specific Files ============ */

/* Import language specific SWIG files.  This allows each language
   to define its own renames as well as any special functionality
   such as language specific iterators for collections. Note 
   that %include allows the included files to generate interface
   wrapper code while %import does not.  Thus use %include since
   this is an important feature (for example, Ruby needs it to #undef
   the select macro) */


#ifdef SWIGPYTHON
%include ../python/python.i
#endif

#ifdef SWIGRUBY
%include ../ruby/ruby.i
#endif


/* ================= Ownership Rules ============== */

/* These disown definitions are not correct in all cases - 
   needs to be fixed */
%apply SWIGTYPE *DISOWN { geos::CoordinateSequence * };
%apply SWIGTYPE *DISOWN { geos::LinearRing * };
%apply SWIGTYPE *DISOWN { std::vector<geos::Geometry * > * };
%apply SWIGTYPE *DISOWN { std::vector<geos::Coordinate> * };


// These methods create new objects
%newobject *::clone;
%newobject *::getCoordinates;
%newobject geos::WKBReader::read;
%newobject geos::WKBReader::read_hex;
%newobject geos::WKTReader::read;
%newobject geos::DefaultCoordinateSequenceFactory::create;
%newobject geos::GeometryFactory::createPointFromInternalCoord;
%newobject geos::GeometryFactory::toGeometry;
%newobject geos::GeometryFactory::createPoint;
%newobject geos::GeometryFactory::createGeometryCollection;
%newobject geos::GeometryFactory::createMultiLineString;
%newobject geos::GeometryFactory::createMultiPolygon;
%newobject geos::GeometryFactory::createLinearRing;
%newobject geos::GeometryFactory::createMultiPoint;
%newobject geos::GeometryFactory::createPolygon;
%newobject geos::GeometryFactory::createLineString;
%newobject geos::GeometryFactory::buildGeometry;
%newobject geos::GeometryFactory::createGeometry;
%newobject geos::GeometricShapeFactory::getEnvelope;

/* ================= Exception Handling  ============== */

/* Mark these classes as exception classes */
%exceptionclass geos::GEOSException;

/* These are all subclasses of GEOSException */
%exceptionclass geos::AssertionFailedException;
%exceptionclass geos::IllegalArgumentException;
%exceptionclass geos::ParseException;
%exceptionclass geos::TopologyException;
%exceptionclass geos::UnsupportedOperationException;

/* This exception class is not surfaced to SWIG 
%exceptionclass geos::NotRepresentableException;*/

/* Setup up generalized exception handling.  Note that GEOS throws
   classes that are allocated on the heap so we need to catch them
   by pointer and are responsible for freeing them.  To do this
   we'll just pass the object along the scripting language (wrapped
   of course) and make it responsbile for freeing the object via the
   use of the SWIG_POINTER_OWN flag.*/

%exception {
    try {
        $action
    }
  catch (geos::AssertionFailedException *e) {
			%raise(SWIG_NewPointerObj(e, SWIGTYPE_p_geos__AssertionFailedException, SWIG_POINTER_OWN), "geos::AssertionFailedException", SWIGTYPE_p_geos__AssertionFailedException);
  }
  catch (geos::IllegalArgumentException *e) {
			%raise(SWIG_NewPointerObj(e, SWIGTYPE_p_geos__IllegalArgumentException, SWIG_POINTER_OWN), "geos::IllegalArgumentException", SWIGTYPE_p_geos__IllegalArgumentException);
  }
  catch (geos::ParseException *e) {
			%raise(SWIG_NewPointerObj(e, SWIGTYPE_p_geos__ParseException, SWIG_POINTER_OWN), "geos::ParseException", SWIGTYPE_p_geos__ParseException);
  }
  catch (geos::TopologyException *e) {
			%raise(SWIG_NewPointerObj(e, SWIGTYPE_p_geos__TopologyException, SWIG_POINTER_OWN), "geos::TopologyException", SWIGTYPE_p_geos__TopologyException);
  }
  catch (geos::UnsupportedOperationException *e) {
			%raise(SWIG_NewPointerObj(e, SWIGTYPE_p_geos__UnsupportedOperationException, SWIG_POINTER_OWN), "geos::UnsupportedOperationException", SWIGTYPE_p_geos__UnsupportedOperationException);
  }
    catch (geos::GEOSException *e) {
			%raise(SWIG_NewPointerObj(e, SWIGTYPE_p_geos__GEOSException, SWIG_POINTER_OWN), "geos::GEOSException", SWIGTYPE_p_geos__GEOSException);
  }
  catch (...) {
      SWIG_exception(SWIG_RuntimeError, "Unknown exception took place in the method: $symname.");
    }
}


/* =============  Classes to ignore (why are these ignored? ======= */
%ignore geos::LineMergeDirectedEdge;
%ignore geos::PolygonizeEdge;
%ignore geos::polygonizeEdgeRing;
%ignore geos::PolygonizeDirectedEdge;
%ignore geos::PolygonizeGraph;


/* =============  Define Attributes ============ */
%include <attribute.i>
%attribute(geos::Point, double, x, getX);
%attribute(geos::Point, double, y, getY);


/* ===========  Headers To Wrap  ================ */
%include "../../source/headers/geos/geom.h"
%include "../../source/headers/geos/planargraph.h"
%include "../../source/headers/geos/util.h"
%include "../../source/headers/geos/io.h"
%include "../../source/headers/geos/opLinemerge.h"
%include "../../source/headers/geos/opPolygonize.h"
