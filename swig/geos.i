/* =========================================================================
 * $Id$
 *
 * geos.i
 * 
 * Copyright 2004 Sean Gillies, sgillies@frii.com
 * Updated 2005 Charlie Savage, cfis@interserv.com
 *
 * Interface for a SWIG generated geos module.  Depends on 
 * SWIG 1.3.29 and higher.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 * ========================================================================= */
 
%module geos
%include "attribute.i"
%include "exception.i"
%include "std_string.i"
%include "std_vector.i"
%include "std_except.i"
%include "factory.i"

%{ 
#include "geos.h"
#include <sstream>
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


namespace geos {
namespace geom {
	%ignore GeometryFactory::buildGeometry(vector<Geometry * > *) const;
	%ignore GeometryFactory::createGeometryCollection(vector<Geometry * > *) const;
	%ignore GeometryFactory::createLinearRing(CoordinateSequence *) const;
	%ignore GeometryFactory::createLineString(CoordinateSequence *) const;
	%ignore GeometryFactory::createMultiLineString(vector<Geometry * > *) const;
	%ignore GeometryFactory::createMultiPoint(vector<Geometry * > *) const;
	%ignore GeometryFactory::createMultiPolygon(vector<Geometry * > *) const;
	%ignore GeometryFactory::createPoint(CoordinateSequence *) const;
	%ignore GeometryFactory::createPolygon(LinearRing *,vector<Geometry * > *) const;

	/* For scripting languages the CoordinateSequence::add method is ambigious
		 since there are two overloaded versions that are the same except
		 a const declaration.  However, one of them is available only for 
		 backwards compatibility so hide that one.*/
	%ignore CoordinateSequence::add(CoordinateSequence *cl,bool allowRepeated,bool direction);
} /* End geom namespace */
} /* End geos namespace */


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
  
  /* Write data to the stream.  Note that the returned size includes
     the last character, which is a null character.  We do not want
     to write that to the stream so subtract one from its size. */
  stream.write(buf, size - 1);

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

namespace geos {
	/* These disown definitions are not correct in all cases - 
		 needs to be fixed */
	%apply SWIGTYPE *DISOWN { geom::CoordinateSequence * };
	%apply SWIGTYPE *DISOWN { geom::LinearRing * };
	%apply SWIGTYPE *DISOWN { std::vector<geos::Geometry * > * };
	%apply SWIGTYPE *DISOWN { std::vector<geos::Coordinate> * };


	// These methods create new objects
	%newobject *::clone;
	%newobject *::getCoordinates;

	%newobject geom::CoordinateArraySequence::create;
	%newobject geom::GeometryFactory::createPointFromInternalCoord;
	%newobject geom::GeometryFactory::toGeometry;
	%newobject geom::GeometryFactory::createPoint;
	%newobject geom::GeometryFactory::createGeometryCollection;
	%newobject geom::GeometryFactory::createMultiLineString;
	%newobject geom::GeometryFactory::createMultiPolygon;
	%newobject geom::GeometryFactory::createLinearRing;
	%newobject geom::GeometryFactory::createMultiPoint;
	%newobject geom::GeometryFactory::createPolygon;
	%newobject geom::GeometryFactory::createLineString;
	%newobject geom::GeometryFactory::buildGeometry;
	%newobject geom::GeometryFactory::createGeometry;
	%newobject geom::GeometricShapeFactory::getEnvelope;

	%newobject io::WKBReader::read;
	%newobject io::WKBReader::readHEX;
	%newobject io::WKTReader::read;

	/* Surface methods that return Geometry* so that they
		 return the actual geometry object (point, linestring, etc.)
		 instead of a generic geometry object.  Make sure the checks
		 go from child class to parent class so we get the right one.*/
	%factory(geom::Geometry * io::WKTReader::read, 
		       geom::Point, 
					 geom::LinearRing, geom::LineString,
					 geom::Polygon, 
					 geom::MultiPoint, geom::MultiLineString,
					 geom::MultiPolygon, geom::GeometryCollection);

	%factory(geom::Geometry * io::WKBReader::read,
		       geom::Point, 
					 geom::LinearRing, geom::LineString,
					 geom::Polygon, 
					 geom::MultiPoint, geom::MultiLineString,
					 geom::MultiPolygon, geom::GeometryCollection);

	%factory(geom::Geometry * io::WKBReader::readHEX,
		       geom::Point, 
					 geom::LinearRing, geom::LineString,
					 geom::Polygon, 
					 geom::MultiPoint, geom::MultiLineString,
					 geom::MultiPolygon, geom::GeometryCollection);

	%newobject Geometry::fromWKT;
	%factory(geom::Geometry * io::Geometry::fromWKT,
		       geom::Point, 
					 geom::LinearRing, geom::LineString,
					 geom::Polygon, 
					 geom::MultiPoint, geom::MultiLineString,
					 geom::MultiPolygon, geom::GeometryCollection);

	%newobject Geometry::fromHEX;
	%factory(geom::Geometry * io::Geometry::fromHEX,
		       geom::Point, 
					 geom::LinearRing, geom::LineString,
					 geom::Polygon, 
					 geom::MultiPoint, geom::MultiLineString,
					 geom::MultiPolygon, geom::GeometryCollection);

	%factory(geom::Geometry * geom::Geometry::downcast,
		       geom::Point, 
					 geom::LinearRing, geom::LineString,
					 geom::Polygon, 
					 geom::MultiPoint, geom::MultiLineString,
					 geom::MultiPolygon, geom::GeometryCollection);

	/* ================= Operators  ============== */
	/* Ignore these for now - if we don't SWIG will create
		 names with the same methods, which doesn't work out very well.

		 std::ostream& operator<<(std::ostream& os, const planarNode& n); 
		 std::ostream& operator<<(std::ostream& os, const planarEdge& n); 
		 std::ostream& operator<<(std::ostream& os, const planarNode& n); 
		 std::ostream& operator<< (std::ostream& os, const Coordinate& c);
		 std::ostream& operator<< (std::ostream& o, const LineSegment& l);
		 std::ostream& operator<< (std::ostream& os, const Coordinate& c); */
	%ignore operator<<;
} /* End geos namespace */


/* =============  Define Attributes ============ */
/* Need to fully qualify the class names for some reason. */
%attribute(geos::geom::Point, double, x, getX);
%attribute(geos::geom::Point, double, y, getY);
%attribute(geos::geom::Geometry, double, srid, getSRID, setSRID);



/* GEOS 3 throws and catches exceptions by reference, thus allowing us
	 to signficantly clean up the code. */

/* Mark these classes as exception classes */
%exceptionclass geos::util::GEOSException;

/* These are all subclasses of GEOSException */
%exceptionclass geos::util::AssertionFailedException;
%exceptionclass geos::util::IllegalArgumentException;
%exceptionclass geos::io::ParseException;
%exceptionclass geos::util::TopologyException;
%exceptionclass geos::util::UnsupportedOperationException;

/* Setup up generalized exception handling.  Note that GEOS 3.0.0 
	 and above throw and catch exceptions be reference.  In earlier
	 versions it threw objects allocated on the heap.*/
%catches(geos::util::AssertionFailedException,
				 geos::util::IllegalArgumentException, geos::io::ParseException;
				 geos::util::TopologyException, geos::util::UnsupportedOperationException, 
				 geos::util::GEOSException);


/* Now tell SWIG what headers to process */
%include "../source/headers/geos/geom/geometry.h"
%include "../source/headers/geos/geom/GeometryFactory.h"
%include "../source/headers/geos/geom/PrecisionModel.h"
%include "../source/headers/geos/geom/CoordinateSequenceFactory.h"
%include "../source/headers/geos/geom/Coordinate.h"
%include "../source/headers/geos/geom/CoordinateSequence.h"
%include "../source/headers/geos/geom/CoordinateArraySequence.h"

%include "../source/headers/geos/geom/Envelope.h"

%include "../source/headers/geos/geom/Point.h"
%include "../source/headers/geos/geom/LineString.h"
%include "../source/headers/geos/geom/LinearRing.h"
%include "../source/headers/geos/geom/Polygon.h"
%include "../source/headers/geos/geom/GeometryCollection.h"
%include "../source/headers/geos/geom/MultiPoint.h"
%include "../source/headers/geos/geom/MultiLineString.h"
%include "../source/headers/geos/geom/MultiPolygon.h"

%include "../source/headers/geos/io/WKBReader.h"
%include "../source/headers/geos/io/WKBWriter.h"
%include "../source/headers/geos/io/WKTReader.h"
%include "../source/headers/geos/io/WKTWriter.h"

%include "../source/headers/geos/util/GEOSException.h"
%include "../source/headers/geos/util/GeometricShapeFactory.h"
